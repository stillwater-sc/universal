// next.cpp: test suite runner for ULP functions nextafter, nextforward
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// use default number system library configuration
#include <universal/number/posit/posit.hpp>
#include <universal/verification/posit_math_test_suite.hpp>

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::cout << "Posit nextafter/forward function validation\n";
	bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	std::string tag = "nextafter/toward failed: ";

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


	return (nrOfFailedTestCases > 0) ? EXIT_FAILURE : EXIT_SUCCESS;

#else

#if REGRESSION_LEVEL_1

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
