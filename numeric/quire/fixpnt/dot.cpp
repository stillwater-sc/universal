// dot.cpp: Fixed-point dot product cancellation - quire for embedded linear algebra
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under
// an MIT Open Source license.
//
// ============================================================================
// Fixed-Point Dot Products for Embedded Linear Algebra
// ============================================================================
//
// Microcontrollers and DSP cores without floating-point units perform all
// arithmetic in fixed-point.  When these devices run linear algebra kernels
// - sensor fusion, Kalman filters, PID controllers, neural network inference
// - the dot products that form the inner loops suffer from two sources of
// error:
//
//   1. PRODUCT TRUNCATION: multiplying two fixpnt<N, R> values produces a
//      2N-bit result with 2R fractional bits.  Truncating back to N bits
//      discards the lower R fractional bits of EVERY product.
//
//   2. ACCUMULATION OVERFLOW: summing many products in an N-bit accumulator
//      can overflow, wrapping (modular) or saturating.
//
// A hardware DSP multiply-accumulate (MAC) unit solves both problems by
// using a wide accumulator (typically 2N or 2N+guard bits).  The quire
// provides the same guarantee in software: it holds the full unrounded
// product and accumulates without overflow (within its capacity).
//
// ============================================================================
// What This Example Demonstrates
// ============================================================================
//
// Case 1: Product truncation error - many small products whose individual
//         truncation errors accumulate to a significant bias.
//
// Case 2: Cancellation in a control residual - computing e = r - y where
//         r and y are dot products of similar magnitude.  The difference
//         is small, and product truncation can flip its sign.
//
// Case 3: Long accumulation - 1000+ multiply-accumulate operations that
//         overflow a narrow accumulator.  The quire's extended range
//         prevents overflow.
//
// Case 4: Reproducibility - same dot product in forward and reverse order
//         gives identical results with FDP, but not with naive accumulation
//         (due to modular wrap at different intermediate sums).
//
// ============================================================================
// References
// ============================================================================
//
// [1] Jejeev Patel and Markus Püschel (2018). "Generating Optimized
//     Fixed-Point Arithmetic for Linear Algebra." ACM TOMS 44(2).
//     - Fixed-point linear algebra kernels for embedded systems.
//
// [2] Yates, R. (2013). "Fixed-Point Arithmetic: An Introduction."
//     - Classic DSP reference for fixed-point accumulator sizing.
//
// [3] Kulisch, U. W. (2013). "Computer Arithmetic and Validity."
//     - The super-accumulator as a universal solution to the
//     accumulator sizing problem.
//
// ============================================================================

#include <universal/utility/directives.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/fixpnt/fdp.hpp>
#include <universal/number/quire/quire.hpp>
#include <universal/verification/test_reporters.hpp>

#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <random>

// ============================================================================
// Case 1: Product truncation bias
//
// Compute x·y where each x[i]*y[i] is a small value whose lower bits
// are lost to truncation.  Over N products, the truncation errors
// accumulate to a systematic bias.
//
// Example: x[i] = 0.1, y[i] = 0.3 for all i.
// In fixpnt<16,8>, 0.1 ~ 26/256, 0.3 ~ 77/256.
// Exact product: 26*77/65536 = 2002/65536 ~ 0.030548
// Truncated to fixpnt<16,8>: floor(2002/256)/256 = 7/256 ~ 0.027344
// Truncation error per product: ~0.003
// Over N=100 products: bias ~ 0.3 (significant!)
// ============================================================================
template<typename Scalar>
void Case1_TruncationBias() {
	constexpr size_t N = 100;
	std::vector<Scalar> x(N, Scalar(0.1));
	std::vector<Scalar> y(N, Scalar(0.3));

	// Naive accumulation
	Scalar naive(0);
	for (size_t i = 0; i < N; ++i)
		naive = naive + x[i] * y[i];

	// FDP via quire
	Scalar fdp_result = sw::universal::fdp(x, y);

	// Reference in double
	double ref = 0;
	for (size_t i = 0; i < N; ++i)
		ref += double(x[i]) * double(y[i]);

	std::cout << "\n  Case 1: Product truncation bias (N=" << N << " products)\n";
	std::cout << "    x[i] = 0.1 ~ " << double(Scalar(0.1)) << " in fixpnt\n";
	std::cout << "    y[i] = 0.3 ~ " << double(Scalar(0.3)) << " in fixpnt\n";
	std::cout << "    Exact (double): " << std::setprecision(8) << ref << '\n';
	std::cout << "    Naive fixpnt:   " << double(naive)
	          << "  (error: " << std::abs(double(naive) - ref) << ")\n";
	std::cout << "    FDP fixpnt:     " << double(fdp_result)
	          << "  (error: " << std::abs(double(fdp_result) - ref) << ")\n";
}

