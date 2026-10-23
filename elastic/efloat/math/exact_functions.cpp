// exact_functions.cpp: regression tests for efloat cbrt, hypot, fma, fmod and remainder, correctly
// rounded in every mode (#1783)
//
// Each is checked against a reference that shares none of its code:
//   - cbrt and hypot: the result r must bracket the exact argument between the cubes or squares
//     of its neighbours and the midpoints next to it, as the rounding mode requires; the cubes and
//     squares are exact. cbrt rounds from an integer cube root since #1800, which
//     arithmetic/limb_kernels.cpp checks against S^3 + R = N; here, besides random arguments
//     and exact cubes, roots of 2 and 4 limbs (where the root without its remainder can lie a
//     unit or two high), three arguments found by search whose root lies just below a rounding
//     boundary, and short arguments, whose exactness a shorter root decides.
//   - fma: x y + z formed exactly at a width that holds it, rounded once.
//   - fmod and remainder: binary long division, one shifted subtraction per quotient bit, which
//     also gives the quotient's parity; and, for quotients up to 2^(2^40), the closed forms
//     2^k mod 3 and 2^k mod 7.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <random>
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using sw::universal::RoundingMode;
	constexpr RoundingMode all_modes[] = { RoundingMode::RoundToNearest, RoundingMode::RoundToZero,
	                                       RoundingMode::RoundTowardPositive, RoundingMode::RoundTowardNegative };

	template<unsigned nlimbs>
	bool same(const sw::universal::efloat<nlimbs>& a, const sw::universal::efloat<nlimbs>& b) {
		return a == b && a.get_precision() == b.get_precision() && a.sign() == b.sign();
	}

	// a full-width P-bit value in [1, 2) times 2^e
	template<unsigned nlimbs>
	sw::universal::efloat<nlimbs> full_width(std::mt19937_64& rng, unsigned P, int64_t e) {
		sw::universal::efloat<nlimbs> v(1);
		v.set_precision(P);
		for (unsigned j = 0; j < (P + 31) / 32; ++j) v.setlimb(j, static_cast<uint32_t>(rng()) | (j == 0 ? 0x80000000u : 0u));
		v.set_precision(P);
		v.setexponent(e);
		return v;
	}

	// t^n exactly, for n = 2 or 3
	template<unsigned nlimbs>
	sw::universal::efloat<nlimbs> exact_pow(const sw::universal::efloat<nlimbs>& t, int n) {
		sw::universal::efloat<nlimbs> w(t);
		w.set_precision(static_cast<unsigned>(n) * (t.get_precision() + 64u));
		sw::universal::efloat<nlimbs> p = w * w;
		if (n == 3) p *= w;
		return p;
	}

	// the midpoint of two P-bit values, exactly
	template<unsigned nlimbs>
	sw::universal::efloat<nlimbs> midpoint(const sw::universal::efloat<nlimbs>& a, const sw::universal::efloat<nlimbs>& b) {
		sw::universal::efloat<nlimbs> m(a);
		m.set_precision(a.get_precision() + 64u);
		m += b;
		m.setexponent(m.scale() - 1);
		return m;
	}

	// whether r > 0, at P bits, is target^(1/n) rounded in `mode`, for an exact target > 0: target
	// must lie between the n-th powers of r's neighbours, or of the midpoints next to r
	template<unsigned nlimbs>
	bool brackets_root(const sw::universal::efloat<nlimbs>& r, const sw::universal::efloat<nlimbs>& target, int n, RoundingMode mode) {
		using namespace sw::universal;
		using E = efloat<nlimbs>;
		const RoundingMode saved = efloat_rounding_mode;
		efloat_rounding_mode = RoundingMode::RoundToNearest;
		const E inf = std::numeric_limits<E>::infinity();
		const E pred = nextafter(r, -inf), succ = nextafter(r, inf);
		bool ok = false;
		switch (mode) {
		case RoundingMode::RoundToNearest: {
			const E lo = exact_pow(midpoint(pred, r), n), hi = exact_pow(midpoint(r, succ), n);
			const bool even = efloat_detail::significant_bits(r) < r.get_precision();
			ok = (lo < target && target < hi) || ((target == lo || target == hi) && even);
			break;
		}
		case RoundingMode::RoundToZero:
		case RoundingMode::RoundTowardNegative:
			ok = exact_pow(r, n) <= target && target < exact_pow(succ, n);
			break;
		case RoundingMode::RoundTowardPositive:
			ok = exact_pow(pred, n) < target && target <= exact_pow(r, n);
			break;
		}
		efloat_rounding_mode = saved;
		return ok;
	}

	// cbrt(a) at P bits in every mode: it brackets the exact root as the mode requires, and
	// cbrt(-a) = -cbrt(a) rounded the other way; returns the failures
	template<unsigned nlimbs>
	int CheckCbrt(const sw::universal::efloat<nlimbs>& a, unsigned P, bool reportTestCases) {
		using namespace sw::universal;
		using E = efloat<nlimbs>;
		int failures = 0;
		for (RoundingMode mode : all_modes) {
			efloat_rounding_mode = mode;
			const E r = cbrt(a);
			const RoundingMode other = (mode == RoundingMode::RoundTowardPositive) ? RoundingMode::RoundTowardNegative
			                         : (mode == RoundingMode::RoundTowardNegative) ? RoundingMode::RoundTowardPositive : mode;
			efloat_rounding_mode = other;
			const E mirrored_mode_result = -cbrt(-a);
			efloat_rounding_mode = mode;
			if (r.get_precision() != P || !brackets_root(r, a, 3, mode) || !same(r, mirrored_mode_result)) {
				if (reportTestCases) std::cout << "    FAIL: cbrt(" << double(a) << ") at " << P << " bits, mode " << int(mode) << '\n';
				++failures;
			}
		}
		efloat_rounding_mode = RoundingMode::RoundToNearest;
		return failures;
	}

	// cbrt of P-bit arguments over 2^-300 .. 2^300, exact cubes among them, in every mode
	// (efloat_mpn::cbrtrem, which decides them since #1800, is checked in arithmetic/limb_kernels.cpp)
	template<unsigned nlimbs>
	int VerifyCbrtRounding(bool reportTestCases, unsigned P, int count) {
		using namespace sw::universal;
		using E = efloat<nlimbs>;
		int failures = 0;
		const RoundingMode saved = efloat_rounding_mode;
		std::mt19937_64 rng(1783);
		for (int i = 0; i < count; ++i) {
			efloat_rounding_mode = RoundingMode::RoundToNearest;
			E a = full_width<nlimbs>(rng, P, static_cast<int64_t>(rng() % 601) - 300);
			if (i % 4 == 1) {   // an exact cube: a root of up to P / 3 bits
				E s = full_width<nlimbs>(rng, P, static_cast<int64_t>(rng() % 201) - 100);
				s.set_precision(std::max(32u, P / 3));
				a = exact_pow(s, 3);
				a.set_precision(P);
			}
			failures += CheckCbrt(a, P, reportTestCases && failures < 8);
		}
		efloat_rounding_mode = saved;
		return failures;
	}

	// cbrt of short arguments at P bits, where a root of a sixth of the limbs decides whether they
	// are exact cubes (#1800): cubes of short roots and the integers next to them, scaled
	template<unsigned nlimbs>
	int VerifyCbrtShortArguments(bool reportTestCases, unsigned P) {
		using namespace sw::universal;
		using E = efloat<nlimbs>;
		int failures = 0;
		const RoundingMode saved = efloat_rounding_mode;
		for (int v : { 2, 3, 7, 8, 9, 26, 27, 28, 124, 125, 126, 999, 1000, 1001, 3375, 1030301 }) {
			for (int64_t k : { int64_t(0), int64_t(-30), int64_t(61) }) {
				E x(v);
				x.set_precision(P);
				x.setexponent(x.scale() + k);
				failures += CheckCbrt(x, P, reportTestCases && failures < 8);
			}
		}
		efloat_rounding_mode = saved;
		return failures;
	}

	// cbrt of 96-bit arguments whose root lies a unit or two below a rounding boundary at the
	// root's 127 bits, where cbrtrem's root without its remainder lands on or past it, so only
	// the exact root and its remainder round them right (#1800). Found by search, about one in
	// 2^31 arguments: m = M 2^-95, for the 96-bit M below.
	int VerifyCbrtNearBoundaries(bool reportTestCases) {
		using namespace sw::universal;
		using E = efloat<3>;
		const uint32_t significands[][3] = {
			{ 0x96c8245au, 0xf5a17aecu, 0x54dd6773u },   // the root 2 units below a 96-bit value
			{ 0x86f9225du, 0x50c14747u, 0xd08a82d2u },   // 1 unit below a 96-bit value
			{ 0xc4481627u, 0x8d37f196u, 0xebae6504u },   // 1 unit below a midpoint
		};
		int failures = 0;
		const RoundingMode saved = efloat_rounding_mode;
		for (const auto& M : significands) {
			E a(1);
			a.set_precision(96);
			for (unsigned i = 0; i < 3; ++i) a.setlimb(i, M[i]);
			a.setexponent(0);
			a.set_precision(96);
			for (int64_t k : { int64_t(0), int64_t(3), int64_t(-300) }) {   // and scaled by 8^k
				E x(a);
				x.setexponent(k);
				failures += CheckCbrt(x, 96, reportTestCases && failures < 8);
			}
		}
		efloat_rounding_mode = saved;
		return failures;
	}

	// hypot of P-bit arguments whose scales differ by up to 3P bits, across the gap where y stops
	// mattering beyond a nudge, in every mode
	template<unsigned nlimbs>
	int VerifyHypotRounding(bool reportTestCases, unsigned P, int count) {
		using namespace sw::universal;
		using E = efloat<nlimbs>;
		int failures = 0;
		const RoundingMode saved = efloat_rounding_mode;
		std::mt19937_64 rng(1783);
		for (int i = 0; i < count; ++i) {
			efloat_rounding_mode = RoundingMode::RoundToNearest;
			const int64_t ex = static_cast<int64_t>(rng() % 201) - 100;
			E x = full_width<nlimbs>(rng, P, ex);
			E y = full_width<nlimbs>(rng, P, ex - static_cast<int64_t>(rng() % (3 * P)));
			if (i % 3 == 1) { x = E(3 * (1 + static_cast<int>(rng() % 1000))); x.set_precision(P); y = x * E(4) / 3; }   // a Pythagorean triple: exact
			if (i % 3 == 2) {
				// a short x, whose square has few bits, and y around the gap where the padding of the
				// exact sum, not the square's own width, decides whether y's square is replaced
				x = E(static_cast<int>(1 + rng() % 255));
				x.set_precision(P);
				x.setexponent(ex);
				y = full_width<nlimbs>(rng, P, ex - static_cast<int64_t>(P / 4 + rng() % (P + 16)));
			}
			if (rng() & 1) x = -x;
			if (rng() & 1) std::swap(x, y);
			E target = exact_pow(x, 2);
			target.set_precision(2 * (P + 64u) + 8 * P);   // holds both squares, up to 6P bits apart, and their sum
			target += exact_pow(y, 2);
			for (RoundingMode mode : all_modes) {
				efloat_rounding_mode = mode;
				const E r = hypot(x, y);
				if (r.get_precision() != P || !brackets_root(r, target, 2, mode)) {
					if (reportTestCases && failures < 8) std::cout << "    FAIL: hypot(" << double(x) << ", " << double(y) << ") at " << P << " bits, mode " << int(mode) << '\n';
					++failures;
				}
			}
		}
		efloat_rounding_mode = saved;
		return failures;
	}

	// fma(x, y, z) against x y + z formed exactly and rounded once, with z cancelling most of x y,
	// or lying up to 4P bits below it, in every mode
	template<unsigned nlimbs>
	int VerifyFmaRounding(bool reportTestCases, unsigned P, int count) {
		using namespace sw::universal;
		using E = efloat<nlimbs>;
		int failures = 0;
		const RoundingMode saved = efloat_rounding_mode;
		std::mt19937_64 rng(1783);
		for (int i = 0; i < count; ++i) {
			efloat_rounding_mode = RoundingMode::RoundToNearest;
			const E x = full_width<nlimbs>(rng, P, static_cast<int64_t>(rng() % 21) - 10);
			const E y = full_width<nlimbs>(rng, P, static_cast<int64_t>(rng() % 21) - 10);
			const int64_t exy = x.scale() + y.scale();
			E z = full_width<nlimbs>(rng, P, exy - static_cast<int64_t>(rng() % (4 * P)));
			if (i % 3 == 1) {   // z cancels x y's leading bits
				E xw(x), yw(y);
				xw.set_precision(2 * P + 64);
				yw.set_precision(2 * P + 64);
				z = -(xw * yw);
				z.set_precision(P);
			}
			E xs(x), ys(y);
			if (i % 4 == 2) {
				// short x and y, whose product has few bits, and z around the gap where the padding,
				// not the product's own width, decides whether z is replaced
				xs = E(static_cast<int>(1 + rng() % 255));
				ys = E(static_cast<int>(1 + rng() % 255));
				xs.set_precision(P);
				ys.set_precision(P);
				z = full_width<nlimbs>(rng, P, xs.scale() + ys.scale() - static_cast<int64_t>(rng() % (2 * P)));
			}
			unsigned result_precision = P;
			if (i % 4 == 3) {   // a wider z: the result takes the widest precision carried
				result_precision = P + 64;
				z = full_width<nlimbs>(rng, result_precision, exy - static_cast<int64_t>(rng() % P));
			}
			if (rng() & 1) z = -z;
			const unsigned width = 8 * P + 256;   // holds x y + z exactly for every z drawn
			E exact(xs);
			exact.set_precision(width);
			E yw(ys);
			yw.set_precision(width);
			exact *= yw;
			E zw(z);
			zw.set_precision(width);
			for (RoundingMode mode : all_modes) {
				efloat_rounding_mode = mode;
				E want = exact + zw;   // exact; its zero sign follows the mode, as fma's must
				want.set_precision(result_precision);
				if (!same(fma(xs, ys, z), want)) {
					if (reportTestCases && failures < 8) std::cout << "    FAIL: fma(" << double(x) << ", " << double(y) << ", " << double(z) << ") at " << P << " bits, mode " << int(mode) << '\n';
					++failures;
				}
			}
		}
		efloat_rounding_mode = saved;
		return failures;
	}

	// |x| mod |y| by binary long division, and the quotient's parity
	template<unsigned nlimbs>
	sw::universal::efloat<nlimbs> long_division_mod(const sw::universal::efloat<nlimbs>& x, const sw::universal::efloat<nlimbs>& y, bool& odd) {
		using namespace sw::universal;
		using E = efloat<nlimbs>;
		E ax(x), ay(y);
		ax.setsign(false);
		ay.setsign(false);
		const int64_t lo = std::min(efloat_detail::lowest_bit(ax), efloat_detail::lowest_bit(ay));
		const unsigned width = static_cast<unsigned>(std::max(ax.scale(), ay.scale()) - lo + 4);
		E r(ax);
		r.set_precision(width);
		odd = false;
		for (int64_t k = ax.scale() - ay.scale(); k >= 0; --k) {
			E t(ay);
			t.set_precision(width);
			t.setexponent(ay.scale() + k);
			const bool take = (r >= t);
			if (take) r -= t;
			if (k == 0) odd = take;
		}
		return r;
	}

	// fmod and remainder of P-bit arguments with quotients up to 2^(2P), in every mode
	template<unsigned nlimbs>
	int VerifyFmodRemainder(bool reportTestCases, unsigned P, int count) {
		using namespace sw::universal;
		using E = efloat<nlimbs>;
		int failures = 0;
		const RoundingMode saved = efloat_rounding_mode;
		std::mt19937_64 rng(1783);
		for (int i = 0; i < count; ++i) {
			efloat_rounding_mode = RoundingMode::RoundToNearest;
			const int64_t ey = static_cast<int64_t>(rng() % 41) - 20;
			E y = full_width<nlimbs>(rng, P, ey);
			if (i % 4 == 1) { y = E(static_cast<int>(1 + rng() % 1000)); y.set_precision(P); }   // a short divisor
			E x = full_width<nlimbs>(rng, P, ey + static_cast<int64_t>(rng() % (2 * P)) - 2);
			if (i % 4 == 2) { x = y * E(static_cast<int>(1 + rng() % 64)); x.set_precision(P); x.setexponent(x.scale() - 1); }   // halfway cases
			unsigned result_precision = P;
			if (i % 4 == 3) {
				// x = k y - 2^lowest_bit(y): x / y lies 2^-P or less below an integer, where a quotient
				// carried too narrow rounds up to it
				result_precision = P + 32;
				x = y;
				x.set_precision(result_precision);
				x *= static_cast<int>(2 + rng() % 1000);
				E last(1);
				last.setexponent(efloat_detail::lowest_bit(y));
				x -= last;
			}
			if (rng() & 1) x = -x;
			if (rng() & 1) y = -y;
			bool odd = false;
			E r = long_division_mod(x, y, odd);
			r.set_precision(result_precision);
			E ay(y);
			ay.setsign(false);
			E q = r;   // remainder: r moved into [-|y|/2, |y|/2], ties to an even quotient
			E twice(r);
			twice.set_precision(result_precision + 32);
			twice.setexponent(twice.iszero() ? 0 : twice.scale() + 1);
			if (!r.iszero() && (twice > ay || (twice == ay && odd))) { q.set_precision(result_precision + 32); q -= ay; q.set_precision(result_precision); }
			if (x.sign() == -1) { r = -r; q = -q; }
			if (r.iszero()) r.setsign(x.sign() == -1);
			if (q.iszero()) q.setsign(x.sign() == -1);
			for (RoundingMode mode : all_modes) {
				efloat_rounding_mode = mode;
				if (!same(fmod(x, y), r) || !same(remainder(x, y), q)) {
					if (reportTestCases && failures < 8) std::cout << "    FAIL: fmod/remainder(" << double(x) << ", " << double(y) << ") at " << P << " bits, mode " << int(mode) << '\n';
					++failures;
				}
			}
		}
		efloat_rounding_mode = saved;
		return failures;
	}

	// quotients far past any precision: 2^k mod 3 = 1 or 2 as k is even or odd, and 5 2^k mod 7
	// cycles through 5, 3, 6, for k up to 2^40
	template<unsigned nlimbs>
	int VerifyHugeQuotients(bool reportTestCases, unsigned P) {
		using namespace sw::universal;
		using E = efloat<nlimbs>;
		int failures = 0;
		std::mt19937_64 rng(1783);
		const E three(3), seven(7);
		for (int i = 0; i < 40; ++i) {
			const int64_t k = static_cast<int64_t>(rng() % (1ull << (rng() % 41)));
			E x(1);
			x.set_precision(P);
			x.setexponent(k);
			const int64_t m3 = (k % 2 == 0) ? 1 : 2;
			const int64_t m7[] = { 5, 3, 6 };
			E x5(5);
			x5.set_precision(P);
			x5.setexponent(x5.scale() + k);
			const bool ok = double(fmod(x, three)) == double(m3) && double(remainder(x, three)) == double(m3 == 1 ? 1 : -1) &&
			                double(fmod(x5, seven)) == double(m7[k % 3]) && double(fmod(-x, three)) == -double(m3);
			if (!ok) {
				if (reportTestCases && failures < 8) std::cout << "    FAIL: fmod/remainder(2^" << k << ", 3 or 7)\n";
				++failures;
			}
		}
		return failures;
	}

	// IEEE special values, zero signs and precisions
	int VerifySpecialValues(bool reportTestCases) {
		using namespace sw::universal;
		using E = efloat<4>;
		int failures = 0;
		const RoundingMode saved = efloat_rounding_mode;
		auto check = [&](bool ok, const char* what) {
			if (!ok) {
				if (reportTestCases) std::cout << "    FAIL: " << what << '\n';
				++failures;
			}
		};
		const E inf = std::numeric_limits<E>::infinity();
		const E nan = std::numeric_limits<E>::quiet_NaN();
		E one(1), zero(0), two(2), three(3);
		for (E* v : { &one, &zero, &two, &three }) v->set_precision(96);
		E third(one);
		third /= 3;

		check(fma(inf, zero, one).isnan(), "fma(inf, 0, 1) is NaN");
		check(fma(inf, one, -inf).isnan(), "fma(inf, 1, -inf) is NaN");
		check(fma(inf, one, one) == inf, "fma(inf, 1, 1) is inf");
		efloat_rounding_mode = RoundingMode::RoundTowardNegative;
		const E rd = fma(one, one, -one);
		efloat_rounding_mode = RoundingMode::RoundToNearest;
		const E rn = fma(one, one, -one);
		check(rd.iszero() && rd.sign() == -1, "fma(1, 1, -1) is -0 rounding down");
		check(rn.iszero() && rn.sign() == 1, "fma(1, 1, -1) is +0 to nearest");
		check(fma(third, three, zero).get_precision() == 96, "fma keeps the precision");

		check(hypot(inf, nan) == inf && hypot(nan, -inf) == inf, "hypot(inf, NaN) is inf");
		check(hypot(nan, one).isnan(), "hypot(NaN, 1) is NaN");
		const E h0 = hypot(-zero, zero);
		check(h0.iszero() && h0.sign() == 1, "hypot(-0, 0) is +0");
		check(same(hypot(-third, zero), -(-third)), "hypot(x, 0) is |x|");
		check(double(hypot(E(3), E(4))) == 5.0, "hypot(3, 4) is 5");

		check(double(cbrt(E(-8))) == -2.0 && double(cbrt(E(27))) == 3.0, "exact cube roots");
		check(cbrt(-zero).iszero() && cbrt(-zero).sign() == -1, "cbrt(-0) is -0");
		check(cbrt(-inf) == -inf && cbrt(nan).isnan(), "cbrt(-inf), cbrt(NaN)");

		check(fmod(inf, one).isnan() && fmod(one, zero).isnan() && fmod(nan, one).isnan(), "fmod NaN cases");
		check(same(fmod(third, inf), third) && same(remainder(third, -inf), third), "fmod and remainder by inf return x");
		const E fz = fmod(-three, three);
		check(fz.iszero() && fz.sign() == -1, "fmod(-3, 3) is -0");
		check(remainder(inf, one).isnan() && remainder(one, zero).isnan(), "remainder NaN cases");
		check(double(remainder(E(5), E(2))) == 1.0 && double(remainder(E(7), E(2))) == -1.0, "remainder ties to even");
		check(double(fmod(E(5.5), -E(2))) == 1.5 && double(fmod(-E(5.5), E(2))) == -1.5, "fmod takes x's sign");
		efloat_rounding_mode = saved;
		return failures;
	}

}  // namespace

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main() try {
	using namespace sw::universal;

	std::string test_suite          = "efloat cbrt, hypot, fma, fmod and remainder";
	std::string test_tag            = "exact_functions";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyCbrtRounding<4>(reportTestCases, 128, 20), "efloat<4>", "cbrt");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifySpecialValues(reportTestCases), "efloat<4>", "special values");
	nrOfFailedTestCases += ReportTestResult(VerifyCbrtRounding<4>(reportTestCases, 128, 200), "efloat<4>", "cbrt");
	nrOfFailedTestCases += ReportTestResult(VerifyHypotRounding<4>(reportTestCases, 128, 200), "efloat<4>", "hypot");
	nrOfFailedTestCases += ReportTestResult(VerifyFmaRounding<4>(reportTestCases, 128, 200), "efloat<4>", "fma");
	nrOfFailedTestCases += ReportTestResult(VerifyFmodRemainder<4>(reportTestCases, 128, 200), "efloat<4>", "fmod, remainder");
	nrOfFailedTestCases += ReportTestResult(VerifyHugeQuotients<4>(reportTestCases, 128), "efloat<4>", "fmod, remainder of huge quotients");
	nrOfFailedTestCases += ReportTestResult(VerifyCbrtRounding<1>(reportTestCases, 32, 200), "efloat<1>", "cbrt");
	// roots of 2 and 4 limbs, where cbrtrem without its remainder can lie a unit or two high (#1800)
	nrOfFailedTestCases += ReportTestResult(VerifyCbrtRounding<3>(reportTestCases, 96, 200), "efloat<3>", "cbrt");
	nrOfFailedTestCases += ReportTestResult(VerifyCbrtRounding<7>(reportTestCases, 224, 200), "efloat<7>", "cbrt");
	nrOfFailedTestCases += ReportTestResult(VerifyCbrtNearBoundaries(reportTestCases), "efloat<3>", "cbrt next to rounding boundaries");
	nrOfFailedTestCases += ReportTestResult(VerifyCbrtShortArguments<16>(reportTestCases, 512), "efloat<16>", "cbrt of short arguments");
	nrOfFailedTestCases += ReportTestResult(VerifyCbrtShortArguments<64>(reportTestCases, 2048), "efloat<64>", "cbrt of short arguments");
	nrOfFailedTestCases += ReportTestResult(VerifyHypotRounding<1>(reportTestCases, 32, 200), "efloat<1>", "hypot");
	nrOfFailedTestCases += ReportTestResult(VerifyFmaRounding<1>(reportTestCases, 32, 200), "efloat<1>", "fma");
	nrOfFailedTestCases += ReportTestResult(VerifyFmodRemainder<1>(reportTestCases, 32, 200), "efloat<1>", "fmod, remainder");
