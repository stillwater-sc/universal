// assignment.cpp: test suite runner for assignment on adaptive precision decimal integers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>
#include <limits>

// minimum set of include files to reflect source code dependencies
#include <universal/number/edecimal/edecimal.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	// a native value converted to edecimal must equal the expected decimal digits, and zero must not be negative
	template<typename Real>
	int VerifyNativeConversion(Real v, const std::string& expected, bool reportTestCases) {
		edecimal d(v);
		edecimal ref;
		ref.parse(expected);
		if (d != ref || (d.iszero() && d.isneg())) {
			if (reportTestCases) std::cerr << "FAIL: edecimal(" << v << ") = " << d << ", expected " << expected << '\n';
			return 1;
		}
		return 0;
	}

	// finite values, truncated toward zero, plus NaN and +/-inf, which edecimal maps to 0 (#1989)
	template<typename Real>
	int VerifyIeee754Assignment(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		nrOfFailedTestCases += VerifyNativeConversion(Real(0), "0", reportTestCases);
		nrOfFailedTestCases += VerifyNativeConversion(Real(-0.0), "0", reportTestCases);
		nrOfFailedTestCases += VerifyNativeConversion(Real(0.4), "0", reportTestCases);
		nrOfFailedTestCases += VerifyNativeConversion(Real(-0.4), "0", reportTestCases);
		nrOfFailedTestCases += VerifyNativeConversion(Real(2.5), "2", reportTestCases);
		nrOfFailedTestCases += VerifyNativeConversion(Real(-2.5), "-2", reportTestCases);
		nrOfFailedTestCases += VerifyNativeConversion(Real(1024), "1024", reportTestCases);
		nrOfFailedTestCases += VerifyNativeConversion(Real(-1048576), "-1048576", reportTestCases);
		nrOfFailedTestCases += VerifyNativeConversion(Real(1099511627776.0), "1099511627776", reportTestCases);   // 2^40
		nrOfFailedTestCases += VerifyNativeConversion(std::numeric_limits<Real>::quiet_NaN(), "0", reportTestCases);
		nrOfFailedTestCases += VerifyNativeConversion(-std::numeric_limits<Real>::quiet_NaN(), "0", reportTestCases);
		nrOfFailedTestCases += VerifyNativeConversion(std::numeric_limits<Real>::infinity(), "0", reportTestCases);
		nrOfFailedTestCases += VerifyNativeConversion(-std::numeric_limits<Real>::infinity(), "0", reportTestCases);
		return nrOfFailedTestCases;
	}

}}  // namespace sw::universal

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

	std::string test_suite  = "adaptive precision decimal integer assignment validation";
	std::string test_tag    = "assignment";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	edecimal d(std::numeric_limits<double>::infinity());
	std::cout << "edecimal(inf) = " << d << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyIeee754Assignment<float>(reportTestCases), "float", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIeee754Assignment<double>(reportTestCases), "double", test_tag);
	// 1e20 and 2^64 are exact in double
	nrOfFailedTestCases += ReportTestResult(VerifyNativeConversion(1.0e20, "100000000000000000000", reportTestCases), "double", "1e20");
	nrOfFailedTestCases += ReportTestResult(VerifyNativeConversion(18446744073709551616.0, "18446744073709551616", reportTestCases), "double", "2^64");
#if LONG_DOUBLE_SUPPORT
	nrOfFailedTestCases += ReportTestResult(VerifyIeee754Assignment<long double>(reportTestCases), "long double", test_tag);
#endif
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
