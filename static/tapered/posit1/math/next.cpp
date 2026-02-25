// next.cpp: test suite runner for ULP functions nextafter, nextforward
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// use default number system library configuration
#include <universal/number/posit1/posit1.hpp>
#include <universal/verification/test_suite_mathlib.hpp>

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

	std::string test_suite  = "posit<> nextafter/toward validation";
	std::string test_tag    = "nextafter/toward";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

	using Posit = posit<32,2>;
	Posit p{1.0f}, pplus{1.0f}, pminus{1.0f};
	++pplus;
	--pminus;

	Posit nut;
	nut = nextafter(p, Posit(10));
	if (nut != pplus) ++nrOfFailedTestCases;
	std::cout << to_binary(nut) << " reference is " << to_binary(pplus) << '\n';
	nut = nexttoward(p, Posit(10));
	if (nut != pplus) ++nrOfFailedTestCases;
	std::cout << to_binary(nut) << " reference is " << to_binary(pplus) << '\n';
	nut = nextafter(p, Posit(-10));
	if (nut != pminus) ++nrOfFailedTestCases;
	std::cout << to_binary(nut) << " reference is " << to_binary(pminus) << '\n';
	nut = nexttoward(p, Posit(-10));
	if (nut != pminus) ++nrOfFailedTestCases;
	std::cout << to_binary(nut) << " reference is " << to_binary(pminus) << '\n';


	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyNextafter< posit<  8, 2> >(reportTestCases), "posit<  8, 2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNextafter< posit< 16, 2> >(reportTestCases), "posit< 16, 2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNextafter< posit< 32, 2> >(reportTestCases), "posit< 32, 2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNextafter< posit< 64, 2> >(reportTestCases), "posit< 64, 2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNextafter< posit< 80, 2> >(reportTestCases), "posit< 80, 2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNextafter< posit<128, 2> >(reportTestCases), "posit<128, 2>", test_tag);

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
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
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
