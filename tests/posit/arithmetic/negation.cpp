// arithmetic_negate.cpp: test suite runner for posit arithmetic negation
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// Configure the posit template environment
// first: enable general or specialized specialized posit configurations
//#define POSIT_FAST_SPECIALIZATION
// second: enable/disable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 0
// third: enable tracing 
// when you define POSIT_VERBOSE_OUTPUT executing an negation the code will print intermediate results
//#define POSIT_VERBOSE_OUTPUT
#define POSIT_TRACE_NEGATE

// minimum set of include files to reflect source code dependencies
#include <universal/number/posit/posit_impl.hpp>
#include <universal/number/posit/numeric_limits.hpp>
#include <universal/number/posit/specializations.hpp>
#include <universal/number/posit/manipulators.hpp>
#include <universal/verification/posit_test_suite.hpp>

// generate specific test case that you can trace with the trace conditions in posit.h
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCase(Ty fa) {
	sw::universal::posit<nbits, es> pa, pref, pneg;
	pa = fa;
	pref = -fa;
	pneg = -pa;
	std::cout << "reference " << pref << " result " << pneg << std::endl << std::endl;
}

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::cout << "Posit negation validation\n";

	std::string tag = "Negation failed: ";

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug
	GenerateTestCase<5, 0, float>(-0.625f);
	GenerateTestCase<5, 0, float>(-0.500f);

	nrOfFailedTestCases += ReportTestResult(VerifyNegation<5, 0>("Manual Testing: ", true), "posit<5,0>", "multiplication");

#else


	nrOfFailedTestCases += ReportTestResult(VerifyNegation<2, 0>(bReportIndividualTestCases), "posit<2,0>", "negation");

	nrOfFailedTestCases += ReportTestResult(VerifyNegation<3, 0>(bReportIndividualTestCases), "posit<3,0>", "negation");
	nrOfFailedTestCases += ReportTestResult(VerifyNegation<3, 1>(bReportIndividualTestCases), "posit<3,1>", "negation");

	nrOfFailedTestCases += ReportTestResult(VerifyNegation<4, 0>(bReportIndividualTestCases), "posit<4,0>", "negation");
	nrOfFailedTestCases += ReportTestResult(VerifyNegation<4, 1>(bReportIndividualTestCases), "posit<4,1>", "negation");

	nrOfFailedTestCases += ReportTestResult(VerifyNegation<5, 0>(bReportIndividualTestCases), "posit<5,0>", "negation");
	nrOfFailedTestCases += ReportTestResult(VerifyNegation<5, 1>(bReportIndividualTestCases), "posit<5,1>", "negation");
	nrOfFailedTestCases += ReportTestResult(VerifyNegation<5, 2>(bReportIndividualTestCases), "posit<5,2>", "negation");

	nrOfFailedTestCases += ReportTestResult(VerifyNegation<6, 0>(bReportIndividualTestCases), "posit<6,0>", "negation");
	nrOfFailedTestCases += ReportTestResult(VerifyNegation<6, 1>(bReportIndividualTestCases), "posit<6,1>", "negation");
	nrOfFailedTestCases += ReportTestResult(VerifyNegation<6, 2>(bReportIndividualTestCases), "posit<6,2>", "negation");
	nrOfFailedTestCases += ReportTestResult(VerifyNegation<6, 3>(bReportIndividualTestCases), "posit<6,3>", "negation");

	nrOfFailedTestCases += ReportTestResult(VerifyNegation<7, 0>(bReportIndividualTestCases), "posit<7,0>", "negation");
	nrOfFailedTestCases += ReportTestResult(VerifyNegation<7, 1>(bReportIndividualTestCases), "posit<7,1>", "negation");
	nrOfFailedTestCases += ReportTestResult(VerifyNegation<7, 2>(bReportIndividualTestCases), "posit<7,2>", "negation");
	nrOfFailedTestCases += ReportTestResult(VerifyNegation<7, 3>(bReportIndividualTestCases), "posit<7,3>", "negation");
	nrOfFailedTestCases += ReportTestResult(VerifyNegation<7, 4>(bReportIndividualTestCases), "posit<7,4>", "negation");

	nrOfFailedTestCases += ReportTestResult(VerifyNegation<8, 0>(bReportIndividualTestCases), "posit<8,0>", "negation");
	nrOfFailedTestCases += ReportTestResult(VerifyNegation<8, 1>(bReportIndividualTestCases), "posit<8,1>", "negation");
	nrOfFailedTestCases += ReportTestResult(VerifyNegation<8, 2>(bReportIndividualTestCases), "posit<8,2>", "negation");
	nrOfFailedTestCases += ReportTestResult(VerifyNegation<8, 3>(bReportIndividualTestCases), "posit<8,3>", "negation");
	nrOfFailedTestCases += ReportTestResult(VerifyNegation<8, 4>(bReportIndividualTestCases), "posit<8,4>", "negation");
	nrOfFailedTestCases += ReportTestResult(VerifyNegation<8, 5>(bReportIndividualTestCases), "posit<8,5>", "negation");

	nrOfFailedTestCases += ReportTestResult(VerifyNegation<10, 1>(bReportIndividualTestCases), "posit<10,1>", "negation");
	nrOfFailedTestCases += ReportTestResult(VerifyNegation<12, 1>(bReportIndividualTestCases), "posit<12,1>", "negation");
	nrOfFailedTestCases += ReportTestResult(VerifyNegation<16, 1>(bReportIndividualTestCases), "posit<16,1>", "negation");

#if STRESS_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifyNegation<20, 1>(bReportIndividualTestCases), "posit<20,1>", "negation");
	nrOfFailedTestCases += ReportTestResult(VerifyNegation<24, 1>(bReportIndividualTestCases), "posit<24,1>", "negation");

#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING
	
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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

