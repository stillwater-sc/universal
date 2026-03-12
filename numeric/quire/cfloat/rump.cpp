// rump.cpp: Rump's polynomial — a classic extended-precision problem
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under
// an MIT Open Source license.
//
// ============================================================================
// Rump's Polynomial — A Classic Floating-Point Failure
// ============================================================================
//
// Siegfried Rump designed a polynomial that defeats every fixed-precision
// floating-point format:
//
//   f(a, b) = 333.75·b⁶ + a²·(11·a²·b² − b⁶ − 121·b⁴ − 2) + 5.5·b⁸ + a/(2b)
//
// Evaluated at a = 77617, b = 33096, the exact answer is:
//
//   f(77617, 33096) = −54767/66192 ≈ −0.827396059946821
//
// Yet every standard precision gets a wildly wrong answer:
//
//   float (24-bit significand):   garbage (~1e30 or 0)
//   double (53-bit significand):  −1.18059e+21
//   long double (64-bit):         −1.18059e+21
//   dd (~106-bit significand):    −1.18059e+21  (also fails!)
//
// Only quad-double (qd, ~212-bit significand) has enough precision to resolve
// the cancellation correctly.
//
// ============================================================================
// Why This Is an Extended-Precision Problem, NOT an Accumulation Problem
// ============================================================================
//
// The Kulisch super-accumulator (quire) provides EXACT ACCUMULATION of
// floating-point products — it eliminates rounding error in the SUMMATION
// step. However, Rump's polynomial fails because the individual TERMS
// (not their sum) exceed the precision of the operands:
//
//   Term 3: −a²·b⁶  ≈ −7.917×10³⁶  (~122 bits to represent exactly)
//   Term 6: +5.5·b⁸ ≈ +7.917×10³⁶  (~122 bits to represent exactly)
//
// These two terms agree to ~36 decimal digits (~120 binary digits). Even
// in double precision (53-bit significand), each term carries an error of
// ~2⁶⁹ — far larger than the true sum (−2). Exact accumulation of
// imprecise operands still yields an imprecise result.
//
// The fix is EXTENDED PRECISION OPERANDS (dd, qd), not exact accumulation.
// For dot product catastrophic cancellation (where the quire shines), see
// dot.cpp in this directory.
//
// ============================================================================
// Derivation: Exact Value of f(77617, 33096)
// ============================================================================
//
// Expand the polynomial:
//   f = 333.75·b⁶ + 11·a⁴·b² − a²·b⁶ − 121·a²·b⁴ − 2·a² + 5.5·b⁸ + a/(2b)
//
// The polynomial part (without a/(2b)) sums to EXACTLY −2:
//   333.75·b⁶ + 11·a⁴·b² − a²·b⁶ − 121·a²·b⁴ − 2·a² + 5.5·b⁸ = −2
//
// Therefore:
//   f(a,b) = −2 + a/(2b) = −2 + 77617/66192 = (−132384 + 77617)/66192
//          = −54767/66192 ≈ −0.827396059946821...
//
// The cancellation involves terms of magnitude ~10³⁶ cancelling to −2.
// This requires ~120 binary digits of precision in the operands — beyond
// float (24), double (53), long double (64), and dd (~106).
// Only qd (~212 bits) has enough precision.
//
// ============================================================================

#include <universal/utility/directives.hpp>
#include <universal/number/dd/dd.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/verification/test_reporters.hpp>

#include <iostream>
#include <iomanip>
#include <cmath>

// Evaluate Rump's polynomial f(a,b) in arbitrary precision type T.
// f(a,b) = 333.75*b^6 + a^2*(11*a^2*b^2 - b^6 - 121*b^4 - 2) + 5.5*b^8 + a/(2*b)
template<typename T>
T rump(T a, T b) {
	T a2 = a * a;
	T b2 = b * b;
	T b4 = b2 * b2;
	T b6 = b4 * b2;
	T b8 = b4 * b4;

	T term1 = T(333.75) * b6;
	T inner = T(11.0) * a2 * b2 - b6 - T(121.0) * b4 - T(2.0);
	T term2 = a2 * inner;
	T term3 = T(5.5) * b8;
	T term4 = a / (T(2.0) * b);

	return term1 + term2 + term3 + term4;
}

