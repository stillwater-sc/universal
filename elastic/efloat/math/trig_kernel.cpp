// trig_kernel.cpp: regression tests for efloat's fixed-point versine kernel (#1790)
//
// sin, cos and tan evaluate v = 1 - cos r for the reduced argument r in fixed point: the series
// for v / y^2 in z = y^2 on the series engine exp uses, and v doubled back in a scaled fixed
// point (math/trigonometry.hpp, versine_with). Ziv's strategy retries an evaluation it cannot
// round, so a kernel whose error broke its bound could still give correctly rounded results,
// only slower: the rounded results do not test the bound. This test does, against references
// that share none of the kernel's code -- the plain Taylor series in efloat arithmetic at
// 2P + 256 bits:
//   - versine(r) within one ulp of 1 - cos r at r's precision, for |r| <= pi/4 of either sign,
//     from r near pi/4 down to r below 2^-P;
//   - trig_working(x, P), for sin, cos and tan, within its declared bound
//     2^(scale - P - exp_log_guard_bits + trig_error_bits), for x inside and outside pi/4, near
//     k pi/2, and either side of 2^40, where the reduction stops taking k from double; reduced
//     for the reference by pi/2 carried at 3P + 256 bits;
//   - sin, cos and tan of tiny x either side of the cut below which they round from x or 1
//     directly, correctly rounded in all four modes.
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

	using E = sw::universal::efloat<300>;
	using sw::universal::efloat_detail::trig_function;

	// 1 - cos r = r^2/2! - r^4/4! + ... and sin r = r - r^3/3! + ..., for |r| <= pi/4, at `bits`
	E reference_versine(const E& r, unsigned bits) {
		E r2 = r * r;
		r2.set_precision(bits);
		E term = r2 / 2, sum = term;
		term.set_precision(bits);
		sum.set_precision(bits);
		for (unsigned k = 2; ; ++k) {
			term = -(term * r2) / ((2 * k - 1) * (2 * k));
			if (term.iszero() || term.scale() < sum.scale() - static_cast<int64_t>(bits) - 4) break;
			sum += term;
		}
		return sum;
	}

	E reference_sine(const E& r, unsigned bits) {
		E x(r);
		x.set_precision(bits);
		const E r2 = x * x;
		E term = x, sum = x;
		for (unsigned k = 1; ; ++k) {
			term = -(term * r2) / ((2 * k) * (2 * k + 1));
			if (term.iszero() || term.scale() < sum.scale() - static_cast<int64_t>(bits) - 4) break;
			sum += term;
		}
		return sum;
	}

	// sin, cos or tan of x by reduction with pi/2 at `bits` bits and the reference series
	E reference_trig(const E& x, unsigned bits, trig_function f) {
		using namespace sw::universal;
		E xw(x);
		xw.set_precision(bits);
		E half_pi = efloat_detail::pi<300>(bits);
		half_pi.setexponent(half_pi.scale() - 1);
		const E k = round(xw / half_pi);
		const E r = xw - k * half_pi;
		const int64_t q = static_cast<int64_t>(double(k));
		const unsigned quadrant = static_cast<unsigned>(((q % 4) + 4) % 4);
		E one(1);
		one.set_precision(bits);
		const E s = reference_sine(r, bits), c = one - reference_versine(r, bits);
		switch (f) {
		case trig_function::sine:   return (quadrant == 0) ? s : (quadrant == 1) ? c : (quadrant == 2) ? -s : -c;
		case trig_function::cosine: return (quadrant == 0) ? c : (quadrant == 1) ? -s : (quadrant == 2) ? -c : s;
		default:                    return (quadrant % 2 == 0) ? s / c : -(c / s);
		}
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

	bool versine_within_bound(const E& r, bool reportTestCases) {
		const E v   = sw::universal::efloat_detail::versine(r);
		const E ref = reference_versine(r, 2 * r.get_precision() + 256);
		const E err = v - ref;
		if (err.iszero()) return true;
		const int64_t bound = ref.scale() + 1 - static_cast<int64_t>(r.get_precision());   // one ulp at r's precision
		const bool ok = err.scale() < bound;
		if (!ok && reportTestCases) {
			std::cerr << "FAIL: versine at " << r.get_precision() << " bits, r = " << double(r) << " (scale " << r.scale()
			          << "): error 2^" << err.scale() << ", bound 2^" << bound << '\n';
		}
		return ok;
	}

	bool trig_within_bound(const E& x, unsigned P, trig_function f, bool reportTestCases) {
		using namespace sw::universal::efloat_detail;
		const E w = trig_working(x, P, f);
		if (w.isinf()) return true;   // tan at a pole the reduction cannot resolve
		const unsigned bits = 3 * P + 256 + static_cast<unsigned>(std::max<int64_t>(0, x.scale()));
		const E ref = reference_trig(x, bits, f);
		const E err = w - ref;
		if (err.iszero()) return true;
		const int64_t bound = w.scale() - static_cast<int64_t>(P) - static_cast<int64_t>(exp_log_guard_bits) + static_cast<int64_t>(trig_error_bits);
		const bool ok = err.scale() < bound;
		if (!ok && reportTestCases) {
			static const char* name[] = { "sin", "cos", "tan" };
			std::cerr << "FAIL: " << name[static_cast<int>(f)] << " trig_working at P = " << P << ", x = " << double(x)
			          << ": error 2^" << err.scale() << ", bound 2^" << bound << '\n';
		}
		return ok;
	}

	int VerifyTrigKernel(const std::vector<unsigned>& precisions, unsigned cases, bool reportTestCases) {
		int fails = 0;
		std::mt19937_64 rng(1790);
		for (unsigned P : precisions) {
			const unsigned n = (P > 1024) ? std::max(4u, cases / 4u) : cases;
			// the versine: r of r's own precision P + 64, as the reduction leaves it
			for (unsigned c = 0; c < n; ++c) {
				const int64_t e = (c % 4 == 0) ? -static_cast<int64_t>(rng() % (P + 8)) : -1 - static_cast<int64_t>(rng() % 12);
				E r = random_value(rng, P + 64, e);
				if (r.scale() == -1 && std::fabs(double(r)) > 0.785) r.setexponent(-2);
				if (!versine_within_bound(r, reportTestCases)) ++fails;
			}
			for (double v : { 0.7853, -0.7853, 0.5, 1e-3, -1e-9 }) {
				E r(v);
				r.set_precision(P + 64);
				if (!versine_within_bound(r, reportTestCases)) ++fails;
			}
			// sin, cos and tan through the reduction
			for (unsigned c = 0; c < n; ++c) {
				const int64_t e = (c % 5 == 0) ? -static_cast<int64_t>(rng() % (P / 2 + 1)) : static_cast<int64_t>(rng() % 12) - 6;
				const E x = random_value(rng, P, e);
				for (int f = 0; f < 3; ++f) if (!trig_within_bound(x, P, trig_function(f), reportTestCases)) ++fails;
			}
			// next to a half-integer multiple of pi/2, where k from double may be the neighbour of the
			// nearest integer and |r| lies just past pi/4: either side, small k and k near 2^39
			for (double k : { 0.5, 1.5, 2.5, -3.5, 1000.5, 549755813887.5 }) {
				E hp = sw::universal::efloat_detail::pi<300>(P + 64);
				hp.setexponent(hp.scale() - 1);
				for (int side : { -1, 1 }) {
					E x = E(k) * hp;
					x.set_precision(P);
					E nudge = x;
					nudge.setexponent(x.scale() - 50);   // 2^-50 off the half-integer, relative: as close as double resolves x 2/pi
					x = (side > 0) ? x + nudge : x - nudge;
					x.set_precision(P);
					for (int f = 0; f < 3; ++f) if (!trig_within_bound(x, P, trig_function(f), reportTestCases)) ++fails;
				}
			}
			// past 2^40, where the reduction divides by pi/2 instead of taking k from double
			for (int64_t e : { 39, 40, 45 }) {
				const E x = random_value(rng, P, e);
				for (int f = 0; f < 3; ++f) if (!trig_within_bound(x, P, trig_function(f), reportTestCases)) ++fails;
			}
			for (int k : { 1, 2, 3, 7, -5, 40 }) {   // near k pi/2: r small, either sign
				E x(1.5707963267948966 * k);
				x.set_precision(P);
				for (int f = 0; f < 3; ++f) if (!trig_within_bound(x, P, trig_function(f), reportTestCases)) ++fails;
			}
		}
		return fails;
	}

	// sin, cos and tan of tiny x either side of the cut below which they round from x or 1
	// directly (round_near): 2 scale(x) < -P - 3 for sin and < -P - 4 for cos and tan (#1790).
	// Each result must be the reference rounded once to the result's precision, in every mode.
	int VerifyTinyCuts(bool reportTestCases) {
		using namespace sw::universal;
		int fails = 0;
		std::mt19937_64 rng(17902);
		const RoundingMode modes[] = { RoundingMode::RoundToNearest, RoundingMode::RoundToZero,
		                               RoundingMode::RoundTowardPositive, RoundingMode::RoundTowardNegative };
		const RoundingMode saved = efloat_rounding_mode;
		for (unsigned P : { 32u, 64u, 128u, 256u, 512u }) {
			for (int f = 0; f < 3; ++f) {
				const int64_t cut = -static_cast<int64_t>(P) - ((f == 0) ? 3 : 4);   // 2 scale(x) < cut rounds directly
				for (int64_t s2 = cut - 2; s2 <= cut + 6; ++s2) {   // round directly below the cut, by Ziv above it
					for (int c = 0; c < 8; ++c) {
						E x = random_value(rng, P, (s2 >= 0) ? s2 / 2 : -((1 - s2) / 2));
						if (c == 0) { x = E(1); x.set_precision(P); x.setexponent((s2 >= 0) ? s2 / 2 : -((1 - s2) / 2)); }   // a power of 2
						const E ref = reference_trig(x, 3 * P + 256, trig_function(f));
						for (RoundingMode m : modes) {
							efloat_rounding_mode = m;
							const E y = (f == 0) ? sin(x) : (f == 1) ? cos(x) : tan(x);
							E expected(ref);
							expected.set_precision(y.get_precision());   // rounded once, in mode m
							efloat_rounding_mode = saved;
							if (!(y == expected)) {
								++fails;
								if (reportTestCases) {
									static const char* name[] = { "sin", "cos", "tan" };
									std::cerr << "FAIL: " << name[f] << " at P = " << P << ", 2 scale(x) = " << 2 * x.scale()
									          << " (cut " << cut << "), mode " << static_cast<int>(m) << '\n';
								}
							}
						}
					}
				}
			}
		}
		efloat_rounding_mode = saved;
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

	std::string test_suite          = "efloat fixed-point versine kernel";
	std::string test_tag            = "trig kernel";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	const std::vector<unsigned> precisions = { 24, 32, 53, 64, 100, 128, 256, 512, 1000, 2048 };

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyTrigKernel({ 128 }, 20, reportTestCases), "efloat", "trig kernel 128 bits");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyTrigKernel(precisions, 16, reportTestCases), "efloat", "error bounds, 24 to 2048 bits");
	nrOfFailedTestCases += ReportTestResult(VerifyTinyCuts(reportTestCases), "efloat", "tiny x either side of the round_near cuts");
#	endif
#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyTrigKernel(precisions, 64, reportTestCases), "efloat", "error bounds, 4x cases");
#	endif
#	if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyTrigKernel({ 4096 }, 32, reportTestCases), "efloat", "error bounds, 4096 bits");
	nrOfFailedTestCases += ReportTestResult(VerifyTrigKernel(precisions, 256, reportTestCases), "efloat", "error bounds, 16x cases");
#	endif
#	if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyTrigKernel({ 8192 }, 32, reportTestCases), "efloat", "error bounds, 8192 bits");
	nrOfFailedTestCases += ReportTestResult(VerifyTrigKernel(precisions, 1024, reportTestCases), "efloat", "error bounds, 64x cases");
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
