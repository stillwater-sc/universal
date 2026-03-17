// dot.cpp: Dot product catastrophic cancellation - the quire's domain
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under
// an MIT Open Source license.
//
// ============================================================================
// Dot Product Catastrophic Cancellation
// ============================================================================
//
// When large intermediate products cancel in a dot product, naive floating-
// point summation loses the small residual to rounding. The Kulisch super-
// accumulator (quire) eliminates this by accumulating ALL products exactly
// (with no intermediate rounding) and performing a SINGLE rounding at the end.
//
// This is the problem the quire was designed to solve. Contrast with Rump's
// polynomial (rump.cpp), which requires extended-precision OPERANDS - a
// fundamentally different failure mode.
//
// The fused dot product (FDP) computes:
//   fdp(x, y) = round_once( sum_i( x[i] * y[i] ) )
//
// where the sum is accumulated in the quire with no intermediate rounding.
//
// ============================================================================

#include <universal/utility/directives.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/cfloat/fdp.hpp>
#include <universal/number/quire/quire.hpp>
#include <universal/verification/test_reporters.hpp>

#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <random>

// ============================================================================
// Case 1: Large cancelling products with small residual
//
// Two large products cancel, leaving a small residual.
//   x = [ 3.2e8,  1,  -1,  8e7 ]
//   y = [ 4.0e7,  1,  -1, -1.6e8 ]
//   x*y = 1.28e16 + 1 + 1 - 1.28e16 = 2
//
// The two dominant products (+/-1.28e16) consume all significand bits.
// In float32, the residual "+2" is below the rounding threshold.
// ============================================================================
void Case1_CancellingProducts() {
	using namespace sw::universal;
	using Scalar = cfloat<32, 8, uint32_t, true, false, false>;

	std::vector<Scalar> x = { Scalar(3.2e8f), Scalar(1.0f), Scalar(-1.0f), Scalar(8e7f) };
	std::vector<Scalar> y = { Scalar(4.0e7f), Scalar(1.0f), Scalar(-1.0f), Scalar(-1.6e8f) };

	// Naive accumulation
	float naive = 0.0f;
	for (size_t i = 0; i < x.size(); ++i)
		naive += float(x[i]) * float(y[i]);

	// FDP via quire
	Scalar fdp_result = fdp(x, y);

	std::cout << "\n  Case 1: Large cancelling products\n";
	std::cout << "    x = [ 3.2e8, 1, -1, 8e7 ]\n";
	std::cout << "    y = [ 4.0e7, 1, -1, -1.6e8 ]\n";
	std::cout << "    Exact:  2.0\n";
	std::cout << "    Naive:  " << naive
	          << (naive == 2.0f ? "  PASS" : "  FAIL") << '\n';
	std::cout << "    FDP:    " << double(fdp_result)
	          << (double(fdp_result) == 2.0 ? "  PASS" : "  FAIL") << '\n';
}

// ============================================================================
// Case 2: Many cancelling pairs + small residual
//
// 511 pairs of (+1e8, -1e8) followed by two small values.
// The quire accumulates all 1024 products exactly.
// ============================================================================
void Case2_ManyCancellingPairs() {
	using namespace sw::universal;
	using Scalar = cfloat<32, 8, uint32_t, true, false, false>;

	constexpr size_t N = 1024;
	std::vector<Scalar> x(N), y(N);
	// Place all +1e8 terms first, then all -1e8, then residuals.
	// This forces naive accumulation to build up a huge sum before
	// subtracting, losing the small residual to rounding.
	for (int i = 0; i < 511; ++i) {
		x[i]       = Scalar(1e8f);   y[i]       = Scalar(1.0f);
		x[511 + i] = Scalar(-1e8f);  y[511 + i] = Scalar(1.0f);
	}
	x[1022] = Scalar(0.5f);   y[1022] = Scalar(1.0f);
	x[1023] = Scalar(0.25f);  y[1023] = Scalar(1.0f);

	float naive = 0.0f;
	for (size_t i = 0; i < N; ++i)
		naive += float(x[i]) * float(y[i]);

	Scalar fdp_result = fdp(x, y);

	std::cout << "\n  Case 2: 511 cancelling pairs + residual 0.75\n";
	std::cout << "    Exact:  0.75\n";
	std::cout << "    Naive:  " << naive
	          << (naive == 0.75f ? "  PASS" : "  FAIL") << '\n';
	std::cout << "    FDP:    " << double(fdp_result)
	          << (double(fdp_result) == 0.75 ? "  PASS" : "  FAIL") << '\n';
}