// ============================================================================
// Case 2: Control loop residual - sign-critical cancellation
//
// In a control system, the error signal e = r - y where:
//   r = sum(w_i * r_i)   (reference signal, weighted)
//   y = sum(w_i * s_i)   (sensor readings, weighted)
//
// When the system is near equilibrium, r ~ y, and the error signal is
// small.  Product truncation can make |error| larger than |r - y|,
// potentially flipping the sign and causing the controller to push
// in the wrong direction.
// ============================================================================
template<typename Scalar>
void Case2_ControlResidual() {
	// Weights and signals chosen so r ~ y with a small positive residual
	std::vector<Scalar> w  = { Scalar(0.4), Scalar(0.35), Scalar(0.25) };
	std::vector<Scalar> r  = { Scalar(10.5), Scalar(20.25), Scalar(15.75) };
	std::vector<Scalar> s  = { Scalar(10.5), Scalar(20.25), Scalar(15.5) };

	// Naive: compute r_dot = w·r, y_dot = w·s, error = r_dot - y_dot
	Scalar naive_r(0), naive_y(0);
	for (size_t i = 0; i < w.size(); ++i) {
		naive_r = naive_r + w[i] * r[i];
		naive_y = naive_y + w[i] * s[i];
	}
	Scalar naive_err = naive_r - naive_y;

	// FDP: compute each dot product exactly, then subtract
	Scalar fdp_r = sw::universal::fdp(w, r);
	Scalar fdp_y = sw::universal::fdp(w, s);
	Scalar fdp_err = fdp_r - fdp_y;

	// Reference in double
	double ref_r = 0, ref_y = 0;
	for (size_t i = 0; i < w.size(); ++i) {
		ref_r += double(w[i]) * double(r[i]);
		ref_y += double(w[i]) * double(s[i]);
	}
	double ref_err = ref_r - ref_y;

	std::cout << "\n  Case 2: Control loop residual (sign-critical)\n";
	std::cout << "    r = weighted reference, y = weighted sensor\n";
	std::cout << "    Exact error (double): " << std::setprecision(8) << ref_err << '\n';
	std::cout << "    Naive error:          " << double(naive_err)
	          << (double(naive_err) * ref_err > 0 ? "  sign OK" : "  WRONG SIGN!") << '\n';
	std::cout << "    FDP error:            " << double(fdp_err)
	          << (double(fdp_err) * ref_err > 0 ? "  sign OK" : "  WRONG SIGN!") << '\n';
}

// ============================================================================
// Case 3: Long accumulation - overflow in narrow accumulators
//
// Sum 500 products where each product is ~0.5.  The sum (~250) exceeds
// the integer range of fixpnt<16,8> (which can hold ±127).  Naive
// accumulation wraps around; the quire's extended range holds the sum.
// ============================================================================
template<typename Scalar>
void Case3_LongAccumulation() {
	constexpr size_t N = 500;
	std::vector<Scalar> x(N, Scalar(0.7));
	std::vector<Scalar> y(N, Scalar(0.7));

	// Naive accumulation
	Scalar naive(0);
	for (size_t i = 0; i < N; ++i)
		naive = naive + x[i] * y[i];

	// FDP via quire
	Scalar fdp_result = sw::universal::fdp(x, y);

	// Reference
	double ref = 0;
	for (size_t i = 0; i < N; ++i)
		ref += double(x[i]) * double(y[i]);

	std::cout << "\n  Case 3: Long accumulation (N=" << N << ", sum ~ " << ref << ")\n";
	std::cout << "    fixpnt<16,8> integer range: [-128, +127]\n";
	std::cout << "    Expected sum: " << std::setprecision(6) << ref << '\n';
	std::cout << "    Naive fixpnt: " << double(naive)
	          << (std::abs(double(naive) - ref) < 1.0 ? "  (close)" : "  (OVERFLOW/WRAP)") << '\n';
	std::cout << "    FDP fixpnt:   " << double(fdp_result)
	          << "  (quire range prevents overflow, but result truncated to fixpnt)\n";
	std::cout << "    Note: FDP converts from quire back to fixpnt<16,8>, so values\n";
	std::cout << "    exceeding the fixpnt range still saturate/wrap on extraction.\n";
	std::cout << "    The quire's value is to maintain EXACT intermediate sums.\n";
}