// Reference value: -54767/66192
static constexpr double RUMP_EXACT = -54767.0 / 66192.0;  // -0.82739605994682...

// ============================================================================
// Evaluate Rump's polynomial across precision levels
// ============================================================================
void EvaluateRumpPolynomial() {
	using namespace sw::universal;

	std::cout << "============================================================\n";
	std::cout << "Rump's Polynomial f(77617, 33096)\n";
	std::cout << "============================================================\n";
	std::cout << std::setprecision(15);
	std::cout << "Exact: -54767/66192 = " << RUMP_EXACT << "\n\n";

	// float (24-bit significand)
	{
		float a = 77617.0f, b = 33096.0f;
		float result = rump(a, b);
		std::cout << "  float          (24-bit) : " << std::setw(25) << result
		          << (std::abs(result - RUMP_EXACT) < 0.01 ? "  PASS" : "  FAIL") << '\n';
	}

	// double (53-bit significand)
	{
		double a = 77617.0, b = 33096.0;
		double result = rump(a, b);
		std::cout << "  double         (53-bit) : " << std::setw(25) << result
		          << (std::abs(result - RUMP_EXACT) < 0.01 ? "  PASS" : "  FAIL") << '\n';
	}

	// long double (64-bit on x86, 53-bit on some platforms)
	{
		long double a = 77617.0L, b = 33096.0L;
		long double result = rump(a, b);
		std::cout << "  long double    (64-bit) : " << std::setw(25) << static_cast<double>(result)
		          << (std::abs(static_cast<double>(result) - RUMP_EXACT) < 0.01 ? "  PASS" : "  FAIL") << '\n';
	}

	// dd (~106-bit significand)
	{
		dd a(77617.0), b(33096.0);
		dd result = rump(a, b);
		std::cout << "  dd            (106-bit) : " << std::setw(25) << double(result)
		          << (std::abs(double(result) - RUMP_EXACT) < 0.01 ? "  PASS" : "  FAIL") << '\n';
	}

	// qd (~212-bit significand)
	{
		qd a(77617.0), b(33096.0);
		qd result = rump(a, b);
		std::cout << "  qd            (212-bit) : " << std::setw(25) << double(result)
		          << (std::abs(double(result) - RUMP_EXACT) < 0.01 ? "  PASS" : "  FAIL") << '\n';
	}

	std::cout << '\n';
	std::cout << "Lesson: Rump's polynomial requires ~120 bits of operand precision.\n";
	std::cout << "Only qd (quad-double, ~212 bits) has enough precision to resolve\n";
	std::cout << "the catastrophic cancellation in the intermediate terms.\n";
	std::cout << "This is an EXTENDED PRECISION problem, not an accumulation problem.\n";
	std::cout << "See dot.cpp for accumulation error examples where the quire helps.\n";
	std::cout << '\n';
}

// Regression testing guards
#define MANUAL_TESTING 1
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

	std::string test_suite = "Rump's polynomial — extended precision";
	std::string test_tag   = "rump";
	bool reportTestCases   = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	EvaluateRumpPolynomial();

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore errors
#else

#if REGRESSION_LEVEL_1
	// Verify qd gets Rump correct
	{
		qd a(77617.0), b(33096.0);
		qd result = rump(a, b);
		if (std::abs(double(result) - RUMP_EXACT) > 0.01) {
			++nrOfFailedTestCases;
			std::cerr << "FAIL: qd Rump result = " << double(result) << '\n';
		}
	}
	// Verify dd fails (expected — only 106 bits, need ~120)
	{
		dd a(77617.0), b(33096.0);
		dd result = rump(a, b);
		if (std::abs(double(result) - RUMP_EXACT) < 0.01) {
			// If dd somehow passes, that's unexpected but not a failure
			std::cout << "NOTE: dd unexpectedly got Rump correct\n";
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
