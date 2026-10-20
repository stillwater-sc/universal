// sqrt_precision.cpp: regression tests for efloat sqrt()/cbrt() accuracy scaling.
//
// Guards issue #1140: sqrt/cbrt used a fixed 7 Newton iterations from a ~1-bit
// exponent-only seed, freezing accuracy at ~323 bits (~97 digits) regardless of
// get_precision(). The iteration count is now precision-adaptive, so accuracy
// must scale with the working precision.
//
// Correct rounding (#1763): sqrt now forms the exact integer root and its remainder and rounds
// once.  Each result r at precision P is checked against a with exact squares only: in every
// rounding mode r and its neighbour at P bits must bracket sqrt(a) as the mode requires, and
// Inexact must be raised exactly when r^2 != a.  The radicands include roots just above a P-bit
// value or a midpoint, by about 2^-2P (a = (R^2 + 7) / 2^k with R^2 = -7 mod 2^k, by Hensel's
// lifting), which only the remainder's sticky bit rounds right.  Before, Newton's iteration rounded from 64
// guard bits: sqrt(4 - 2^-62) at 64 bits came out 2 rather than 2 - 2^-63, and an exact root
// such as sqrt(9) raised Inexact.
//
// REGRESSION_LEVEL convention (intensity progression):
//   LEVEL 1 -- the basic values, the exponents, 400 rounded roots per precision
//   LEVEL 2 -- 4,000 rounded roots per precision
//   LEVEL 3 -- 40,000
//   LEVEL 4 -- 400,000
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <limits>
#include <random>
#include <vector>
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	// |a - b| binary scale (very negative == agreement to many bits; -1e6 == exact)
	template<unsigned nlimbs>
	int64_t err_scale(sw::universal::efloat<nlimbs> a, const sw::universal::efloat<nlimbs>& b) {
		a = a - b; a.setsign(false);
		return a.iszero() ? -1000000 : a.scale();
	}

	int VerifyEfloatSqrtPrecision(bool reportTestCases) {
		using namespace sw::universal;
		int failures = 0;

		// ---------------------------------------------------------------------
		// 1. Basic values
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying basic sqrt/cbrt/hypot values...\n";
		{
			if (std::abs(double(sqrt(efloat<8>(4.0)))  - 2.0) > 1e-13) { if (reportTestCases) std::cout << "    FAIL: sqrt(4)!=2\n"; ++failures; }
			if (std::abs(double(sqrt(efloat<8>(2.0)))  - std::sqrt(2.0)) > 1e-13) { if (reportTestCases) std::cout << "    FAIL: sqrt(2)\n"; ++failures; }
			if (std::abs(double(cbrt(efloat<8>(27.0))) - 3.0) > 1e-13) { if (reportTestCases) std::cout << "    FAIL: cbrt(27)!=3\n"; ++failures; }
			if (std::abs(double(cbrt(efloat<8>(-8.0))) + 2.0) > 1e-13) { if (reportTestCases) std::cout << "    FAIL: cbrt(-8)!=-2\n"; ++failures; }
			if (std::abs(double(hypot(efloat<8>(3.0), efloat<8>(4.0))) - 5.0) > 1e-13) { if (reportTestCases) std::cout << "    FAIL: hypot(3,4)!=5\n"; ++failures; }
		}

		// ---------------------------------------------------------------------
		// 2. Accuracy scales with precision (the #1140 guard).
		//    At capacity B bits, |sqrt(2)^2 - 2| must fall well below 2^-300
		//    (the old cap) -- we require agreement to at least B-8 bits.
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying sqrt accuracy scales with precision...\n";
		{
			// efloat<16> -> 512-bit capacity; require > 300-bit agreement (old cap was ~323)
			{
				efloat<16> two(2.0);
				efloat<16> r = sqrt(two);
				int64_t sc = err_scale<16>(r * r, two);
				if (sc > -500) { if (reportTestCases) std::cout << "    FAIL: sqrt@512 only " << -sc << " bits (want > 500)\n"; ++failures; }
			}
			// efloat<64> -> 2048-bit capacity; must far exceed the ~323-bit old cap
			{
				efloat<64> two(2.0);
				efloat<64> r = sqrt(two);
				int64_t sc = err_scale<64>(r * r, two);
				if (sc > -2000) { if (reportTestCases) std::cout << "    FAIL: sqrt@2048 only " << -sc << " bits (want > 2000; old cap ~323)\n"; ++failures; }
			}
			// cbrt at 2048-bit capacity
			{
				efloat<64> two(2.0);
				efloat<64> r = cbrt(two);
				int64_t sc = err_scale<64>(r * r * r, two);
				if (sc > -2000) { if (reportTestCases) std::cout << "    FAIL: cbrt@2048 only " << -sc << " bits (want > 2000)\n"; ++failures; }
			}
		}

		// ---------------------------------------------------------------------
		// 3. Special values
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying sqrt special values...\n";
		{
			if (!sqrt(efloat<8>(0.0)).iszero())      { if (reportTestCases) std::cout << "    FAIL: sqrt(0)\n"; ++failures; }
			if (!sqrt(efloat<8>(-1.0)).isnan())      { if (reportTestCases) std::cout << "    FAIL: sqrt(-1) not NaN\n"; ++failures; }
			efloat<8> inf; inf.setinf(false);
			if (!sqrt(inf).isinf())                  { if (reportTestCases) std::cout << "    FAIL: sqrt(inf)\n"; ++failures; }
			efloat<8> nan; nan.setnan();
			if (!sqrt(nan).isnan())                  { if (reportTestCases) std::cout << "    FAIL: sqrt(nan)\n"; ++failures; }
		}

		return failures;
	}

	// sqrt scales its argument by 2^-2 floor(e / 2) to [1, 4): odd and negative exponents, and
	// ones far past int32, must take the floor, not the truncation (#1756)
	int VerifySqrtExponents(bool reportTestCases) {
		using namespace sw::universal;
		int fails = 0;
		for (long long e : { -9LL, -8LL, -7LL, -1LL, 0LL, 1LL, 7LL, 8LL, -4000000001LL, 4000000001LL }) {
			efloat<4> a(3);
			a.setexponent(e);
			const efloat<4> r = sqrt(a);
			efloat<4> d = r * r - a;
			d.setsign(false);
			if (!(d.iszero() || d.scale() < a.scale() - 120)) {
				++fails;
				if (reportTestCases) std::cout << "    FAIL: sqrt(1.5 * 2^" << e << ")^2 is off by 2^" << d.scale() << "\n";
			}
		}
		return fails;
	}

	using Wide = sw::universal::efloat<64>;

	// x^2 exactly: at a precision that holds every bit of it
	Wide exact_square(const Wide& x) {
		Wide w(x);
		w.set_precision(static_cast<unsigned>(64 * x.bits().size() + 64));
		return w * w;
	}

	// a at precision P with n random limbs: runs of ones and zeros, all ones, a perfect square,
	// or a perfect square one unit in its last place away
	Wide random_radicand(std::mt19937_64& rng, unsigned p, unsigned shape) {
		using namespace sw::universal;
		Wide a(1);
		if (shape == 3 || shape == 4) {   // x^2 with x of at most P/2 bits, then +-1 unit for shape 4
			const unsigned half = std::max(1u, p / 2);
			Wide x(1);
			x.set_precision(32 * half);
			for (unsigned i = 0; i < half; ++i) x.setlimb(i, static_cast<uint32_t>(rng()) | (i == 0 ? 0x8000'0000u : 0u));
			x.setexponent(static_cast<int64_t>(rng() % 41) - 20);
			a = exact_square(x);
			a.set_precision(32 * p);
			if (shape == 4) {
				Wide unit(1);
				unit.setexponent(a.scale() - static_cast<int64_t>(a.get_precision()) + 1);
				if (rng() & 1u) a += unit; else a -= unit;
			}
			return a;
		}
		a.set_precision(32 * p);
		const unsigned n = 1 + static_cast<unsigned>(rng() % p);
		for (unsigned i = 0; i < n; ++i) {
			const uint64_t r = rng();
			uint32_t w = (shape == 1) ? 0xFFFF'FFFFu : (shape == 2 && (r & 3u) != 0u) ? ((r & 4u) ? 0xFFFF'FFFFu : 0u) : static_cast<uint32_t>(r >> 8);
			if (i == 0) w |= 0x8000'0000u;
			a.setlimb(i, w);
		}
		a.setexponent(static_cast<int64_t>(rng() % 401) - 200);
		a.set_precision(32 * p);   // normalizes: setlimb can leave zero limbs at the bottom
		return a;
	}

	// y with y^2 = -7 (mod 2^m), m >= 3: Hensel's lifting a bit at a time from y = 1, a root mod 8
	// since every odd square is 1 mod 8.  Each step takes y^2 + 7 = 0 (mod 2^k) to mod 2^(k+1).
	std::vector<uint64_t> root_of_minus_seven(unsigned m) {
		using namespace sw::universal::efloat_mpn;
		const size_t L = (m + 63) / 64;
		std::vector<limb_t> y(L, 0u), sq(2 * L), work(scratch_limbs(L));
		y[0] = 1u;
		for (unsigned k = 3; k < m; ++k) {
			mul_n(sq.data(), y.data(), y.data(), L, work.data(), true);
			add_1(sq.data(), 2 * L, 7u);
			if ((sq[k / 64] >> (k % 64)) & 1u) y[(k - 1) / 64] += limb_t(1) << ((k - 1) % 64);
		}
		return y;
	}

	// a, P bits, whose square root lies just above a P-bit value (midpoint false) or a midpoint
	// between two (midpoint true), by about 2^-2P: a = (R^2 + 7) / 2^(bits - P) for R of
	// Rb = P or P + 1 bits and `bits` the length of R^2 + 7.  R is a root of -7 mod 2^(Rb + 1):
	// with y one, so are y mod 2^Rb and 2^Rb minus it, which sum to 2^Rb, so one has its top bit
	// set.  bits - P <= 2 Rb - P <= Rb + 1, so the division is exact.  The root's bits past the
	// guard bit are then zero down to far below the integer root's last one, and only the
	// remainder, the sticky bit, rounds it the right way.
	Wide just_above(std::mt19937_64& rng, unsigned p, bool midpoint) {
		using namespace sw::universal::efloat_mpn;
		const unsigned P  = 32 * p;
		const unsigned rb = midpoint ? P + 1 : P;
		std::vector<limb_t> r = root_of_minus_seven(rb + 1);
		const size_t L = r.size();
		auto top_bit = [rb](const std::vector<limb_t>& v) { return ((v[(rb - 1) / 64] >> ((rb - 1) % 64)) & 1u) != 0u; };
		// y mod 2^Rb, then 2^Rb minus it when that one has the top bit
		for (size_t k = 0; k < L; ++k) {
			const size_t lo = 64 * k;
			if (lo >= rb) r[k] = 0u;
			else if (rb - lo < 64) r[k] &= (limb_t(1) << (rb - lo)) - 1u;
		}
		if (!top_bit(r)) {
			std::vector<limb_t> twoRb(L, 0u);
			twoRb[rb / 64] = limb_t(1) << (rb % 64);   // Rb < 64 L: L limbs hold Rb + 1 bits
			sub_n(r.data(), twoRb.data(), r.data(), L);
		}
		std::vector<limb_t> sq(2 * L), work(scratch_limbs(L));
		mul_n(sq.data(), r.data(), r.data(), L, work.data(), true);
		add_1(sq.data(), 2 * L, 7u);
		size_t bits = 128 * L;
		while (((sq[(bits - 1) / 64] >> ((bits - 1) % 64)) & 1u) == 0u) --bits;
		const size_t shift = bits - P;
		bool exact = top_bit(r);
		for (size_t b = 0; exact && b < shift; ++b) exact = ((sq[b / 64] >> (b % 64)) & 1u) == 0u;
		if (!exact) return Wide(0);   // the construction failed: the caller's check reports it
		// K = (R^2 + 7) / 2^shift, its P bits as P / 32 limbs
		Wide a(1);
		a.set_precision(P);
		for (unsigned i = 0; i < p; ++i) {
			const size_t lo = bits - 32 * (i + 1);   // the bit at the bottom of limb i
			const uint64_t w = (sq[lo / 64] >> (lo % 64)) | ((lo % 64 > 32 && lo / 64 + 1 < 2 * L) ? sq[lo / 64 + 1] << (64 - lo % 64) : 0u);
			a.setlimb(i, static_cast<uint32_t>(w));
		}
		// a = K 2^shift times a power of 4: the leading bit at bits - 1, plus an even offset
		a.setexponent(static_cast<int64_t>(bits) - 1 + 2 * (static_cast<int64_t>(rng() % 41) - 20) - 2 * (static_cast<int64_t>(bits) / 2));
		a.set_precision(P);
		return a;
	}

	int VerifyCorrectlyRoundedSqrt(bool reportTestCases, unsigned nrSamples) {
		using namespace sw::universal;
		int failures = 0;
		std::mt19937_64 rng(1763);
		const RoundingMode modes[] = { RoundingMode::RoundToNearest, RoundingMode::RoundToZero,
		                               RoundingMode::RoundTowardPositive, RoundingMode::RoundTowardNegative };
		const unsigned precisions[] = { 1, 2, 3, 5, 16, 40 };   // in limbs
		const RoundingMode saved = efloat_rounding_mode;
		for (unsigned p : precisions) {
			const unsigned samples = (p >= 16) ? 1 + nrSamples / 10 : nrSamples;
			for (unsigned i = 0; i < samples; ++i) {
				const unsigned shape = i % 7;
				Wide a = (shape >= 5) ? just_above(rng, p, shape == 6) : random_radicand(rng, p, shape);
				if (a.iszero()) {
					++failures;
					if (reportTestCases) std::cout << "    FAIL: the radicand just above a " << (shape == 6 ? "midpoint" : "value") << " at " << 32 * p << " bits is not exact\n";
					continue;
				}
				if (i % 9 == 0 && shape < 5) a.clear_precision();   // the type's 2048 bits
				const RoundingMode mode = modes[i % 4];
				efloat_rounding_mode = mode;
				clear_efloat_exceptions();
				const Wide r = sqrt(a);
				const bool inexact = has_efloat_exception(ExceptionFlag::Inexact);
				efloat_rounding_mode = RoundingMode::RoundToNearest;

				// r's neighbours at its precision: up by u, down by u, or u/2 below a power of 2
				const unsigned P = a.get_precision();
				Wide u(1), below(1);
				u.setexponent(r.scale() - static_cast<int64_t>(P) + 1);
				below.setexponent(u.scale() - ((r.bits().size() == 1 && r.bits()[0] == 0x8000'0000u) ? 1 : 0));
				Wide up(r), down(r);
				up.set_precision(P + 64);
				down.set_precision(P + 64);
				up += u;
				down -= below;
				const Wide r2 = exact_square(r);
				bool ok = (r.get_precision() == P) && (r.carries_precision() == a.carries_precision()) && (inexact == !(r2 == a));
				switch (mode) {
				case RoundingMode::RoundToNearest: {   // within half the gap on either side
					Wide hi(r), lo(r);
					hi.set_precision(P + 64);
					lo.set_precision(P + 64);
					Wide hu(u), hb(below);
					hu.setexponent(hu.scale() - 1);
					hb.setexponent(hb.scale() - 1);
					hi += hu;
					lo -= hb;
					ok = ok && !(a < exact_square(lo)) && !(exact_square(hi) < a);
					break;
				}
				case RoundingMode::RoundToZero:
				case RoundingMode::RoundTowardNegative:   // r <= sqrt(a) < r + u
					ok = ok && !(a < r2) && (a < exact_square(up));
					break;
				case RoundingMode::RoundTowardPositive:   // r - below < sqrt(a) <= r
					ok = ok && !(r2 < a) && (exact_square(down) < a);
					break;
				}
				if (!ok) {
					++failures;
					if (reportTestCases && failures <= 3) std::cout << "    FAIL: sqrt at " << P << " bits (shape " << shape << ", mode " << (i % 4) << ") is not correctly rounded\n";
				}
			}
		}
		efloat_rounding_mode = saved;
		// the case main misrounded: sqrt(4 - 2^-62) at 64 bits is 2 - 2^-63, all ones
		{
			efloat<2> a(1);
			a.set_precision(64);
			a.setlimb(0, 0xFFFF'FFFFu);
			a.setlimb(1, 0xFFFF'FFFFu);
			a.setexponent(1);
			const efloat<2> r = sqrt(a);
			if (!(r.scale() == 0 && r.bits() == std::vector<uint32_t>{ 0xFFFF'FFFFu, 0xFFFF'FFFFu })) {
				++failures;
				if (reportTestCases) std::cout << "    FAIL: sqrt(4 - 2^-62) at 64 bits is not 2 - 2^-63\n";
			}
		}
		return failures;
	}

}  // anonymous namespace

#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 0
#	define REGRESSION_LEVEL_3 0
#	define REGRESSION_LEVEL_4 0
#endif

int main() try {
	using namespace sw::universal;

	std::string test_suite          = "efloat sqrt precision library";
	std::string test_tag            = "sqrt-precision";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyCorrectlyRoundedSqrt(reportTestCases, 40), "efloat", "correctly rounded sqrt");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures

#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyEfloatSqrtPrecision(reportTestCases), "efloat", "sqrt precision");
	nrOfFailedTestCases += ReportTestResult(VerifySqrtExponents(reportTestCases), "efloat", "sqrt odd and wide exponents");
	nrOfFailedTestCases += ReportTestResult(VerifyCorrectlyRoundedSqrt(reportTestCases, 400), "efloat", "correctly rounded sqrt");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyCorrectlyRoundedSqrt(reportTestCases, 4000), "efloat", "correctly rounded sqrt");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyCorrectlyRoundedSqrt(reportTestCases, 40000), "efloat", "correctly rounded sqrt");
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyCorrectlyRoundedSqrt(reportTestCases, 400000), "efloat", "correctly rounded sqrt");
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