// ============================================================================
// Case 4: Reproducibility - order independence
//
// Naive fixpnt accumulation is order-dependent because intermediate sums
// may wrap or truncate differently.  FDP gives identical results regardless
// of element order because the quire accumulates all products exactly.
// ============================================================================
template<typename Scalar>
void Case4_Reproducibility() {
	constexpr size_t N = 256;
	std::vector<Scalar> x(N), y(N);

	// Deterministic pseudo-random values
	std::mt19937 rng(12345);
	std::uniform_real_distribution<double> dist(-2.0, 2.0);
	for (size_t i = 0; i < N; ++i) {
		x[i] = Scalar(dist(rng));
		y[i] = Scalar(dist(rng));
	}

	// Forward and reverse
	Scalar fwd_fdp = sw::universal::fdp(x, y);

	std::vector<Scalar> xr(x.rbegin(), x.rend());
	std::vector<Scalar> yr(y.rbegin(), y.rend());
	Scalar rev_fdp = sw::universal::fdp(xr, yr);

	Scalar fwd_naive(0), rev_naive(0);
	for (size_t i = 0; i < N; ++i) {
		fwd_naive = fwd_naive + x[i] * y[i];
		rev_naive = rev_naive + xr[i] * yr[i];
	}

	std::cout << "\n  Case 4: Reproducibility (N=" << N << " random elements)\n";
	std::cout << "    Naive fwd: " << std::setprecision(8) << double(fwd_naive) << '\n';
	std::cout << "    Naive rev: " << double(rev_naive)
	          << (double(fwd_naive) == double(rev_naive) ? "  match" : "  DIFFER") << '\n';
	std::cout << "    FDP   fwd: " << double(fwd_fdp) << '\n';
	std::cout << "    FDP   rev: " << double(rev_fdp)
	          << (double(fwd_fdp) == double(rev_fdp) ? "  match" : "  DIFFER") << '\n';
}

// Regression testing guards
#ifndef MANUAL_TESTING
#define MANUAL_TESTING 0
#endif
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "Fixed-point dot product - quire FDP";
	std::string test_tag   = "fixpnt_dot";
	bool reportTestCases   = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	std::cout << "============================================================\n";
	std::cout << "Fixed-Point Dot Product Cancellation (fixpnt<16,8>)\n";
	std::cout << "============================================================\n";
	std::cout << "\nFixed-point multiply truncates the lower fractional bits of\n";
	std::cout << "every product.  Over many products, this truncation error\n";
	std::cout << "accumulates as a systematic bias.  The quire holds the full\n";
	std::cout << "unrounded product and eliminates this bias.\n";

	using Fp = fixpnt<16, 8, Modulo, uint16_t>;

	Case1_TruncationBias<Fp>();
	Case2_ControlResidual<Fp>();
	Case3_LongAccumulation<Fp>();
	Case4_Reproducibility<Fp>();

	std::cout << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore errors
#else

#if REGRESSION_LEVEL_1
	// Verify FDP reproducibility
	{
		using Fp = fixpnt<16, 8, Modulo, uint16_t>;
		constexpr size_t N = 256;
		std::vector<Fp> x(N), y(N);
		std::mt19937 rng(12345);
		std::uniform_real_distribution<double> dist(-2.0, 2.0);
		for (size_t i = 0; i < N; ++i) {
			x[i] = Fp(dist(rng));
			y[i] = Fp(dist(rng));
		}
		Fp fwd = fdp(x, y);
		std::vector<Fp> xr(x.rbegin(), x.rend());
		std::vector<Fp> yr(y.rbegin(), y.rend());
		Fp rev = fdp(xr, yr);
		if (double(fwd) != double(rev)) {
			++nrOfFailedTestCases;
			std::cerr << "FAIL: fixpnt FDP not reproducible\n";
		}
	}
	// Verify FDP reduces truncation bias
	{
		using Fp = fixpnt<16, 8, Modulo, uint16_t>;
		constexpr size_t N = 100;
		std::vector<Fp> x(N, Fp(0.1));
		std::vector<Fp> y(N, Fp(0.3));
		Fp fdp_result = fdp(x, y);
		Fp naive(0);
		for (size_t i = 0; i < N; ++i) naive = naive + x[i] * y[i];
		double ref = 0;
		for (size_t i = 0; i < N; ++i) ref += double(x[i]) * double(y[i]);
		// FDP should be closer to reference than naive
		if (std::abs(double(fdp_result) - ref) > std::abs(double(naive) - ref)) {
			++nrOfFailedTestCases;
			std::cerr << "FAIL: FDP truncation bias worse than naive\n";
		}
	}
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const sw::universal::quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << '\n';
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << '\n';
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
