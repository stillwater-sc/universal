// native_edges.cpp: blocktriple conversion from native IEEE-754 and integer values at the edges
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The QA sweep of #2029 (tracking issue #2052) found that:
//  - a float or double converted to a blocktriple with fbits >= srcbits - 1 (23 or more from
//    float, 52 or more from double) lost a bit: 1.5f read back as 0.75, and fbits >= 32 from
//    float was not shifted at all;
//  - a subnormal float or double converted to zero;
//  - integers took -rhs of INT_MIN (UB, read back -0), converted a plain char through the
//    unsigned path (char(-1) read back -2.2e12), and UINT64_MAX rounded into bit 63 (read
//    back -0) because the carry-out pattern was clamped at 2^63;
//  - a MUL, DIV or ADD blocktriple built from a native value did not have the operand layout
//    mul() and div() consume, and to_double() of a MUL result read its overflow bit as a sign.
// Every case below failed before the fix.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <cmath>
#include <cfloat>
#include <climits>
#include <cstdint>
#include <limits>
#include <universal/internal/blocktriple/blocktriple.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;

// the reference: x rounded to fbits + 1 significant bits, to nearest even (the default rounding mode)
double round_to_significand(double x, unsigned fbits) {
	int e = 0;
	double m = std::frexp(x, &e);   // m in [0.5, 1)
	double scaled = std::ldexp(m, static_cast<int>(fbits) + 1);
	return std::ldexp(std::nearbyint(scaled), e - static_cast<int>(fbits) - 1);
}

template<unsigned fbits, typename Real>
int CheckNative(Real x, double expected, bool reportTestCases) {
	blocktriple<fbits, BlockTripleOperator::REP, uint32_t> v(x);
	double result = double(v);
	if (result != expected) {
		if (reportTestCases) std::cerr << "FAIL: blocktriple<" << fbits << ">(" << x << ") = " << result << " expected " << expected << '\n';
		return 1;
	}
	return 0;
}

// float values convert exactly for fbits >= 23, double values for fbits >= 52
template<unsigned fbits>
int VerifyExactWideConversion(bool reportTestCases) {
	int nrOfFailedTests = 0;
	const float floats[] = { 1.5f, 1.0f + FLT_EPSILON, 1.75f, 3.0f / 7.0f, -1.0f - 3 * FLT_EPSILON, FLT_MAX, FLT_MIN };
	for (float f : floats) nrOfFailedTests += CheckNative<fbits>(f, double(f), reportTestCases);
	if constexpr (fbits >= 52) {
		const double doubles[] = { 1.5, 1.0 + DBL_EPSILON, 1.0 / 3.0, -2.0 - 4 * DBL_EPSILON, DBL_MAX, DBL_MIN };
		for (double d : doubles) nrOfFailedTests += CheckNative<fbits>(d, d, reportTestCases);
	}
	return nrOfFailedTests;
}

// narrower targets round to nearest even
template<unsigned fbits>
int VerifyRoundedConversion(bool reportTestCases) {
	int nrOfFailedTests = 0;
	const double doubles[] = { 1.0 / 3.0, 2.0 / 3.0, 1.0 + DBL_EPSILON, 1.0 - DBL_EPSILON / 2, 3.14159265358979, -2.718281828459045, 1.0e300, -7.0e-300 };
	for (double d : doubles) nrOfFailedTests += CheckNative<fbits>(d, round_to_significand(d, fbits), reportTestCases);
	return nrOfFailedTests;
}

int VerifySubnormals(bool reportTestCases) {
	int nrOfFailedTests = 0;
	const float  fsub[] = { std::numeric_limits<float>::denorm_min(), FLT_MIN / 8, -FLT_MIN / 3, FLT_MIN - std::numeric_limits<float>::denorm_min() };
	const double dsub[] = { std::numeric_limits<double>::denorm_min(), DBL_MIN / 4, -DBL_MIN / 3, DBL_MIN - std::numeric_limits<double>::denorm_min() };
	for (float f : fsub) {
		nrOfFailedTests += CheckNative<23>(f, double(f), reportTestCases);
		nrOfFailedTests += CheckNative<8>(f, round_to_significand(double(f), 8), reportTestCases);
	}
	for (double d : dsub) {
		nrOfFailedTests += CheckNative<52>(d, d, reportTestCases);
		nrOfFailedTests += CheckNative<10>(d, round_to_significand(d, 10), reportTestCases);
	}
	return nrOfFailedTests;
}

template<typename Integer>
int CheckInteger(Integer i, double expected, bool reportTestCases) {
	blocktriple<30, BlockTripleOperator::REP, uint32_t> v(i);
	double result = double(v);
	if (result != expected) {
		if (reportTestCases) std::cerr << "FAIL: blocktriple<30>(" << +i << ") = " << result << " expected " << expected << '\n';
		return 1;
	}
	return 0;
}