// ============================================================================
// Case 3: Telescoping cancellation across scales
//
// Cancelling pairs at magnitudes 1e30, 1e25, 1e20, ..., 1e1,
// followed by a small residual pi ~= 3.14.
// ============================================================================
void Case3_TelescopingCancellation() {
	using namespace sw::universal;
	using Scalar = cfloat<32, 8, uint32_t, true, false, false>;

	std::vector<Scalar> x, y;
	float magnitudes[] = { 1e30f, 1e25f, 1e20f, 1e15f, 1e10f, 1e5f, 1e3f, 1e1f };
	// All positive terms first, then all negative - forces naive to build
	// a huge sum before subtracting, losing the residual to rounding.
	for (float mag : magnitudes) {
		x.push_back(Scalar(mag));    y.push_back(Scalar(1.0f));
	}
	for (float mag : magnitudes) {
		x.push_back(Scalar(-mag));   y.push_back(Scalar(1.0f));
	}
	x.push_back(Scalar(3.14f));  y.push_back(Scalar(1.0f));

	float naive = 0.0f;
	for (size_t i = 0; i < x.size(); ++i)
		naive += float(x[i]) * float(y[i]);

	Scalar fdp_result = fdp(x, y);
	double expected = double(float(Scalar(3.14f)));

	std::cout << "\n  Case 3: Telescoping cancellation (1e30 down to 1e1) + residual\n";
	std::cout << "    Exact:  " << expected << '\n';
	std::cout << "    Naive:  " << naive
	          << (std::abs(naive - expected) < 1e-5 ? "  PASS" : "  FAIL") << '\n';
	std::cout << "    FDP:    " << double(fdp_result)
	          << (std::abs(double(fdp_result) - expected) < 1e-5 ? "  PASS" : "  FAIL") << '\n';
}

// ============================================================================
// Case 4: Reproducibility - same vectors, reversed order
//
// In IEEE arithmetic, a+b+c != c+b+a due to non-associativity.
// The quire guarantees identical results regardless of summation order
// because it accumulates into a fixed-point register wide enough to hold
// any product exactly - order doesn't matter when there's no rounding.
// ============================================================================
void Case4_Reproducibility() {
	using namespace sw::universal;
	using Scalar = cfloat<32, 8, uint32_t, true, false, false>;

	constexpr size_t N = 1024;
	std::vector<Scalar> x(N), y(N);
	std::mt19937 rng(42);
	std::uniform_real_distribution<float> dist(-100.0f, 100.0f);
	for (size_t i = 0; i < N; ++i) {
		x[i] = Scalar(dist(rng));
		y[i] = Scalar(dist(rng));
	}

	Scalar fwd = fdp(x, y);

	std::vector<Scalar> xr(x.rbegin(), x.rend());
	std::vector<Scalar> yr(y.rbegin(), y.rend());
	Scalar rev = fdp(xr, yr);

	// Naive forward vs reverse
	float naive_fwd = 0.0f, naive_rev = 0.0f;
	for (size_t i = 0; i < N; ++i) {
		naive_fwd += float(x[i]) * float(y[i]);
		naive_rev += float(xr[i]) * float(yr[i]);
	}

	std::cout << "\n  Case 4: Reproducibility (1024 random elements, fwd vs rev)\n";
	std::cout << "    Naive fwd:  " << naive_fwd << '\n';
	std::cout << "    Naive rev:  " << naive_rev
	          << (naive_fwd == naive_rev ? "  match" : "  DIFFER") << '\n';
	std::cout << "    FDP   fwd:  " << double(fwd) << '\n';
	std::cout << "    FDP   rev:  " << double(rev)
	          << (double(fwd) == double(rev) ? "  match" : "  DIFFER") << '\n';
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

	std::string test_suite = "Dot product catastrophic cancellation - quire FDP";
	std::string test_tag   = "dot";
	bool reportTestCases   = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	std::cout << "============================================================\n";
	std::cout << "Dot Product Catastrophic Cancellation (cfloat<32,8>)\n";
	std::cout << "============================================================\n";
	std::cout << std::setprecision(15);

	Case1_CancellingProducts();
	Case2_ManyCancellingPairs();
	Case3_TelescopingCancellation();
	Case4_Reproducibility();

	std::cout << '\n';
	std::cout << "Lesson: The quire resolves catastrophic cancellation in DOT PRODUCTS\n";
	std::cout << "by accumulating unrounded products exactly. This is the Kulisch insight:\n";
	std::cout << "never round intermediate results, and the final answer is correctly rounded.\n";
	std::cout << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore errors
#else

#if REGRESSION_LEVEL_1
	// Verify FDP resolves Case 1 cancellation
	{
		using Scalar = cfloat<32, 8, uint32_t, true, false, false>;
		std::vector<Scalar> x = { Scalar(3.2e8f), Scalar(1.0f), Scalar(-1.0f), Scalar(8e7f) };
		std::vector<Scalar> y = { Scalar(4.0e7f), Scalar(1.0f), Scalar(-1.0f), Scalar(-1.6e8f) };
		Scalar result = fdp(x, y);
		if (double(result) != 2.0) {
			++nrOfFailedTestCases;
			std::cerr << "FAIL: FDP cancellation, expected 2.0, got " << double(result) << '\n';
		}
	}
	// Verify FDP reproducibility (Case 4)
	{
		using Scalar = cfloat<32, 8, uint32_t, true, false, false>;
		constexpr size_t N = 1024;
		std::vector<Scalar> x(N), y(N);
		std::mt19937 rng(42);
		std::uniform_real_distribution<float> dist(-100.0f, 100.0f);
		for (size_t i = 0; i < N; ++i) {
			x[i] = Scalar(dist(rng));
			y[i] = Scalar(dist(rng));
		}
		Scalar fwd = fdp(x, y);
		std::vector<Scalar> xr(x.rbegin(), x.rend());
		std::vector<Scalar> yr(y.rbegin(), y.rend());
		Scalar rev = fdp(xr, yr);
		if (double(fwd) != double(rev)) {
			++nrOfFailedTestCases;
			std::cerr << "FAIL: FDP not reproducible, fwd=" << double(fwd)
			          << " rev=" << double(rev) << '\n';
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
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << '\n';
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
