// exponent.cpp: test suite runner for exponent (exp, exp2, exp10) functions
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>

// when you define ALGORITHM_VERBOSE_OUTPUT the code will print intermediate results for selected arithmetic operations
//#define ALGORITHM_VERBOSE_OUTPUT
#define ALGORITHM_TRACE_POW

// minimum set of include files to reflect source code dependencies
#include <universal/number/valid/valid_impl.hpp>
#include <universal/number/valid/manipulators.hpp>
//#include <universal/number/valid/math/exponent.hpp>
//#include <universal/verification/valid_math_test_suite.hpp>

// Background: http://numbers.computation.free.fr/Constants/E/e.html
//
// generate digits of Euler's number
void GenerateEulersNumber() {
	int N = 9009, a[9009], x = 0;
	for (int n = N - 1; n > 0; --n) {
		a[n] = 1;
	}
	a[1] = 2;
	while (N > 9) {
		int n = N--;
		while (--n) {
			a[n] = x % n;
			x = 10 * a[n - 1] + x / n;
		}
		std::cout << x;
	}
	std::cout << std::endl;
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


int main(int argc, char** argv)
try {
	using namespace sw::universal;

	std::cout << "VALID exponential function validation\n";
	GenerateEulersNumber();

	int nrOfFailedTestCases = 0;

	std::string tag = "Exponent failed: ";

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug
	GenerateTestCase<16, 1, float>(4.0f);

#if GENERATE_EXPONENT_TABLES
	GenerateExponentTable<3, 0>();
	GenerateExponentTable<4, 0>();
	GenerateExponentTable<4, 1>();
	GenerateExponentTable<5, 0>();
	GenerateExponentTable<5, 1>();
	GenerateExponentTable<5, 2>();
	GenerateExponentTable<6, 0>();
	GenerateExponentTable<6, 1>();
	GenerateExponentTable<6, 2>();
	GenerateExponentTable<6, 3>();
	GenerateExponentTable<7, 0>();
#endif

	std::cout << std::endl;

	bool bReportIndividualTestCases = true;

	nrOfFailedTestCases += ReportTestResult(VerifyExp<2, 0>("Manual Testing", bReportIndividualTestCases), "valid<2,0>", "exp");

	nrOfFailedTestCases += ReportTestResult(VerifyExp<3, 0>("Manual Testing", bReportIndividualTestCases), "valid<3,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<3, 1>("Manual Testing", bReportIndividualTestCases), "valid<3,1>", "exp");

	nrOfFailedTestCases += ReportTestResult(VerifyExp<4, 0>("Manual Testing", bReportIndividualTestCases), "valid<4,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<4, 1>("Manual Testing", bReportIndividualTestCases), "valid<4,1>", "exp");

	nrOfFailedTestCases += ReportTestResult(VerifyExp<5, 0>("Manual Testing", bReportIndividualTestCases), "valid<5,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<5, 1>("Manual Testing", bReportIndividualTestCases), "valid<5,1>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<5, 2>("Manual Testing", bReportIndividualTestCases), "valid<5,2>", "exp");

	nrOfFailedTestCases += ReportTestResult(VerifyExp<8, 4>("Manual Testing", bReportIndividualTestCases), "valid<8,4>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<8, 4>("Manual Testing", bReportIndividualTestCases), "valid<8,4>", "exp2");

#else


#if LATER
#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyExp<2, 0>(tag, bReportIndividualTestCases), "valid<2,0>", "exp");

	nrOfFailedTestCases += ReportTestResult(VerifyExp<3, 0>(tag, bReportIndividualTestCases), "valid<3,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<3, 1>(tag, bReportIndividualTestCases), "valid<3,1>", "exp");

	nrOfFailedTestCases += ReportTestResult(VerifyExp<4, 0>(tag, bReportIndividualTestCases), "valid<4,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<4, 1>(tag, bReportIndividualTestCases), "valid<4,1>", "exp");

	nrOfFailedTestCases += ReportTestResult(VerifyExp<5, 0>(tag, bReportIndividualTestCases), "valid<5,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<5, 1>(tag, bReportIndividualTestCases), "valid<5,1>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<5, 2>(tag, bReportIndividualTestCases), "valid<5,2>", "exp");

	nrOfFailedTestCases += ReportTestResult(VerifyExp<6, 0>(tag, bReportIndividualTestCases), "valid<6,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<6, 1>(tag, bReportIndividualTestCases), "valid<6,1>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<6, 2>(tag, bReportIndividualTestCases), "valid<6,2>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<6, 3>(tag, bReportIndividualTestCases), "valid<6,3>", "exp");

	nrOfFailedTestCases += ReportTestResult(VerifyExp<7, 0>(tag, bReportIndividualTestCases), "valid<7,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<7, 1>(tag, bReportIndividualTestCases), "valid<7,1>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<7, 2>(tag, bReportIndividualTestCases), "valid<7,2>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<7, 3>(tag, bReportIndividualTestCases), "valid<7,3>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<7, 4>(tag, bReportIndividualTestCases), "valid<7,4>", "exp");

	nrOfFailedTestCases += ReportTestResult(VerifyExp<8, 0>(tag, bReportIndividualTestCases), "valid<8,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<8, 1>(tag, bReportIndividualTestCases), "valid<8,1>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<8, 2>(tag, bReportIndividualTestCases), "valid<8,2>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<8, 3>(tag, bReportIndividualTestCases), "valid<8,3>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<8, 4>(tag, bReportIndividualTestCases), "valid<8,4>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<8, 5>(tag, bReportIndividualTestCases), "valid<8,5>", "exp");

	nrOfFailedTestCases += ReportTestResult(VerifyExp<9, 0>(tag, bReportIndividualTestCases), "valid<9,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<9, 1>(tag, bReportIndividualTestCases), "valid<9,1>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<9, 2>(tag, bReportIndividualTestCases), "valid<9,2>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<9, 3>(tag, bReportIndividualTestCases), "valid<9,3>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<9, 4>(tag, bReportIndividualTestCases), "valid<9,4>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<9, 5>(tag, bReportIndividualTestCases), "valid<9,5>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<9, 6>(tag, bReportIndividualTestCases), "valid<9,6>", "exp");
	
	nrOfFailedTestCases += ReportTestResult(VerifyExp<10, 0>(tag, bReportIndividualTestCases), "valid<10,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<10, 1>(tag, bReportIndividualTestCases), "valid<10,1>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<10, 2>(tag, bReportIndividualTestCases), "valid<10,2>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<10, 7>(tag, bReportIndividualTestCases), "valid<10,7>", "exp");

	nrOfFailedTestCases += ReportTestResult(VerifyExp<12, 0>(tag, bReportIndividualTestCases), "valid<12,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<12, 1>(tag, bReportIndividualTestCases), "valid<12,1>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<12, 2>(tag, bReportIndividualTestCases), "valid<12,2>", "exp");

	nrOfFailedTestCases += ReportTestResult(VerifyExp<16, 0>(tag, bReportIndividualTestCases), "valid<16,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<16, 1>(tag, bReportIndividualTestCases), "valid<16,1>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<16, 2>(tag, bReportIndividualTestCases), "valid<16,2>", "exp");

	// base-2 exponent testing
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<2, 0>(tag, bReportIndividualTestCases), "valid<2,0>", "exp2");

	nrOfFailedTestCases += ReportTestResult(VerifyExp2<3, 0>(tag, bReportIndividualTestCases), "valid<3,0>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<3, 1>(tag, bReportIndividualTestCases), "valid<3,1>", "exp2");

	nrOfFailedTestCases += ReportTestResult(VerifyExp2<4, 0>(tag, bReportIndividualTestCases), "valid<4,0>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<4, 1>(tag, bReportIndividualTestCases), "valid<4,1>", "exp2");

	nrOfFailedTestCases += ReportTestResult(VerifyExp2<5, 0>(tag, bReportIndividualTestCases), "valid<5,0>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<5, 1>(tag, bReportIndividualTestCases), "valid<5,1>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<5, 2>(tag, bReportIndividualTestCases), "valid<5,2>", "exp2");

	nrOfFailedTestCases += ReportTestResult(VerifyExp2<6, 0>(tag, bReportIndividualTestCases), "valid<6,0>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<6, 1>(tag, bReportIndividualTestCases), "valid<6,1>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<6, 2>(tag, bReportIndividualTestCases), "valid<6,2>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<6, 3>(tag, bReportIndividualTestCases), "valid<6,3>", "exp2");

	nrOfFailedTestCases += ReportTestResult(VerifyExp2<7, 0>(tag, bReportIndividualTestCases), "valid<7,0>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<7, 1>(tag, bReportIndividualTestCases), "valid<7,1>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<7, 2>(tag, bReportIndividualTestCases), "valid<7,2>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<7, 3>(tag, bReportIndividualTestCases), "valid<7,3>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<7, 4>(tag, bReportIndividualTestCases), "valid<7,4>", "exp2");

	nrOfFailedTestCases += ReportTestResult(VerifyExp2<8, 0>(tag, bReportIndividualTestCases), "valid<8,0>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<8, 1>(tag, bReportIndividualTestCases), "valid<8,1>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<8, 2>(tag, bReportIndividualTestCases), "valid<8,2>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<8, 3>(tag, bReportIndividualTestCases), "valid<8,3>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<8, 4>(tag, bReportIndividualTestCases), "valid<8,4>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<8, 5>(tag, bReportIndividualTestCases), "valid<8,5>", "exp2");

	nrOfFailedTestCases += ReportTestResult(VerifyExp2<9, 0>(tag, bReportIndividualTestCases), "valid<9,0>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<9, 1>(tag, bReportIndividualTestCases), "valid<9,1>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<9, 2>(tag, bReportIndividualTestCases), "valid<9,2>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<9, 3>(tag, bReportIndividualTestCases), "valid<9,3>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<9, 4>(tag, bReportIndividualTestCases), "valid<9,4>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<9, 5>(tag, bReportIndividualTestCases), "valid<9,5>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<9, 6>(tag, bReportIndividualTestCases), "valid<9,6>", "exp2");

	nrOfFailedTestCases += ReportTestResult(VerifyExp2<10, 0>(tag, bReportIndividualTestCases), "valid<10,0>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<10, 1>(tag, bReportIndividualTestCases), "valid<10,1>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<10, 2>(tag, bReportIndividualTestCases), "valid<10,2>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<10, 7>(tag, bReportIndividualTestCases), "valid<10,7>", "exp2");

	nrOfFailedTestCases += ReportTestResult(VerifyExp2<12, 0>(tag, bReportIndividualTestCases), "valid<12,0>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<12, 1>(tag, bReportIndividualTestCases), "valid<12,1>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<12, 2>(tag, bReportIndividualTestCases), "valid<12,2>", "exp2");

	nrOfFailedTestCases += ReportTestResult(VerifyExp2<16, 0>(tag, bReportIndividualTestCases), "valid<16,0>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<16, 1>(tag, bReportIndividualTestCases), "valid<16,1>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<16, 2>(tag, bReportIndividualTestCases), "valid<16,2>", "exp2");
#endif

#if REGRESSION_LEVEL_2

#endif

#if REGRESSION_LEVEL_3

#endif

#if REGRESSION_LEVEL_4

#endif
#endif // LATER

#endif  // MANUAL_TESTING

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
