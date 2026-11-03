// divide_by_zero_optout.cpp: with the exception opted out, an erational division by zero is NaN (0/0)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// opt out through the older switch: ERATIONAL_THROW_DIVIDE_BY_ZERO follows it when not set itself (#1894)
#define ERATIONAL_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/erational/erational.hpp>
#include <universal/verification/test_reporters.hpp>

static_assert(ERATIONAL_THROW_DIVIDE_BY_ZERO == 0, "ERATIONAL_THROW_DIVIDE_BY_ZERO follows ERATIONAL_THROW_ARITHMETIC_EXCEPTION");

namespace sw { namespace universal {

int VerifyDivideByZeroValue(bool reportTestCases) {
	int nrOfFailedTests = 0;
	auto expectNaN = [&](const erational& v, const char* label) {
		if (!v.isnan()) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: " << label << " = " << v << " expected NaN\n";
		}
	};
	erational seven(7), minusseven(-7), zero(0);
	expectNaN(seven / zero, "7 / 0");
	expectNaN(minusseven / zero, "-7 / 0");
	expectNaN(zero / zero, "0 / 0");
	erational r(seven); r /= zero; expectNaN(r, "7 /= 0");
	auto d = erational_divide(seven, zero);
	expectNaN(d.quot, "erational_divide(7, 0) quotient");
	expectNaN(d.rem, "erational_divide(7, 0) remainder");
	// the NaN carries through further arithmetic
	erational nan = seven / zero;
	expectNaN(nan + seven, "NaN + 7");
	expectNaN(nan * seven, "NaN * 7");
	expectNaN(nan / seven, "NaN / 7");
	expectNaN(seven / nan, "7 / NaN");
	// a NaN converts to integer 0, instead of throwing out of the noexcept conversion operators
	if (int(nan) != 0 || static_cast<unsigned long long>(nan) != 0ull) {
		++nrOfFailedTests;
		if (reportTestCases) std::cerr << "FAIL: integer conversion of NaN is not 0\n";
	}
	if (seven / erational(2) != erational(7, 2)) {
		++nrOfFailedTests;
		if (reportTestCases) std::cerr << "FAIL: 7 / 2 != 7/2\n";
	}
	return nrOfFailedTests;
}

} } // namespace sw::universal

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

	std::string test_suite  = "erational division by zero, exception opted out";
	std::string test_tag    = "divide by zero";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZeroValue(true), "erational", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZeroValue(reportTestCases), "erational", test_tag);
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
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::erational_arithmetic_exception& err) {
	std::cerr << "Caught unexpected erational arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::erational_internal_exception& err) {
	std::cerr << "Caught unexpected erational internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
