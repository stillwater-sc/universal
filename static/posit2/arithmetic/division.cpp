// division.cpp: test suite runner for posit2 division
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// Configure the posit template environment
// first: enable general or specialized posit configurations
//#define POSIT_FAST_SPECIALIZATION
// second: enable/disable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 0
// third: enable tracing
// when you define ALGORITHM_VERBOSE_OUTPUT executing a DIV the code will print intermediate results
//#define ALGORITHM_VERBOSE_OUTPUT
//#define ALGORITHM_TRACE_DIV
#include <universal/number/posit2/posit.hpp>
#include <universal/verification/posit_test_suite.hpp>

// generate specific test case that you can trace with the trace conditions in posit.h
// for most bugs they are traceable with _trace_conversion and _trace_div
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCase(Ty a, Ty b) {
	Ty ref;
	sw::universal::posit<nbits, es> pa, pb, pref, pdiv;
	pa = a;
	pb = b;
	ref = a / b;
	pref = ref;
	pdiv = pa / pb;
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " / " << std::setw(nbits) << b << " = " << std::setw(nbits) << ref << std::endl;
	std::cout << pa.get() << " / " << pb.get() << " = " << pdiv.get() << " (reference: " << pref.get() << ")  ";
	std::cout << (pref == pdiv ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
}

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

	std::string test_suite  = "posit2 division verification";
	std::string test_tag    = "division";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// generate individual testcases to hand trace/debug
	GenerateTestCase<4, 0, double>(0.5, 1.0);
	GenerateTestCase<4, 0, double>(0.5, -1.0);
	GenerateTestCase<8, 0, double>(1.0, 0.5);

	// manual exhaustive testing
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<4, 0>>(true), "posit<4,0>", "division");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<2, 0>>(reportTestCases), "posit< 2,0>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<3, 0>>(reportTestCases), "posit< 3,0>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<3, 1>>(reportTestCases), "posit< 3,1>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<4, 0>>(reportTestCases), "posit< 4,0>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<4, 1>>(reportTestCases), "posit< 4,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<4, 2>>(reportTestCases), "posit< 4,2>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<5, 0>>(reportTestCases), "posit< 5,0>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<5, 1>>(reportTestCases), "posit< 5,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<5, 2>>(reportTestCases), "posit< 5,2>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<5, 3>>(reportTestCases), "posit< 5,3>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<6, 0>>(reportTestCases), "posit< 6,0>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<6, 1>>(reportTestCases), "posit< 6,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<6, 2>>(reportTestCases), "posit< 6,2>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<6, 3>>(reportTestCases), "posit< 6,3>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<6, 4>>(reportTestCases), "posit< 6,4>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<7, 0>>(reportTestCases), "posit< 7,0>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<7, 1>>(reportTestCases), "posit< 7,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<7, 2>>(reportTestCases), "posit< 7,2>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<7, 3>>(reportTestCases), "posit< 7,3>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<7, 4>>(reportTestCases), "posit< 7,4>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<8, 0>>(reportTestCases), "posit< 8,0>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<8, 1>>(reportTestCases), "posit< 8,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<8, 2>>(reportTestCases), "posit< 8,2>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<8, 3>>(reportTestCases), "posit< 8,3>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<8, 4>>(reportTestCases), "posit< 8,4>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<8, 5>>(reportTestCases), "posit< 8,5>", "division");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<10, 0>>(reportTestCases), "posit<10,0>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<10, 1>>(reportTestCases), "posit<10,1>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<10, 2>>(reportTestCases), "posit<10,2>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<posit<10, 3>>(reportTestCases), "posit<10,3>", "division");
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