#	endif

#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyCbrtRounding<16>(reportTestCases, 512, 60), "efloat<16>", "cbrt");
	nrOfFailedTestCases += ReportTestResult(VerifyHypotRounding<16>(reportTestCases, 512, 60), "efloat<16>", "hypot");
	nrOfFailedTestCases += ReportTestResult(VerifyFmaRounding<16>(reportTestCases, 512, 60), "efloat<16>", "fma");
	nrOfFailedTestCases += ReportTestResult(VerifyFmodRemainder<16>(reportTestCases, 512, 60), "efloat<16>", "fmod, remainder");
#	endif

#	if REGRESSION_LEVEL_3
#	endif

#	if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyCbrtRounding<64>(reportTestCases, 2048, 10), "efloat<64>", "cbrt");
	nrOfFailedTestCases += ReportTestResult(VerifyHypotRounding<64>(reportTestCases, 2048, 10), "efloat<64>", "hypot");
	nrOfFailedTestCases += ReportTestResult(VerifyFmaRounding<64>(reportTestCases, 2048, 10), "efloat<64>", "fma");
	nrOfFailedTestCases += ReportTestResult(VerifyFmodRemainder<64>(reportTestCases, 2048, 10), "efloat<64>", "fmod, remainder");
#	endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif
}
catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
