// function_exponent.cpp: test suite runner for exponent (exp, exp2, exp10) functions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// when you define ALGORITHM_VERBOSE_OUTPUT the code will print intermediate results for selected arithmetic operations
//#define ALGORITHM_VERBOSE_OUTPUT
#define ALGORITHM_TRACE_EXP

// use default number system library configuration
#include <universal/number/posit1/posit1.hpp>
#include <universal/number/posit1/mathlib.hpp>
#include <universal/verification/posit_test_suite_mathlib.hpp>

// generate specific test case that you can trace with the trace conditions in posit.hpp
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCase(Ty a) {
	Ty ref;
	sw::universal::posit<nbits, es> pa, pref, pexp;
	pa = a;
	ref = std::exp(a);
	pref = ref;
	pexp = sw::universal::exp(pa);
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " -> exp(" << a << ") = " << std::setw(nbits) << ref << std::endl;
	std::cout << pa.get() << " -> exp( " << pa << ") = " << pexp.get() << " (reference: " << pref.get() << ")   ";
	std::cout << (pref == pexp ? "PASS" : "FAIL") << std::endl << std::endl;
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
#define GENERATE_EXPONENT_TABLES 0

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "posit exponent function validation";
	std::string test_tag    = "exponent failed: ";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

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

	cout << endl;

	// manual exhaustive test
	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<2, 0>>(reportTestCases), "posit<2,0>", "exp");

	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<3, 0>>(reportTestCases), "posit<3,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<3, 1>>(reportTestCases), "posit<3,1>", "exp");

	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<4, 0>>(reportTestCases), "posit<4,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<4, 1>>(reportTestCases), "posit<4,1>", "exp");

	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<5, 0>>(reportTestCases), "posit<5,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<5, 1>>(reportTestCases), "posit<5,1>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<5, 2>>(reportTestCases), "posit<5,2>", "exp");

	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<8, 4>>(reportTestCases), "posit<8,4>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<8, 4>>(reportTestCases), "posit<8,4>", "exp2");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

	std::cout << "Posit exponential function validation\n";

	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<2, 0>>(reportTestCases), "posit<2,0>", "exp");

	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<3, 0>>(reportTestCases), "posit<3,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<3, 1>>(reportTestCases), "posit<3,1>", "exp");

	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<4, 0>>(reportTestCases), "posit<4,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<4, 1>>(reportTestCases), "posit<4,1>", "exp");

	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<5, 0>>(reportTestCases), "posit<5,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<5, 1>>(reportTestCases), "posit<5,1>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<5, 2>>(reportTestCases), "posit<5,2>", "exp");

	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<6, 0>>(reportTestCases), "posit<6,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<6, 1>>(reportTestCases), "posit<6,1>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<6, 2>>(reportTestCases), "posit<6,2>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<6, 3>>(reportTestCases), "posit<6,3>", "exp");

	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<7, 0>>(reportTestCases), "posit<7,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<7, 1>>(reportTestCases), "posit<7,1>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<7, 2>>(reportTestCases), "posit<7,2>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<7, 3>>(reportTestCases), "posit<7,3>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<7, 4>>(reportTestCases), "posit<7,4>", "exp");

	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<8, 0>>(reportTestCases), "posit<8,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<8, 1>>(reportTestCases), "posit<8,1>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<8, 2>>(reportTestCases), "posit<8,2>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<8, 3>>(reportTestCases), "posit<8,3>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<8, 4>>(reportTestCases), "posit<8,4>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<8, 5>>(reportTestCases), "posit<8,5>", "exp");

	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<9, 0>>(reportTestCases), "posit<9,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<9, 1>>(reportTestCases), "posit<9,1>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<9, 2>>(reportTestCases), "posit<9,2>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<9, 3>>(reportTestCases), "posit<9,3>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<9, 4>>(reportTestCases), "posit<9,4>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<9, 5>>(reportTestCases), "posit<9,5>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<9, 6>>(reportTestCases), "posit<9,6>", "exp");
	
	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<10, 0>>(reportTestCases), "posit<10,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<10, 1>>(reportTestCases), "posit<10,1>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<10, 2>>(reportTestCases), "posit<10,2>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<10, 7>>(reportTestCases), "posit<10,7>", "exp");

	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<12, 0>>(reportTestCases), "posit<12,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<12, 1>>(reportTestCases), "posit<12,1>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<12, 2>>(reportTestCases), "posit<12,2>", "exp");

	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<16, 0>>(reportTestCases), "posit<16,0>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<16, 1>>(reportTestCases), "posit<16,1>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp<posit<16, 2>>(reportTestCases), "posit<16,2>", "exp");

	// base-2 exponent testing
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<2, 0>>(reportTestCases), "posit<2,0>", "exp2");

	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<3, 0>>(reportTestCases), "posit<3,0>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<3, 1>>(reportTestCases), "posit<3,1>", "exp2");

	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<4, 0>>(reportTestCases), "posit<4,0>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<4, 1>>(reportTestCases), "posit<4,1>", "exp2");

	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<5, 0>>(reportTestCases), "posit<5,0>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<5, 1>>(reportTestCases), "posit<5,1>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<5, 2>>(reportTestCases), "posit<5,2>", "exp2");

	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<6, 0>>(reportTestCases), "posit<6,0>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<6, 1>>(reportTestCases), "posit<6,1>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<6, 2>>(reportTestCases), "posit<6,2>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<6, 3>>(reportTestCases), "posit<6,3>", "exp2");

	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<7, 0>>(reportTestCases), "posit<7,0>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<7, 1>>(reportTestCases), "posit<7,1>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<7, 2>>(reportTestCases), "posit<7,2>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<7, 3>>(reportTestCases), "posit<7,3>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<7, 4>>(reportTestCases), "posit<7,4>", "exp2");

	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<8, 0>>(reportTestCases), "posit<8,0>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<8, 1>>(reportTestCases), "posit<8,1>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<8, 2>>(reportTestCases), "posit<8,2>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<8, 3>>(reportTestCases), "posit<8,3>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<8, 4>>(reportTestCases), "posit<8,4>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<8, 5>>(reportTestCases), "posit<8,5>", "exp2");

	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<9, 0>>(reportTestCases), "posit<9,0>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<9, 1>>(reportTestCases), "posit<9,1>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<9, 2>>(reportTestCases), "posit<9,2>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<9, 3>>(reportTestCases), "posit<9,3>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<9, 4>>(reportTestCases), "posit<9,4>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<9, 5>>(reportTestCases), "posit<9,5>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<9, 6>>(reportTestCases), "posit<9,6>", "exp2");

	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<10, 0>>(reportTestCases), "posit<10,0>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<10, 1>>(reportTestCases), "posit<10,1>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<10, 2>>(reportTestCases), "posit<10,2>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<10, 7>>(reportTestCases), "posit<10,7>", "exp2");

	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<12, 0>>(reportTestCases), "posit<12,0>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<12, 1>>(reportTestCases), "posit<12,1>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<12, 2>>(reportTestCases), "posit<12,2>", "exp2");

	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<16, 0>>(reportTestCases), "posit<16,0>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<16, 1>>(reportTestCases), "posit<16,1>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<posit<16, 2>>(reportTestCases), "posit<16,2>", "exp2");


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
