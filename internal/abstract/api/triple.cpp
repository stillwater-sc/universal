// triple.cpp: the abstract (sign, scale, fraction) triple: conversion, special values and order
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The first test of abstract/triple. The QA sweep of #2029 (tracking issue #2053) found the
// integer and floating-point assignments empty, so a triple held whatever its members were
// left at (triple(2.5) read back 1.0), every infinity negative, NaN dropped by negation and
// abs, and the fractions compared as signed numbers. The reference is the value rounded once
// to fbits + 1 significant bits, to nearest even.
#include <universal/utility/directives.hpp>
#include <cfloat>
#include <climits>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <universal/internal/abstract/triple.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;

double rne(double x, unsigned fbits) {
	if (x == 0.0 || !std::isfinite(x)) return x;
	int e = 0;
	const double m = std::frexp(x, &e);
	return std::ldexp(std::nearbyint(std::ldexp(m, static_cast<int>(fbits) + 1)), e - static_cast<int>(fbits) - 1);
}

template<size_t fbits>
int VerifyConversion(bool reportTestCases) {
	using T = triple<fbits, std::uint32_t>;
	int nrOfFailedTests = 0;
	auto check = [&](double got, double expected, const std::string& label) {
		if (got != expected || std::signbit(got) != std::signbit(expected)) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: triple<" << fbits << "> " << label << " = " << got << " expected " << expected << '\n';
		}
	};
	for (double d : { 2.5, -0.1, 1.0 / 3.0, 1.0 + DBL_EPSILON, 1e-310, -1e300, 0.75, -0.0 }) {
		check(double(T(d)), rne(d, fbits), "from double " + std::to_string(d));
	}
	check(double(T(0.1f)), rne(double(0.1f), fbits), "from float 0.1f");
	for (long long v : { 0LL, 5LL, -7LL, 123456789LL, -987654321LL, LLONG_MIN, LLONG_MAX }) {
		check(double(T(v)), rne(static_cast<double>(v), fbits), "from long long " + std::to_string(v));
	}
	check(double(T(4000000000u)), rne(4000000000.0, fbits), "from unsigned 4000000000");
	check(double(T(static_cast<char>(-1))), std::is_signed_v<char> ? -1.0 : 255.0, "from char(-1)");
	return nrOfFailedTests;
}

// to a narrower native type the value rounds once: 1 + 2^-24 + 2^-25 is above the tie, so it
// rounds up to 1 + 2^-23 (adding the bits one at a time in float lost both); and triple<0>
// rounds an integer tie with the hidden bit as its odd last bit, so 3 rounds to 4
int VerifySingleRounding(bool reportTestCases) {
	int nrOfFailedTests = 0;
	const triple<52, std::uint32_t> t(1.0 + std::ldexp(1.0, -24) + std::ldexp(1.0, -25));
	if (float(t) != 0x1.000002p0f) {
		++nrOfFailedTests;
		if (reportTestCases) std::cerr << "FAIL: float(triple<52>(1 + 2^-24 + 2^-25)) = " << float(t) << '\n';
	}
	const triple<52, std::uint32_t> tie(1.0 + std::ldexp(1.0, -24));   // exactly a tie: to even, 1
	if (float(tie) != 1.0f) {
		++nrOfFailedTests;
		if (reportTestCases) std::cerr << "FAIL: float(triple<52>(1 + 2^-24)) = " << float(tie) << '\n';
	}
	// to a subnormal float: rounded at the subnormal's precision, not at 24 bits and again
	// in the scaling (0x1.0000028p-127 came out 0x1p-127)
	for (double d : { 0x1.0000028p-127, 0x1.0000018p-127, -0x1.0000038p-130, 0x1.8p-150, 0x1p-150, 0x1.0000001p-150, 0x1p-152, 0x1.fffffep-127 }) {
		const triple<52, std::uint32_t> s(d);
		if (float(s) != static_cast<float>(d)) {   // the cast of the double is correctly rounded
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: float(triple<52>(" << d << ")) = " << float(s) << " expected " << static_cast<float>(d) << '\n';
		}
	}
	for (long long v : { 3LL, 5LL, 6LL, -3LL }) {
		const double expected = rne(static_cast<double>(v), 0);
		if (double(triple<0, std::uint8_t>(v)) != expected) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: triple<0>(" << v << ") = " << double(triple<0, std::uint8_t>(v)) << " expected " << expected << '\n';
		}
	}
	return nrOfFailedTests;
}

int VerifySpecialValues(bool reportTestCases) {
	using T = triple<23, std::uint32_t>;
	const double inf = std::numeric_limits<double>::infinity();
	int nrOfFailedTests = 0;
	auto fail = [&](const char* label) { ++nrOfFailedTests; if (reportTestCases) std::cerr << "FAIL: " << label << '\n'; };
	const T pinf(inf), ninf(-inf), nan(std::numeric_limits<double>::quiet_NaN()), one(1.0), zero(0.0), mzero(-0.0);
	if (double(pinf) != inf) fail("triple(+inf) is +inf");
	if (double(ninf) != -inf) fail("triple(-inf) is -inf");
	if (!std::isnan(double(nan))) fail("triple(NaN) is NaN");
	if (!(-nan).isnan() || !abs(nan).isnan()) fail("negation and abs keep NaN");
	if (double(abs(ninf)) != inf) fail("abs(-inf) is +inf");
	if (nan == nan || !(zero == mzero)) fail("NaN != NaN, +0 == -0");
	if (!(ninf < one) || !(one < pinf) || (pinf < one) || (nan < one) || (nan <= nan)) fail("order of the special values");
	return nrOfFailedTests;
}

// order of finite values: the fractions compare as unsigned numbers (blockbinary is 2's complement)
int VerifyOrder(bool reportTestCases) {
	using T = triple<8, std::uint8_t>;
	int nrOfFailedTests = 0;
	const double values[] = { -3.75, -1.75, -1.5, -1.0, -0.5, 0.0, 0.5, 1.0, 1.5, 1.75, 1.9921875, 3.75 };
	for (double x : values) {
		for (double y : values) {
			const T a(x), b(y);
			if ((a < b) != (x < y) || (a == b) != (x == y) || (a >= b) != (x >= y)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: triple<8> order of " << x << " and " << y << '\n';
			}
		}
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

	std::string test_suite  = "abstract triple";
	std::string test_tag    = "triple";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	nrOfFailedTestCases += VerifyConversion<23>(true);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<10>(reportTestCases), "triple<10>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<23>(reportTestCases), "triple<23>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<52>(reportTestCases), "triple<52>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifySpecialValues(reportTestCases), "triple<23>", "special values");
	nrOfFailedTestCases += ReportTestResult(VerifyOrder(reportTestCases), "triple<8>", "order");
	nrOfFailedTestCases += ReportTestResult(VerifySingleRounding(reportTestCases), "triple", "single rounding");
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
