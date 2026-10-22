// exp_kernel.cpp: regression tests for efloat's fixed-point exp kernel (#1788)
//
// exp_working(x, P) sums the series for exp in fixed point by rectangular splitting and squares
// it back (math/exponent.hpp, exp_fixed). Ziv's strategy retries an evaluation it cannot round,
// so a kernel whose error broke its bound could still give correctly rounded results, only
// slower: the rounded results do not test the bound. This test does. It compares exp_working
// with a reference that shares none of the kernel's code -- the plain Taylor series in efloat
// arithmetic at 2P + 256 bits, on x halved below 2^-8 and squared back -- and checks the declared
// bound 2^(scale - P - exp_log_guard_bits + exp_error_bits).
//
// REGRESSION_LEVEL convention (intensity progression):
//   LEVEL 1 -- 12 precisions from 24 to 2048 bits, random x of either sign at scales -40 to 6,
//              and the kernel's edges: x near k ln 2 (r ~ 0, either sign), x tiny, |r| ~ ln 2 / 2.
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

	// exp x at `bits` bits by the plain series: x / 2^j below 2^-8, summed term by term, squared j times
	E reference_exp(const E& x, unsigned bits) {
		E y(x);
		y.set_precision(bits);
		const int64_t j = std::max<int64_t>(0, y.scale() + 9);
		y.setexponent(y.scale() - j);
		E sum(1), term(1);
		sum.set_precision(bits);
		term.set_precision(bits);
		for (unsigned k = 1; ; ++k) {
			term = term * y / k;
			if (term.iszero() || term.scale() < sum.scale() - static_cast<int64_t>(bits) - 4) break;
			sum += term;
		}
		for (int64_t i = 0; i < j; ++i) sum = sum * sum;
		return sum;
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

	// whether exp_working(x, P) lies within its declared bound of exp x
	bool within_bound(const E& x, unsigned P, bool reportTestCases) {
		using namespace sw::universal::efloat_detail;
		const E w   = exp_working(x, P);
		const E ref = reference_exp(x, 2 * P + 256);
		E err = w - ref;
		if (err.iszero()) return true;
		const int64_t bound = w.scale() - static_cast<int64_t>(P) - static_cast<int64_t>(exp_log_guard_bits) + static_cast<int64_t>(exp_error_bits);
		const bool ok = err.scale() < bound;
		if (!ok && reportTestCases) {
			std::cerr << "FAIL: exp_working at P = " << P << ", x = " << double(x) << " (scale " << x.scale()
			          << "): error 2^" << err.scale() << ", bound 2^" << bound << '\n';
		}
		return ok;
	}

	int VerifyExpKernel(const std::vector<unsigned>& precisions, unsigned cases, bool reportTestCases) {
		int fails = 0;
		std::mt19937_64 rng(1788);
		const double ln2 = 0.6931471805599453;
		for (unsigned P : precisions) {
			const unsigned n = (P > 1024) ? std::max(4u, cases / 4u) : cases;
			for (unsigned c = 0; c < n; ++c) {
				const int64_t e = static_cast<int64_t>(rng() % 47) - 40;   // scales -40 .. 6
				if (!within_bound(random_value(rng, P, e), P, reportTestCases)) ++fails;
			}
			// x near k ln 2: r ~ 0 of either sign, the alternating and the positive series
			for (int k : { -40, -3, -1, 1, 2, 17 }) {
				E x(static_cast<double>(k) * ln2);
				x.set_precision(P);
				if (!within_bound(x, P, reportTestCases)) ++fails;
				E y = x * E(1.0000001);
				y.set_precision(P);
				if (!within_bound(y, P, reportTestCases)) ++fails;
			}
			// |r| near ln 2 / 2, where the halvings start from the largest r
			for (double v : { 0.3465, -0.3465, 1.0397, -1.0397 }) {
				E x(v);
				x.set_precision(P);
				if (!within_bound(x, P, reportTestCases)) ++fails;
			}
			// x tiny but above exp's round_near cut: few terms, no halving
			E tiny = random_value(rng, P, -static_cast<int64_t>(P) - 4);
			if (!within_bound(tiny, P, reportTestCases)) ++fails;
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

	std::string test_suite          = "efloat fixed-point exp kernel";
	std::string test_tag            = "exp kernel";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	const std::vector<unsigned> precisions = { 24, 32, 53, 64, 100, 128, 192, 256, 400, 512, 1000, 2048 };

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyExpKernel({ 128 }, 20, reportTestCases), "efloat", "exp kernel 128 bits");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyExpKernel(precisions, 24, reportTestCases), "efloat", "error bound, 24 to 2048 bits");
#	endif
#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyExpKernel(precisions, 96, reportTestCases), "efloat", "error bound, 4x cases");
#	endif
#	if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyExpKernel({ 4096 }, 64, reportTestCases), "efloat", "error bound, 4096 bits");
	nrOfFailedTestCases += ReportTestResult(VerifyExpKernel(precisions, 384, reportTestCases), "efloat", "error bound, 16x cases");
#	endif
#	if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyExpKernel({ 8192 }, 64, reportTestCases), "efloat", "error bound, 8192 bits");
	nrOfFailedTestCases += ReportTestResult(VerifyExpKernel(precisions, 1536, reportTestCases), "efloat", "error bound, 64x cases");
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
