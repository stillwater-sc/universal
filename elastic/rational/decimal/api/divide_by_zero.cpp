// divide_by_zero.cpp: by default an erational division by zero throws erational_divide_by_zero
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// no exception switch is set: this test checks the defaults (#1894)
#include <universal/number/erational/erational.hpp>
#include <universal/verification/test_reporters.hpp>

static_assert(ERATIONAL_THROW_DIVIDE_BY_ZERO == 1, "a erational division by zero throws by default");
static_assert(ERATIONAL_THROW_ARITHMETIC_EXCEPTION == 0, "the other erational arithmetic errors keep their default");

namespace sw { namespace universal {

// a NaN made directly: x / 0 throws by default, so the NaN comes from a zero denominator
inline erational MakeNaN() {
	erational r;
	r.setnumerator(edecimal(1));
	r.setdenominator(edecimal(0));
	r += erational(0);   // normalize() settles a zero denominator on 0/0
	return r;
}

int VerifyDivideByZeroThrows(bool reportTestCases) {
	int nrOfFailedTests = 0;
	auto expectThrow = [&](auto op, const char* label) {
		try {
			op();
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: " << label << " did not throw\n";
		}
		catch (const erational_divide_by_zero&) {}
	};
	auto expectNaN = [&](const erational& v, const char* label) {
		if (!v.isnan()) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: " << label << " = " << v << " expected NaN\n";
		}
	};
	erational seven(7), minusseven(-7), zero(0);
	expectThrow([&] { erational r = seven / zero; (void)r; }, "7 / 0");
	expectThrow([&] { erational r = minusseven / zero; (void)r; }, "-7 / 0");
	expectThrow([&] { erational r = zero / zero; (void)r; }, "0 / 0");
	expectThrow([&] { erational r(seven); r /= zero; }, "7 /= 0");
	expectThrow([&] { auto d = erational_divide(seven, zero); (void)d; }, "erational_divide(7, 0)");
	// a NaN is not a zero divisor, and arithmetic carries it along without throwing
	erational nan = MakeNaN();
	expectNaN(nan, "1/0 normalized");
	try {
		expectNaN(seven / nan, "7 / NaN");
		expectNaN(nan / seven, "NaN / 7");
		expectNaN(nan + seven, "NaN + 7");
		expectNaN(nan * seven, "NaN * 7");
		expectNaN(erational_divide(seven, nan).quot, "erational_divide(7, NaN)");
	}
	catch (const erational_divide_by_zero&) {
		++nrOfFailedTests;
		if (reportTestCases) std::cerr << "FAIL: arithmetic on a NaN threw erational_divide_by_zero\n";
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

	std::string test_suite  = "erational division by zero, default";
	std::string test_tag    = "divide by zero";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZeroThrows(true), "erational", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZeroThrows(reportTestCases), "erational", test_tag);
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
