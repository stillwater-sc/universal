// function_pow.cpp: test suite runner for pow function
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// when you define ALGORITHM_VERBOSE_OUTPUT the code will print intermediate results for selected arithmetic operations
//#define ALGORITHM_VERBOSE_OUTPUT
#define ALGORITHM_TRACE_POW

// use default number system library configuration
#include <universal/number/posit1/posit1.hpp>
#include <universal/verification/posit_test_suite_mathlib.hpp>
#include <universal/native/integers.hpp> // for ipow

// generate specific test case that you can trace with the trace conditions in posit.hpp
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCase(Ty a, Ty b) {
	Ty ref;
	sw::universal::posit<nbits, es> pa, pb, pref, ppow;
	pa = a;
	pb = b;
	ref = std::pow(a,b);
	pref = ref;
	ppow = sw::universal::pow(pa,pb);
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << " -> pow(" << a << "," << b << ") = " << std::setw(nbits) << ref << std::endl;
	std::cout << " -> pow( " << pa << "," << pb << ") = " << ppow.get() << " (reference: " << pref.get() << ")   " ;
	std::cout << (pref == ppow ? "PASS" : "FAIL") << std::endl << std::endl;
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
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "posit power function validation";
	std::string test_tag    = "power failed: ";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug
	GenerateTestCase<16, 1, float>(4.0f, 2.0f);

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

	// manual exhaustive test
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<2, 0>(reportTestCases), "posit<2,0>", "pow");

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<3, 0>(reportTestCases), "posit<3,0>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<3, 1>(reportTestCases), "posit<3,1>", "pow");

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<4, 0>(reportTestCases), "posit<4,0>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<4, 1>(reportTestCases), "posit<4,1>", "pow");

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<5, 0>(reportTestCases), "posit<5,0>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<5, 1>(reportTestCases), "posit<5,1>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<5, 2>(reportTestCases), "posit<5,2>", "pow");

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<8, 0>(reportTestCases), "posit<8,0>", "pow");	
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<8, 1>(reportTestCases), "posit<8,1>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<8, 4>(reportTestCases), "posit<8,4>", "pow");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else


	std::cout << "Integer power function\n";
	int a = 2;
	unsigned int b = 32;
	std::cout << "2 ^ 32   = " << ipow(a, b) << '\n';
	std::cout << "2 ^ 32   = " << fastipow(a, uint8_t(b)) << '\n';

	int64_t c = 1024;
	uint8_t d = 2;
	std::cout << "1024 ^ 2 = " << ipow(c, unsigned(d)) << '\n';
	std::cout << "1M ^ 2   = " << ipow(ipow(c, d), d) << '\n';

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<2, 0>>(reportTestCases), "posit<2,0>", "pow");

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<3, 0>>(reportTestCases), "posit<3,0>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<3, 1>>(reportTestCases), "posit<3,1>", "pow");

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<4, 0>>(reportTestCases), "posit<4,0>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<4, 1>>(reportTestCases), "posit<4,1>", "pow");

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<5, 0>>(reportTestCases), "posit<5,0>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<5, 1>>(reportTestCases), "posit<5,1>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<5, 2>>(reportTestCases), "posit<5,2>", "pow");

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<6, 0>>(reportTestCases), "posit<6,0>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<6, 1>>(reportTestCases), "posit<6,1>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<6, 2>>(reportTestCases), "posit<6,2>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<6, 3>>(reportTestCases), "posit<6,3>", "pow");

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<7, 0>>(reportTestCases), "posit<7,0>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<7, 1>>(reportTestCases), "posit<7,1>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<7, 2>>(reportTestCases), "posit<7,2>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<7, 3>>(reportTestCases), "posit<7,3>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<7, 4>>(reportTestCases), "posit<7,4>", "pow");

	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<8, 0>>(reportTestCases), "posit<8,0>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<8, 1>>(reportTestCases), "posit<8,1>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<8, 2>>(reportTestCases), "posit<8,2>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<8, 3>>(reportTestCases), "posit<8,3>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<8, 4>>(reportTestCases), "posit<8,4>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<8, 5>>(reportTestCases), "posit<8,5>", "pow");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<9, 0>>(reportTestCases), "posit<9,0>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<9, 1>>(reportTestCases), "posit<9,1>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<9, 2>>(reportTestCases), "posit<9,2>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<9, 3>>(reportTestCases), "posit<9,3>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<9, 4>>(reportTestCases), "posit<9,4>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<9, 5>>(reportTestCases), "posit<9,5>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<9, 6>>(reportTestCases), "posit<9,6>", "pow");
	
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<10, 0>>(reportTestCases), "posit<10,0>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<10, 1>>(reportTestCases), "posit<10,1>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<10, 2>>(reportTestCases), "posit<10,2>", "pow");
	// fails due to regime representation not being able to be represented by double
	// nrOfFailedTestCases += ReportTestResult(VerifyPowMethod<posit<10, 7>>(reportTestCases), "posit<10,7>", "pow");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<12, 0>>(reportTestCases), "posit<12,0>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<12, 1>>(reportTestCases), "posit<12,1>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<12, 2>>(reportTestCases), "posit<12,2>", "pow");
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<16, 0>>(reportTestCases), "posit<16,0>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<16, 1>>(reportTestCases), "posit<16,1>", "pow");
	nrOfFailedTestCases += ReportTestResult(VerifyPowerFunction<posit<16, 2>>(reportTestCases), "posit<16,2>", "pow");
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
