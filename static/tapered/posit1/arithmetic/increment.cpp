// increment.cpp: test suite runner for increment operator
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/posit1/posit1.hpp>
#include <universal/verification/posit_test_suite_mathlib.hpp>

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

	std::string test_suite  = "posit increment verification";
	std::string test_tag    = "increment";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	constexpr size_t nbits = 5;
	constexpr size_t es = 0;
	using Scalar = posit<nbits, es>;
	const std::string positConfig = "posit<5,0>";
	std::vector< Scalar > set;
	GenerateOrderedPositSet<nbits, es>(set);
	for_each(begin(set), end(set), [](const Scalar& s){
		std::cout << s.get() << " " << s << std::endl;
	});

	nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<nbits, es>>(reportTestCases), positConfig, test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else
	// Note: increment/decrement depend on the 2's complement ordering of the posit encoding
	// This implies that this functionality is independent of the <nbits,es> configuration of the posit.
	// Otherwise stated, an enumeration of tests for different posit configurations is a bit superfluous.

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<3, 0>>(reportTestCases), "posit<3,0>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<4, 0>>(reportTestCases), "posit<4,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<4, 1>>(reportTestCases), "posit<4,1>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<5, 0>>(reportTestCases), "posit<5,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<5, 1>>(reportTestCases), "posit<5,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<5, 2>>(reportTestCases), "posit<5,2>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<6, 0>>(reportTestCases), "posit<6,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<6, 1>>(reportTestCases), "posit<6,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<6, 2>>(reportTestCases), "posit<6,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<6, 3>>(reportTestCases), "posit<6,3>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<7, 0>>(reportTestCases), "posit<7,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<7, 1>>(reportTestCases), "posit<7,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<7, 2>>(reportTestCases), "posit<7,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<7, 3>>(reportTestCases), "posit<7,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<7, 4>>(reportTestCases), "posit<7,4>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<8, 0>>(reportTestCases), "posit<8,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<8, 1>>(reportTestCases), "posit<8,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<8, 2>>(reportTestCases), "posit<8,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<8, 3>>(reportTestCases), "posit<8,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<8, 4>>(reportTestCases), "posit<8,4>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<8, 5>>(reportTestCases), "posit<8,5>", test_tag);
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
	// AD/DA adapted data path configurations
	nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<10, 0>>(reportTestCases), "posit<10,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<10, 1>>(reportTestCases), "posit<10,1>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<12, 0>>(reportTestCases), "posit<12,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<12, 1>>(reportTestCases), "posit<12,1>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<14, 0>>(reportTestCases), "posit<14,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<14, 1>>(reportTestCases), "posit<14,1>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<15, 0>>(reportTestCases), "posit<15,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<15, 1>>(reportTestCases), "posit<15,1>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<16, 0>>(reportTestCases), "posit<16,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<16, 1>>(reportTestCases), "posit<16,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<16, 2>>(reportTestCases), "posit<16,2>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<18, 0>>(reportTestCases), "posit<18,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<18, 1>>(reportTestCases), "posit<18,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<18, 2>>(reportTestCases), "posit<18,2>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<20, 1>>(reportTestCases), "posit<20,1>", test_tag);
		
	// legit float replacement
	//nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<24, 1>>(reportTestCases), "posit<24,1>", test_tag);
	//nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<28, 2>>(reportTestCases), "posit<28,2>", test_tag);

	// legit double replacement
	//nrOfFailedTestCases += ReportTestResult(VerifyIncrement<posit<32, 2>>(reportTestCases), "posit<32,2>", test_tag);
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
