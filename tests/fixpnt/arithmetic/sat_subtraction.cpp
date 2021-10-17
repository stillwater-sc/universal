// sat_subtraction.cpp: test suite runner for arbitrary configuration fixed-point saturating subtraction
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>

// Configure the fixpnt template environment
// first: enable general or specialized fixed-point configurations
#define FIXPNT_FAST_SPECIALIZATION
// second: enable/disable fixpnt arithmetic exceptions
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/verification/fixpnt_test_suite.hpp>

// generate specific test case that you can trace with the trace conditions in fixed_point.hpp
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t rbits, typename Ty>
void GenerateTestCase(Ty _a, Ty _b) {
	Ty ref;
	sw::universal::fixpnt<nbits, rbits, sw::universal::Saturating> a, b, cref, result;
	a = _a;
	b = _b;
	result = a - b;
	ref = _a - _b;
	cref = ref;
	std::streamsize oldPrecision = std::cout.precision();
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << _a << " - " << std::setw(nbits) << _b << " = " << std::setw(nbits) << ref << std::endl;
	std::cout << a << " - " << b << " = " << result << " (reference: " << cref << ")   " ;
	std::cout << (cref == result ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::dec << std::setprecision(oldPrecision);
}

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

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "fixed-point saturating subtraction ";
	std::string test_tag = "saturating subtraction";
	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

#if MANUAL_TESTING

	// generate individual testcases to hand trace/debug
	GenerateTestCase<4, 1>(3.5f, 3.5f);
	GenerateTestCase<4, 1>(-4.0f, -0.5f);
	GenerateTestCase<4, 1>(-4.0f, 0.5f);
	GenerateTestCase<4, 1>(-1.5f, 3.5f);
	GenerateTestCase<4, 1>(-4.0f, -4.0f);

	bReportIndividualTestCases = true;
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<4, 1, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt<4,1,Saturating,uint8_t>", test_tag);

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<4, 0, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt<4,0,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<4, 1, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt<4,1,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<4, 2, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt<4,2,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<4, 3, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt<4,3,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<4, 4, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt<4,4,Saturating,uint8_t>", test_tag);
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< 5, 0, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt< 5, 0,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< 5, 1, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt< 5, 1,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< 5, 2, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt< 5, 2,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< 5, 3, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt< 5, 3,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< 5, 4, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt< 5, 4,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< 5, 5, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt< 5, 5,Saturating,uint8_t>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< 7, 0, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt< 7, 0,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< 7, 1, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt< 7, 1,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< 7, 2, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt< 7, 2,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< 7, 3, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt< 7, 3,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< 7, 4, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt< 7, 4,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< 7, 5, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt< 7, 5,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< 7, 6, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt< 7, 6,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< 7, 7, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt< 7, 7,Saturating,uint8_t>", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< 8, 0, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt< 8, 0,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< 8, 1, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt< 8, 1,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< 8, 2, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt< 8, 2,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< 8, 3, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt< 8, 3,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< 8, 4, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt< 8, 4,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< 8, 5, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt< 8, 5,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< 8, 6, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt< 8, 6,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< 8, 7, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt< 8, 7,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< 8, 8, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt< 8, 8,Saturating,uint8_t>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< 9, 3, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt<9,3,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< 9, 5, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt<9,5,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction< 9, 7, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt<9,7,Saturating,uint8_t>", test_tag);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<13, 0, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt<13, 0,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<13, 5, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt<13, 5,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<13, 9, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt<13, 9,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<13,12, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt<13,12,Saturating,uint8_t>", test_tag);
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<15, 3, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt<15, 3,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<15, 6, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt<15, 6,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<15, 9, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt<15, 9,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<15,12, Saturating, uint8_t>(bReportIndividualTestCases), "fixpnt<15,12,Saturating,uint8_t>", test_tag);
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_arithmetic_exception& err) {
	std::cerr << "Uncaught fixpnt arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_internal_exception& err) {
	std::cerr << "Uncaught fixpnt internal exception: " << err.what() << std::endl;
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