int VerifyIntegers(bool reportTestCases) {
	int nrOfFailedTests = 0;
	nrOfFailedTests += CheckInteger(INT_MIN, -2147483648.0, reportTestCases);
	nrOfFailedTests += CheckInteger(INT_MAX, round_to_significand(2147483647.0, 30), reportTestCases);
	nrOfFailedTests += CheckInteger(LLONG_MIN, -9223372036854775808.0, reportTestCases);
	nrOfFailedTests += CheckInteger(static_cast<short>(-32768), -32768.0, reportTestCases);
	nrOfFailedTests += CheckInteger(static_cast<signed char>(-1), -1.0, reportTestCases);
	nrOfFailedTests += CheckInteger(static_cast<signed char>(-128), -128.0, reportTestCases);
	nrOfFailedTests += CheckInteger(static_cast<unsigned char>(255), 255.0, reportTestCases);
	nrOfFailedTests += CheckInteger(static_cast<char>(-1), std::is_signed_v<char> ? -1.0 : 255.0, reportTestCases);
	nrOfFailedTests += CheckInteger(static_cast<char>(65), 65.0, reportTestCases);
	nrOfFailedTests += CheckInteger(12345, 12345.0, reportTestCases);
	nrOfFailedTests += CheckInteger(-12345L, -12345.0, reportTestCases);
	nrOfFailedTests += CheckInteger(4000000000u, 4000000000.0, reportTestCases);

	// rounding a 64-bit integer: UINT64_MAX carries out of 1.11..1 into the next binade, 2^64
	{
		blocktriple<62, BlockTripleOperator::REP, uint64_t> v(std::numeric_limits<unsigned long long>::max());
		if (double(v) != 18446744073709551616.0 || v.scale() != 64) {
			if (reportTestCases) std::cerr << "FAIL: blocktriple<62>(UINT64_MAX) = " << double(v) << " scale " << v.scale() << '\n';
			++nrOfFailedTests;
		}
		blocktriple<8, BlockTripleOperator::REP, uint8_t> w(12345);   // 1.1000000111001b * 2^13 -> 1.10000010b * 2^13
		if (double(w) != 12352.0) {
			if (reportTestCases) std::cerr << "FAIL: blocktriple<8>(12345) = " << double(w) << '\n';
			++nrOfFailedTests;
		}
	}
	return nrOfFailedTests;
}

// a blocktriple built from a native value has the operand layout of its operator
int VerifyOperatorLayouts(bool reportTestCases) {
	int nrOfFailedTests = 0;
	auto check = [&](double result, double expected, const char* label) {
		if (result != expected) {
			if (reportTestCases) std::cerr << "FAIL: " << label << " = " << result << " expected " << expected << '\n';
			++nrOfFailedTests;
		}
	};
	{
		using Mul = blocktriple<4, BlockTripleOperator::MUL, uint8_t>;
		Mul a(1.5), b(1.5), c;
		c.mul(a, b);
		check(double(c), 2.25, "MUL 1.5 * 1.5");   // the product sets the overflow bit: not a sign
		Mul d(-1.75), e(1.75), f;
		f.mul(d, e);
		check(double(f), -3.0625, "MUL -1.75 * 1.75");
		Mul g(-12), h(3), k;
		k.mul(g, h);
		check(double(k), -36.0, "MUL int -12 * 3");
	}
	{
		using Div = blocktriple<8, BlockTripleOperator::DIV, uint32_t>;
		Div a(3.0), b(1.5), c;
		c.div(a, b);
		check(double(c), 2.0, "DIV 3 / 1.5");
		Div d(7.0), e(0.5), f;
		f.div(d, e);
		check(double(f), 14.0, "DIV 7 / 0.5");
		Div g(12), h(-3), k;
		k.div(g, h);
		check(double(k), -4.0, "DIV int 12 / -3");
	}
	{
		using Add = blocktriple<8, BlockTripleOperator::ADD, uint32_t>;
		Add a(3.0), b(-1.25), c;
		c.add(a, b);
		check(double(c), 1.75, "ADD 3 + -1.25");
		Add d(100), e(-28), f;
		f.add(d, e);
		check(double(f), 72.0, "ADD int 100 + -28");
	}
	return nrOfFailedTests;
}

} // namespace

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
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "blocktriple native conversion edges";
	std::string test_tag    = "conversion";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	nrOfFailedTestCases += VerifyExactWideConversion<23>(true);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyExactWideConversion<23>(reportTestCases), "blocktriple<23>", "float, fbits == 23");
	nrOfFailedTestCases += ReportTestResult(VerifyExactWideConversion<24>(reportTestCases), "blocktriple<24>", "float, fbits == 24");
	nrOfFailedTestCases += ReportTestResult(VerifyExactWideConversion<40>(reportTestCases), "blocktriple<40>", "float, fbits > 32");
	nrOfFailedTestCases += ReportTestResult(VerifyExactWideConversion<52>(reportTestCases), "blocktriple<52>", "double, fbits == 52");
	nrOfFailedTestCases += ReportTestResult(VerifyExactWideConversion<53>(reportTestCases), "blocktriple<53>", "double, fbits == 53");
	nrOfFailedTestCases += ReportTestResult(VerifyExactWideConversion<80>(reportTestCases), "blocktriple<80>", "double, fbits > 64");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundedConversion<10>(reportTestCases), "blocktriple<10>", "double, rounded");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundedConversion<23>(reportTestCases), "blocktriple<23>", "double, rounded");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundedConversion<51>(reportTestCases), "blocktriple<51>", "double, rounded");
	nrOfFailedTestCases += ReportTestResult(VerifySubnormals(reportTestCases), "blocktriple", "subnormals");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegers(reportTestCases), "blocktriple", "integers");
	nrOfFailedTestCases += ReportTestResult(VerifyOperatorLayouts(reportTestCases), "blocktriple", "operator layouts");
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
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
