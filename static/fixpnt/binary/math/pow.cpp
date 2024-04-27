// pow.cpp: test suite runner for pow function
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// use default library configuration
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/verification/fixpnt_test_suite_mathlib.hpp>

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

	std::string test_suite  = "fixed-point mathlib power function";
	std::string test_tag    = "mathlib pow";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

#if GENERATE_POW_TABLES
	GeneratePowTable<3, 0>();
	GeneratePowTable<4, 0>();
	GeneratePowTable<4, 1>();
	GeneratePowTable<5, 0>();
	GeneratePowTable<5, 1>();
	GeneratePowTable<5, 2>();
	GeneratePowTable<6, 0>();
	GeneratePowTable<6, 1>();
	GeneratePowTable<6, 2>();
	GeneratePowTable<6, 3>();
	GeneratePowTable<7, 0>();
#endif

	cout << endl;

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction< fixpnt<2, 0, Modulo, uint8_t> >(reportTestCases, true), "fixpnt<2,0>", "pow");

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction< fixpnt<3, 0, Modulo, uint8_t> >(reportTestCases, true), "fixpnt<3,0>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction< fixpnt<3, 1, Modulo, uint8_t> >(reportTestCases, true), "fixpnt<3,1>", "pow");

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction< fixpnt<4, 0, Modulo, uint8_t> >(reportTestCases, true), "fixpnt<4,0>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction< fixpnt<4, 1, Modulo, uint8_t> >(reportTestCases, true), "fixpnt<4,1>", "pow");

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction< fixpnt<5, 0, Modulo, uint8_t> >(reportTestCases, true), "fixpnt<5,0>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction< fixpnt<5, 1, Modulo, uint8_t> >(reportTestCases, true), "fixpnt<5,1>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction< fixpnt<5, 2, Modulo, uint8_t> >(reportTestCases, true), "fixpnt<5,2>", "pow");

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction< fixpnt<8, 0, Modulo, uint8_t> >(reportTestCases, true), "fixpnt<8,0>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction< fixpnt<8, 1, Modulo, uint8_t> >(reportTestCases, true), "fixpnt<8,1>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction< fixpnt<8, 4, Modulo, uint8_t> >(reportTestCases, true), "fixpnt<8,4>", "pow");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

	std::cout << "Integer power function\n";
	int a = 2;
	unsigned b = 32;
	std::cout << "2 ^ 32   = " << ipow(a, b) << '\n';
	std::cout << "2 ^ 32   = " << fastipow(a, uint8_t(b)) << '\n';

	int64_t c = 1024;
	uint8_t d = 2;
	std::cout << "1024 ^ 2 = " << ipow(c, d) << '\n';
	std::cout << "1M ^ 2   = " << ipow(ipow(c, d), d) << '\n';

	std::cout << "fixpnt pow() function validation\n";

	using FixedPoint = fixpnt<8, 2, Saturate, uint8_t>;
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction< FixedPoint >(reportTestCases), type_tag(FixedPoint()), "pow");


#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction< fixpnt<2, 0, Modulo, uint8_t> >(reportTestCases, true), "fixpnt<2,0>", "pow");

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction< fixpnt<3, 0, Modulo, uint8_t> >(reportTestCases, true), "fixpnt<3,0>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction< fixpnt<3, 1, Modulo, uint8_t> >(reportTestCases, true), "fixpnt<3,1>", "pow");

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction< fixpnt<4, 0, Modulo, uint8_t> >(reportTestCases, true), "fixpnt<4,0>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction< fixpnt<4, 1, Modulo, uint8_t> >(reportTestCases, true), "fixpnt<4,1>", "pow");

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction< fixpnt<5, 0, Modulo, uint8_t> >(reportTestCases, true), "fixpnt<5,0>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction< fixpnt<5, 1, Modulo, uint8_t> >(reportTestCases, true), "fixpnt<5,1>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction< fixpnt<5, 2, Modulo, uint8_t> >(reportTestCases, true), "fixpnt<5,2>", "pow");

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction< fixpnt<8, 0, Modulo, uint8_t> >(reportTestCases, true), "fixpnt<8,0>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction< fixpnt<8, 1, Modulo, uint8_t> >(reportTestCases, true), "fixpnt<8,1>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction< fixpnt<8, 4, Modulo, uint8_t> >(reportTestCases, true), "fixpnt<8,4>", "pow");
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
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_arithmetic_exception& err) {
	std::cerr << "Uncaught fixpnt arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
//catch (const sw::universal::fixpnt_quire_exception& err) {
//	std::cerr << "Uncaught fixpnt quire exception: " << err.what() << std::endl;
//	return EXIT_FAILURE;
//}
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
