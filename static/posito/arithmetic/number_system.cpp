// number_system.cpp: test suite runner for full number system check
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// Configure the posit template environment
// first: enable general or specialized posit configurations
#//define POSITO_FAST_SPECIALIZATION
// second: enable/disable posit arithmetic exceptions
#define POSITO_THROW_ARITHMETIC_EXCEPTION 0
// third: enable tracing 
// when you define ALGORITHM_VERBOSE_OUTPUT executing an ADD the code will print intermediate results
//#define ALGORITHM_VERBOSE_OUTPUT
//#define ALGORITHM_TRACE_ADD
#include <universal/number/posit/posit.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posito/posito.hpp>
#include <universal/verification/posit_number_system.hpp>

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
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

	std::string test_suite  = "posito number system validation";
	std::string test_tag    = "number system";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	
	// TestType: posit<nbits, es, uint8_t>
	nrOfFailedTestCases += ExhaustiveNumberSystemTest<posito<2, 0>, posito<3, 0>>("posito<2,0>", reportTestCases);
	nrOfFailedTestCases += ExhaustiveNumberSystemTest<posito<3, 0>, posito<4, 0>>("posito<3,0>", reportTestCases);
//	nrOfFailedTestCases += ExhaustiveNumberSystemTest<posito<3, 1>, posito<4, 1>>("posito<3,1>", true);
	nrOfFailedTestCases += ExhaustiveNumberSystemTest<posito<4, 0>, posito<5, 0>>("posito<4,0>", reportTestCases);
	nrOfFailedTestCases += ExhaustiveNumberSystemTest<posito<5, 2>, posito<6, 2>>("posito<5,2>", reportTestCases);
	nrOfFailedTestCases += ExhaustiveNumberSystemTest<posito<8, 0>, posito<9, 0>>("posito<8,0>", reportTestCases);
//	nrOfFailedTestCases += ExhaustiveNumberSystemTest<posito<8, 2>, posit<9, 2>>("posito<8,2>", reportTestCases);

//	nrOfFailedTestCases += ExhaustiveNumberSystemTest<posito<16, 1>, posito<17, 1>>("posito<16,1>", reportTestCases);
//	nrOfFailedTestCases += ExhaustiveNumberSystemTest<posito<16, 2>, posito<17, 2>>("posito<16,2>", reportTestCases);

//	nrOfFailedTestCases += ExhaustiveNumberSystemTest<posito<8, 0>, posito<9, 0>>("posito<8,0>", reportTestCases);

//	nrOfFailedTestCases += ExhaustiveNumberSystemTest<cfloat<8, 5>, cfloat<9, 5>>("cfloat<8,5>", reportTestCases);
//	nrOfFailedTestCases += ExhaustiveNumberSystemTest<posito<7, 0>, posito<8, 0>>("posito<7,0>", reportTestCases);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ExhaustiveNumberSystemTest<posit<8, 0>, posit<9, 0>>("posit<8,0>", reportTestCases);
	nrOfFailedTestCases += ExhaustiveNumberSystemTest<posito<8, 0>, posito<9, 0>>("posito<8,0>", reportTestCases);
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ExhaustiveNumberSystemTest<posit<16, 2>, posit<9, 0>>("posit<8,0>", reportTestCases);
	nrOfFailedTestCases += ExhaustiveNumberSystemTest<posito<16, 2>, posito<9, 0>>("posito<8,0>", reportTestCases);
#endif // REGRESSION_LEVEL_4

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
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
