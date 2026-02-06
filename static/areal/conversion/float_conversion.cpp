// float_conversion.cpp: test suite runner for IEEE float conversions to areals
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// Configure the areal template environment
// first: enable general or specialized configurations
#define AREAL_FAST_SPECIALIZATION
// second: enable/disable arithmetic exceptions
#define AREAL_THROW_ARITHMETIC_EXCEPTION 0
// third: enable trace conversion
#define TRACE_CONVERSION 0

#include <universal/number/areal/areal.hpp>
#include <universal/verification/test_suite_arithmetic.hpp>
#include <universal/verification/areal_test_suite.hpp>
#include <universal/number/areal/table.hpp> // only used for value table generation

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
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "areal float conversion";
	std::string test_tag    = "conversion";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// areal<> is organized as a set of exact samples and an interval to the next exact value
	//
	// vprev    exact value          ######-0     ubit = false     some value [vprev,vprev]
	//          interval value       ######-1     ubit = true      (vprev, v)
	// v        exact value          ######-0     ubit = false     some value [v,v]
	//          interval value       ######-1     ubit = true      (v, vnext)
	// vnext    exact value          ######-0     ubit = false     some value [vnext,vnext]
	//          interval value       ######-1     ubit = true      (vnext, vnextnext)
	//
	// the assignment test can thus be constructed by enumerating the exact values
	// and taking a -diff to obtain the interval value of vprev, 
	// and taking a +diff to obtain the interval value of v

//	GenerateArealTable<10, 1>(cout, true); 

	{
		areal<11, 8> a;
		a.set_raw_bits(0x002); 
		float f;
		f = float(a);
		std::cout << to_binary(a) << " : " << a << " : " << f << endl;
	}

	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<11, 8, uint8_t>, float >(true), test_tag, "areal<11,8,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<11, 8, uint16_t>, float >(false), test_tag, "areal<11,8,uint16_t>");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1

	// es = 1
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<4, 1>, float >(reportTestCases), test_tag, "areal<4,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<5, 1>, float >(reportTestCases), test_tag, "areal<5,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<6, 1>, float >(reportTestCases), test_tag, "areal<6,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<7, 1>, float >(reportTestCases), test_tag, "areal<7,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<8, 1>, float >(reportTestCases), test_tag, "areal<8,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<9, 1>, float >(reportTestCases), test_tag, "areal<9,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<10, 1>, float >(reportTestCases), test_tag, "areal<10,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<12, 1>, float >(reportTestCases), test_tag, "areal<12,1>");


	// es = 2
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<5, 2>, float >(reportTestCases), test_tag, "areal<5,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<6, 2>, float >(reportTestCases), test_tag, "areal<6,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<7, 2>, float >(reportTestCases), test_tag, "areal<7,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<8, 2>, float >(reportTestCases), test_tag, "areal<8,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<10, 2>, float >(reportTestCases), test_tag, "areal<10,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<12, 2>, float >(reportTestCases), test_tag, "areal<12,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<14, 2>, float >(reportTestCases), test_tag, "areal<14,2>");


	// es = 3
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<6, 3>, float >(reportTestCases), test_tag, "areal<6,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<7, 3>, float >(reportTestCases), test_tag, "areal<7,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<8, 3>, float >(reportTestCases), test_tag, "areal<8,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<10, 3>, float >(reportTestCases), test_tag, "areal<10,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<12, 3>, float >(reportTestCases), test_tag, "areal<12,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<14, 3>, float >(reportTestCases), test_tag, "areal<14,3>");


	// es = 4
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<7, 4>, float >(reportTestCases), test_tag, "areal<7,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<8, 4>, float >(reportTestCases), test_tag, "areal<8,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<10, 4>, float >(reportTestCases), test_tag, "areal<10,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<12, 4>, float >(reportTestCases), test_tag, "areal<12,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<14, 4>, float >(reportTestCases), test_tag, "areal<14,4>");


	// es = 5
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<8, 5>, float >(reportTestCases), test_tag, "areal<8,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<10, 5>, float >(reportTestCases), test_tag, "areal<10,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<12, 5>, float >(reportTestCases), test_tag, "areal<12,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<14, 5>, float >(reportTestCases), test_tag, "areal<14,5>");


	// es = 6
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<9, 6>, float >(reportTestCases), test_tag, "areal<9,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<10, 6>, float >(reportTestCases), test_tag, "areal<10,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<12, 6>, float >(reportTestCases), test_tag, "areal<12,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<14, 6>, float >(reportTestCases), test_tag, "areal<14,6>");


	// es = 7
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<10, 7>, float >(reportTestCases), test_tag, "areal<10,7>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<12, 7>, float >(reportTestCases), test_tag, "areal<12,7>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<14, 7>, float >(reportTestCases), test_tag, "areal<14,7>");

	// es = 8
//	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<11, 8>, float >(reportTestCases), test_tag, "areal<11,8>");
//	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<12, 8>, float >(reportTestCases), test_tag, "areal<12,8>");
//	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<14, 8>, float >(reportTestCases), test_tag, "areal<14,8>");
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
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
