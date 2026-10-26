// atan_kernel.cpp: regression tests for efloat's fixed-point arctangent kernel (#1792)
//
// atan, asin, acos and atan2 evaluate atan(a) = y S(z) in atan_working: a reflected below 1 and
// halved to y, and S(z) = 1 - z/3 + z^2/5 - ... summed in fixed point by rectangular splitting
// (fixed_series::arctangent_series in math/exponent.hpp). Ziv's strategy retries an evaluation it
// cannot round, so a kernel whose error broke its bound could still give correctly rounded
// results, only slower: the rounded results do not test the bound. This test does, against
// references that share none of the kernel's code -- Euler's series
//     atan x = x / (1 + x^2) sum_n (2n)!! / (2n + 1)!! (x^2 / (1 + x^2))^n,   |x| <= 1,
// in efloat arithmetic at 3P + 256 bits, reflected with pi/2 above 1:
//   - atan_working, asin_working, acos_working and atan2_working at P + exp_log_guard_bits bits,
//     within the declared 2^(scale - working + inverse_trig_error_bits);
//   - arguments near 0 (down past the public functions' round_near cut), near 1 either side of
//     the reflection, large, either side of the halving target 2^-T, and asin, acos near +-1.
//
// REGRESSION_LEVEL convention (intensity progression):
//   LEVEL 1 -- 10 precisions from 24 to 2048 bits, random arguments and the edges.
//   LEVEL 2 -- four times the random cases.
//   LEVEL 3 -- sixteen times, and 4096 bits.
//   LEVEL 4 -- sixty-four times, and 8192 bits.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <random>
#include <vector>
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using E = sw::universal::efloat<900>;

	enum class inverse { atan, asin, acos, atan2 };
	const char* name(inverse f) {
		static const char* names[] = { "atan", "asin", "acos", "atan2" };
		return names[static_cast<int>(f)];
	}

	E half_pi(unsigned bits) {
		E hp = sw::universal::efloat_detail::pi<900>(bits);
		hp.setexponent(hp.scale() - 1);
		return hp;
	}

	// atan x at `bits` by Euler's series, |x| <= 1 directly and pi/2 - atan(1/x) above
	E reference_atan(const E& x, unsigned bits) {
		E a(x);
		a.setsign(false);
		a.set_precision(bits);
		const bool reflect = a > E(1);
		if (reflect) a = 1 / a;
		const E a2 = a * a;
		E d = a2 + 1;
		const E u = a2 / d;   // at most 1/2
		E term = a / d, sum = term;
		for (unsigned n = 1; ; ++n) {
			term = term * u * (2 * n) / (2 * n + 1);
			if (term.iszero() || term.scale() < sum.scale() - static_cast<int64_t>(bits) - 4) break;
			sum += term;
		}
		E r = reflect ? half_pi(bits) - sum : sum;
		if (x.sign() == -1) r = -r;
		return r;
	}

	// asin x = atan(x / sqrt((1 - x)(1 + x))) for |x| < 1
	E reference_asin(const E& x, unsigned bits) {
		E xw(x);
		xw.set_precision(bits);
		const E one(1);
		return reference_atan(xw / sqrt((one - xw) * (one + xw)), bits);
	}

	// acos x = pi/2 - asin x, at the wider width the cancellation near 1 needs
	E reference_acos(const E& x, unsigned bits) {
		return half_pi(bits) - reference_asin(x, bits);
	}

	E reference_atan2(const E& y, const E& x, unsigned bits) {
		E yw(y), xw(x);
		yw.set_precision(bits);
		xw.set_precision(bits);
		E q = yw / xw;
		q.setsign(false);
		E r = reference_atan(q, bits);
		if (x.sign() == -1) r = 2 * half_pi(bits) - r;
		if (y.sign() == -1) r = -r;
		return r;
	}

	// a full-width P-bit value in [1, 2) times 2^e, of either sign
	E random_value(std::mt19937_64& rng, unsigned P, int64_t e) {
		E v(1);
		v.set_precision(P);
		for (unsigned i = 0; i < (P + 31) / 32; ++i) v.setlimb(i, static_cast<uint32_t>(rng()) | (i == 0 ? 0x80000000u : 0u));
		v.set_precision(P);
		v.setexponent(e);
		return (rng() & 1u) ? -v : v;
	}

	// 1 - d or 1 + d for d = 2^-e times a random P-bit value: next to the reflection, or to +-1
	E near_one(std::mt19937_64& rng, unsigned P, int64_t e, bool above) {
		E d = random_value(rng, P, -e);
		d.setsign(false);
		E one(1);
		one.set_precision(P);
		E v = above ? one + d : one - d;
		v.set_precision(P);
		return v;
	}

	bool within_bound(inverse f, const E& x, const E& y, unsigned P, bool reportTestCases) {
		using namespace sw::universal::efloat_detail;
		const unsigned working = P + exp_log_guard_bits;
		const unsigned bits    = 3 * P + 256;
		E w, ref;
		switch (f) {
		case inverse::atan:  w = atan_working(x, working);     ref = reference_atan(x, bits);     break;
		case inverse::asin:  w = asin_working(x, working);     ref = reference_asin(x, bits);     break;
		case inverse::acos:  w = acos_working(x, working);     ref = reference_acos(x, bits);     break;
		case inverse::atan2: w = atan2_working(y, x, working); ref = reference_atan2(y, x, bits); break;
		}
		const E err = w - ref;
		if (err.iszero()) return true;
		const int64_t bound = w.scale() - static_cast<int64_t>(working) + static_cast<int64_t>(inverse_trig_error_bits);
		const bool ok = err.scale() < bound;
		if (!ok && reportTestCases) {
			std::cerr << "FAIL: " << name(f) << "_working at P = " << P << ", x = " << double(x) << " (scale " << x.scale() << ")"
			          << ": error 2^" << err.scale() << ", bound 2^" << bound << '\n';
		}
		return ok;
	}

	int VerifyAtanKernel(const std::vector<unsigned>& precisions, unsigned cases, bool reportTestCases) {
		using namespace sw::universal::efloat_detail;
		int fails = 0;
		std::mt19937_64 rng(1792);
		const E none(0);
		for (unsigned P : precisions) {
			const unsigned n = (P > 1024) ? std::max(4u, cases / 4u) : cases;
			const int64_t  T = static_cast<int64_t>(arctangent_target(P + exp_log_guard_bits));
			auto check = [&](inverse f, const E& x, const E& y) { if (!within_bound(f, x, y, P, reportTestCases)) ++fails; };
			for (unsigned c = 0; c < n; ++c) {
				// atan and atan2 over every scale: tiny (above the round_near cut), around 1, large
				const int64_t e = (c % 4 == 0) ? -static_cast<int64_t>(rng() % (P / 2 + 1)) - 1
				                : (c % 4 == 1) ? static_cast<int64_t>(rng() % 64)
				                               : static_cast<int64_t>(rng() % 8) - 5;
				const E x = random_value(rng, P, e);
				check(inverse::atan, x, none);
				check(inverse::atan2, random_value(rng, P, static_cast<int64_t>(rng() % 12) - 6), x);
				// asin and acos inside (-1, 1)
				const E s = random_value(rng, P, -1 - static_cast<int64_t>(rng() % 10));
				check(inverse::asin, s, none);
				check(inverse::acos, s, none);
			}
			for (int64_t e : { int64_t(1), int64_t(4), int64_t(P / 2), int64_t(P) - 1 }) {
				for (bool above : { false, true }) {
					// either side of the reflection at 1
					check(inverse::atan, near_one(rng, P, e, above), none);
					if (!above) {   // asin and acos next to +1 and -1
						const E s = near_one(rng, P, e, false);
						check(inverse::asin, s, none);
						check(inverse::acos, s, none);
						check(inverse::asin, -s, none);
						check(inverse::acos, -s, none);
					}
				}
			}
			// either side of the halving target: |a| just below 2^-T needs no halving, just above one
			for (int64_t t : { -T - 1, -T, -T + 1 }) {
				for (int c = 0; c < 2; ++c) check(inverse::atan, random_value(rng, P, t), none);
			}
			// tiny, below the public functions' round_near cut, as atan2(y, x < 0) passes a tiny |y / x|;
			// at 2 e ~ -(P + 34), z = a^2 sits in the lowest limbs of the series' fixed point, where
			// its z / 3 is still above the bound
			const int64_t Ps = static_cast<int64_t>(P);
			for (int64_t e : { -Ps / 2 - 8, -(Ps + 34) / 2, -(Ps + 40) / 2, -Ps - 8, -Ps - 40, -2 * Ps - 100 }) {
				check(inverse::atan, random_value(rng, P, e), none);
			}
			for (double v : { 0.5, 0.75, -0.9999, 1e-6, 2.0, 1e10, -1e300 }) {
				E x(v);
				x.set_precision(P);
				check(inverse::atan, x, none);
			}
		}
		return fails;
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

	std::string test_suite          = "efloat fixed-point arctangent kernel";
	std::string test_tag            = "atan kernel";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	const std::vector<unsigned> precisions = { 24, 32, 53, 64, 100, 128, 256, 512, 1000, 2048 };

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyAtanKernel({ 128 }, 20, reportTestCases), "efloat", "atan kernel 128 bits");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyAtanKernel(precisions, 16, reportTestCases), "efloat", "error bounds, 24 to 2048 bits");
#	endif
#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyAtanKernel(precisions, 64, reportTestCases), "efloat", "error bounds, 4x cases");
#	endif
#	if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyAtanKernel({ 4096 }, 32, reportTestCases), "efloat", "error bounds, 4096 bits");
	nrOfFailedTestCases += ReportTestResult(VerifyAtanKernel(precisions, 256, reportTestCases), "efloat", "error bounds, 16x cases");
#	endif
#	if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyAtanKernel({ 8192 }, 32, reportTestCases), "efloat", "error bounds, 8192 bits");
	nrOfFailedTestCases += ReportTestResult(VerifyAtanKernel(precisions, 1024, reportTestCases), "efloat", "error bounds, 64x cases");
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
