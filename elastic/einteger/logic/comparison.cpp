// comparison.cpp: test suite runner for logic comparisons on adaptive precision binary integers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//  SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>
#include <limits>

// minimum set of include files to reflect source code dependencies
#include <universal/number/einteger/einteger.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	// the value selectors against the native value; isone() used to report true for every value (#1878)
	template<typename BlockType>
	int VerifySelectors(bool reportTestCases) {
		using EInteger = einteger<BlockType>;
		int nrOfFailedTests = 0;
		for (long long v : { 0ll, 1ll, -1ll, 2ll, -2ll, 3ll, 255ll, 256ll, 257ll, -257ll, 65536ll, 65537ll, 4294967297ll, -4294967297ll }) {
			const EInteger x(v);
			if (x.iszero() != (v == 0) || x.isone() != (v == 1) || x.isodd() != (v % 2 != 0) || x.iseven() != (v % 2 == 0) || x.isneg() != (v < 0) || x.ispos() != (v >= 0)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: selectors of " << v << ": iszero " << x.iszero() << " isone " << x.isone() << " isodd " << x.isodd() << " isneg " << x.isneg() << '\n';
			}
		}
		const EInteger empty;   // the default-constructed state has no limbs
		if (!empty.iszero() || empty.isone()) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: default-constructed einteger selectors\n";
		}
		return nrOfFailedTests;
	}

}} // namespace sw::universal

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
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "einteger logic validation";
	std::string test_tag    = "logic";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else


#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifySelectors<std::uint8_t>(reportTestCases), "einteger<uint8_t>", "selectors");
	nrOfFailedTestCases += ReportTestResult(VerifySelectors<std::uint16_t>(reportTestCases), "einteger<uint16_t>", "selectors");
	nrOfFailedTestCases += ReportTestResult(VerifySelectors<std::uint32_t>(reportTestCases), "einteger<uint32_t>", "selectors");
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
