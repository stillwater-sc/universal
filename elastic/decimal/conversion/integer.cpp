//  integer.cpp : test suite for conversion of adaptive precision decimal integers to native integers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <limits>
#include <string>
#include <universal/number/edecimal/edecimal.hpp>
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

	// each conversion gives the value as long long would, then narrowed as C++ narrows it
	int VerifyNarrowing(long long v, bool reportTestCases) {
		const sw::universal::edecimal d(v);
		int fails = 0;
		auto check = [&](bool ok, const char* type) {
			if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL: " << type << '(' << v << ")\n"; }
		};
		check(static_cast<short>(d)     == static_cast<short>(v), "short");
		check(static_cast<int>(d)       == static_cast<int>(v),   "int");
		check(static_cast<long>(d)      == static_cast<long>(v),  "long");
		check(static_cast<long long>(d) == v,                     "long long");
		check(static_cast<unsigned int>(d)       == static_cast<unsigned int>(v),       "unsigned int");
		check(static_cast<unsigned long>(d)      == static_cast<unsigned long>(v),      "unsigned long");
		check(static_cast<unsigned long long>(d) == static_cast<unsigned long long>(v), "unsigned long long");
		return fails;
	}

}

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "edecimal conversion to native integers ";
	std::string test_tag    = "integer conversion";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	edecimal d(100000);
	std::cout << d << " -> int " << int(d) << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else // !MANUAL_TESTING

#if REGRESSION_LEVEL_1
	{
		// int and long used to narrow through short: int(edecimal(100000)) was -31072 (#1908)
		int fails = 0;
		for (long long v : { 0LL, 1LL, -1LL, 32767LL, 32768LL, -32769LL, 100000LL, -100000LL,
		                     2147483647LL, -2147483648LL, 1000000000000LL, -1000000000000LL }) {
			fails += VerifyNarrowing(v, reportTestCases);
		}
		nrOfFailedTestCases += ReportTestResult(fails, "edecimal", "narrowing to native integers");
	}
	{
		// the 64-bit edges: the accumulator used to be a signed long long, which overflowed (UB) on
		// the magnitude 2^63 of INT64_MIN and on every value above LLONG_MAX (#1908)
		constexpr long long          llmin  = std::numeric_limits<long long>::min();
		constexpr long long          llmax  = std::numeric_limits<long long>::max();
		constexpr unsigned long long ullmax = std::numeric_limits<unsigned long long>::max();
		int fails = 0;
		fails += VerifyNarrowing(llmin, reportTestCases);
		fails += VerifyNarrowing(llmax, reportTestCases);
		auto check = [&](bool ok, const char* what) {
			if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL: " << what << '\n'; }
		};
		check(static_cast<unsigned long long>(edecimal(ullmax)) == ullmax, "edecimal(ULLONG_MAX) -> unsigned long long");
		check(static_cast<unsigned long long>(edecimal(9223372036854775808ull)) == 9223372036854775808ull, "edecimal(2^63) -> unsigned long long");
		check(static_cast<long long>(edecimal(ullmax)) == -1LL, "edecimal(ULLONG_MAX) -> long long wraps to -1");
		check(static_cast<unsigned long long>(edecimal(-1LL)) == ullmax, "edecimal(-1) -> unsigned long long wraps to ULLONG_MAX");
		nrOfFailedTestCases += ReportTestResult(fails, "edecimal", "64-bit edges");
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
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
