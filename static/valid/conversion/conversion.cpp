// conversion.cpp : test suite runner for conversion operators to valid numbers
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// if you want to trace the valid intermediate results
// #define ALGORITHM_VERBOSE_OUTPUT
#define VALID_TRACE_CONVERT
// enable the ability to use literals in binary logic and arithmetic operators
#define VALID_ENABLE_LITERALS 1
// minimum set of include files to reflect source code dependencies
#include <universal/number/valid/valid_impl.hpp>
#include <universal/number/valid/manipulators.hpp>
#include <universal/verification/test_suite_arithmetic.hpp>

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	std::cout << "VALID conversion validation\n";
	int nrOfFailedTestCases = 0;

	std::string tag = "Conversion test";

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

	bool bReportIndividualTestCases = false;
	// manual exhaustive testing
	tag = "Manual Testing";

#ifdef VERBOSE_ENUMERATION_TESTING
	GenerateLogicPatternsForDebug<3, 0>();
	GenerateLogicPatternsForDebug<4, 0>();	
	GenerateLogicPatternsForDebug<4, 1>();
	GenerateLogicPatternsForDebug<5, 1>();
	GenerateLogicPatternsForDebug<5, 2>();
	GenerateLogicPatternsForDebug<6, 2>();
	GenerateLogicPatternsForDebug<7, 3>();
	GenerateLogicPatternsForDebug<8, 0>();
	GenerateLogicPatternsForDebug<8, 1>();
	GenerateLogicPatternsForDebug<8, 2>();
	std::cout << "----------------\n";
#endif

	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<3, 0>(tag, true), "valid<3,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<4, 0>(tag, true), "valid<4,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<5, 0>(tag, true), "valid<5,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<6, 0>(tag, true), "valid<6,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<7, 0>(tag, true), "valid<7,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<8, 0>(tag, true), "valid<8,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<9, 0>(tag, true), "valid<9,0>", "conversion");

	nrOfFailedTestCases += ReportTestResult(VerifyConversion<3, 0>(tag, true), "valid<3,0>", "conversion");

	nrOfFailedTestCases += ReportTestResult(VerifyConversion<4, 1>(tag, true), "valid<4,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<5, 2>(tag, true), "valid<5,2>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<6, 3>(tag, true), "valid<6,3>", "conversion");

	nrOfFailedTestCases += ReportTestResult(VerifyConversion<4, 0>(tag, true), "valid<4,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<4, 1>(tag, true), "valid<4,1>", "conversion"); 
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<5, 0>(tag, true), "valid<5,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<5, 1>(tag, true), "valid<5,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<5, 2>(tag, true), "valid<5,2>", "conversion");

	nrOfFailedTestCases += ReportTestResult(VerifyAddition<6, 0>("Posit<6,0> addition failed: ", bReportIndividualTestCases), "valid<6,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<6, 1>("Posit<6,1> addition failed: ", bReportIndividualTestCases), "valid<6,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<6, 2>("Posit<6,2> addition failed: ", bReportIndividualTestCases), "valid<6,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<6, 3>("Posit<6,3> addition failed: ", bReportIndividualTestCases), "valid<6,3>", "addition");

#else


#if 0
#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<3, 0>(tag, bReportIndividualTestCases), "valid<3,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<4, 0>(tag, bReportIndividualTestCases), "valid<4,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<5, 0>(tag, bReportIndividualTestCases), "valid<5,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<6, 0>(tag, bReportIndividualTestCases), "valid<6,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<7, 0>(tag, bReportIndividualTestCases), "valid<7,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<8, 0>(tag, bReportIndividualTestCases), "valid<8,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion<9, 0>(tag, bReportIndividualTestCases), "valid<9,0>", "conversion");

	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 3, 0>(tag, bReportIndividualTestCases), "valid<3,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 4, 0>(tag, bReportIndividualTestCases), "valid<4,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 5, 0>(tag, bReportIndividualTestCases), "valid<5,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 6, 0>(tag, bReportIndividualTestCases), "valid<6,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 7, 0>(tag, bReportIndividualTestCases), "valid<7,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 8, 0>(tag, bReportIndividualTestCases), "valid<8,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 9, 0>(tag, bReportIndividualTestCases), "valid<9,0>", "conversion");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 4, 1>(tag, bReportIndividualTestCases), "valid<4,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 5, 1>(tag, bReportIndividualTestCases), "valid<5,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 6, 1>(tag, bReportIndividualTestCases), "valid<6,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 7, 1>(tag, bReportIndividualTestCases), "valid<7,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 8, 1>(tag, bReportIndividualTestCases), "valid<8,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 9, 1>(tag, bReportIndividualTestCases), "valid<9,1>", "conversion");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 5, 2>(tag, bReportIndividualTestCases), "valid<5,2>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 6, 2>(tag, bReportIndividualTestCases), "valid<6,2>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 7, 2>(tag, bReportIndividualTestCases), "valid<7,2>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 8, 2>(tag, bReportIndividualTestCases), "valid<8,2>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 9, 2>(tag, bReportIndividualTestCases), "valid<9,2>", "conversion");

	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 6, 3>(tag, bReportIndividualTestCases), "valid<6,3>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 7, 3>(tag, bReportIndividualTestCases), "valid<7,3>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 8, 3>(tag, bReportIndividualTestCases), "valid<8,3>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion< 9, 3>(tag, bReportIndividualTestCases), "valid<9,3>", "conversion");
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<10, 0>(tag, bReportIndividualTestCases), "valid<10,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<10, 1>(tag, bReportIndividualTestCases), "valid<10,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<10, 2>(tag, bReportIndividualTestCases), "valid<10,2>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<10, 3>(tag, bReportIndividualTestCases), "valid<10,3>", "conversion");

	nrOfFailedTestCases += ReportTestResult(VerifyConversion<12, 0>(tag, bReportIndividualTestCases), "valid<12,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<12, 1>(tag, bReportIndividualTestCases), "valid<12,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<12, 2>(tag, bReportIndividualTestCases), "valid<12,2>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<12, 3>(tag, bReportIndividualTestCases), "valid<12,3>", "conversion");

	nrOfFailedTestCases += ReportTestResult(VerifyConversion<14, 0>(tag, bReportIndividualTestCases), "valid<14,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<14, 1>(tag, bReportIndividualTestCases), "valid<14,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<14, 2>(tag, bReportIndividualTestCases), "valid<14,2>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<14, 3>(tag, bReportIndividualTestCases), "valid<14,3>", "conversion");

	nrOfFailedTestCases += ReportTestResult(VerifyConversion<16, 0>(tag, bReportIndividualTestCases), "valid<16,0>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<16, 1>(tag, bReportIndividualTestCases), "valid<16,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<16, 2>(tag, bReportIndividualTestCases), "valid<16,2>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<16, 3>(tag, bReportIndividualTestCases), "valid<16,3>", "conversion");

#endif

#endif // 0 till we got the valid conversion test suite written

#endif // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
/*
catch (const sw::universal::valid_arithmetic_exception& err) {
	std::cerr << "Uncaught valid arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::valid_internal_exception& err) {
	std::cerr << "Uncaught valid internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
*/
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}

