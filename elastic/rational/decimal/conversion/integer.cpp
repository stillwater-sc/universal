//  integer.cpp : test suite for conversion of adaptive precision decimal rational numbers to native integers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <universal/number/erational/erational.hpp>
#include <universal/verification/test_suite.hpp>

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

namespace {

	using sw::universal::erational;

	int check(bool ok, const std::string& what, bool reportTestCases) {
		if (!ok && reportTestCases) std::cerr << "FAIL: " << what << '\n';
		return ok ? 0 : 1;
	}

	// every signed conversion of r gives the integer part, truncated toward zero, with its sign;
	// every unsigned conversion gives that value wrapped modulo 2^n, as C++ converts a signed int
	int VerifyIntegerPart(const erational& r, long long expected, const std::string& tag, bool reportTestCases) {
		int fails = 0;
		fails += check(static_cast<short>(r)     == static_cast<short>(expected), tag + " short",     reportTestCases);
		fails += check(static_cast<int>(r)       == static_cast<int>(expected),   tag + " int",       reportTestCases);
		fails += check(static_cast<long>(r)      == static_cast<long>(expected),  tag + " long",      reportTestCases);
		fails += check(static_cast<long long>(r) == expected,                     tag + " long long", reportTestCases);
		fails += check(static_cast<unsigned int>(r)       == static_cast<unsigned int>(expected),       tag + " unsigned int",       reportTestCases);
		fails += check(static_cast<unsigned long>(r)      == static_cast<unsigned long>(expected),      tag + " unsigned long",      reportTestCases);
		fails += check(static_cast<unsigned long long>(r) == static_cast<unsigned long long>(expected), tag + " unsigned long long", reportTestCases);
		return fails;
	}

}

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "erational conversion to native integers ";
	std::string test_tag    = "integer conversion";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	erational a(-7);
	std::cout << a << " -> int " << int(a) << ", unsigned " << unsigned(a) << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else // !MANUAL_TESTING

#if REGRESSION_LEVEL_1
	{
		// the sign lives in a flag, not in the digits: every integer conversion must apply it (#1908)
		int fails = 0;
		fails += VerifyIntegerPart(erational(7),                  7, "7",        reportTestCases);
		fails += VerifyIntegerPart(erational(-7),                -7, "-7",       reportTestCases);
		fails += VerifyIntegerPart(erational(0) - erational(7),  -7, "0 - 7",    reportTestCases);
		fails += VerifyIntegerPart(erational(-3) * erational(5), -15, "-3 * 5",  reportTestCases);
		fails += VerifyIntegerPart(erational(-7, 2),             -3, "-7/2",     reportTestCases);
		fails += VerifyIntegerPart(erational(7, -2),             -3, "7/-2",     reportTestCases);
		fails += VerifyIntegerPart(erational(-7, -2),             3, "-7/-2",    reportTestCases);
		fails += VerifyIntegerPart(erational(-3, 2),             -1, "-3/2",     reportTestCases);
		fails += VerifyIntegerPart(erational(-1, 2),              0, "-1/2",     reportTestCases);
		fails += VerifyIntegerPart(erational(0) * erational(-3),  0, "0 * -3 (sign-flagged zero)", reportTestCases);
		fails += VerifyIntegerPart(erational(-2.75),             -2, "-2.75",    reportTestCases);
		fails += VerifyIntegerPart(erational(-1000000000000LL), -1000000000000LL, "-1e12", reportTestCases);
		nrOfFailedTestCases += ReportTestResult(fails, "erational", "signed integer part");
	}
	{
		// the 64-bit edges, through edecimal's conversion: INT64_MIN's magnitude 2^63 and values
		// above LLONG_MAX used to overflow its signed accumulator (#1908)
		constexpr long long          llmin  = std::numeric_limits<long long>::min();
		constexpr unsigned long long ullmax = std::numeric_limits<unsigned long long>::max();
		int fails = 0;
		fails += VerifyIntegerPart(erational(llmin, 1), llmin, "INT64_MIN/1", reportTestCases);
		fails += VerifyIntegerPart(erational(llmin),    llmin, "INT64_MIN",   reportTestCases);
		fails += check(static_cast<unsigned long long>(erational(ullmax)) == ullmax, "ULLONG_MAX -> unsigned long long", reportTestCases);
		fails += check(static_cast<unsigned long long>(erational(ullmax, 1u)) == ullmax, "ULLONG_MAX/1 -> unsigned long long", reportTestCases);
		nrOfFailedTestCases += ReportTestResult(fails, "erational", "64-bit edges");
	}
	{
		// NaN has no integer value: 0, without throwing out of the noexcept operators
		erational nan;
		nan.setnan();
		nrOfFailedTestCases += ReportTestResult(VerifyIntegerPart(nan, 0, "nan", reportTestCases), "erational", "NaN");
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
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::erational_arithmetic_exception& err) {
	std::cerr << "Uncaught arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::erational_internal_exception& err) {
	std::cerr << "Uncaught internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
