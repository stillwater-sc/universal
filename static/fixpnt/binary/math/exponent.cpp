// exponent.cpp: test suite runner for exponent (exp, exp2, exp10) functions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// use default number system library configuration
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/fixpnt_test_suite_mathlib.hpp>

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

// generate specific test case
template<size_t nbits, size_t rbits, bool arithmetic, typename bt, typename Ty>
void GenerateTestCase(Ty a) {
	Ty ref;
	sw::universal::fixpnt<nbits, rbits, arithmetic, bt> pa, pref, pexp;
	pa = a;
	ref = std::exp(a);
	pref = ref;
	pexp = sw::universal::exp(pa);
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " -> exp(" << a << ") = " << std::setw(nbits) << ref << std::endl;
	std::cout << to_binary(pa) << " -> exp( " << pa << ") = " << to_binary(pexp) << " (reference: " << to_binary(pref) << ")   ";
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
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif
#define GENERATE_EXPONENT_TABLES 0

int main()
try {
	using namespace sw::universal;

//	GenerateEulersNumber();  // 9000 digits of e

	std::string test_suite  = "fixed-point mathlib exponent";
	std::string test_tag    = "mathlib exponent";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug
	GenerateTestCase<16, 1, Saturate, uint8_t, float>(4.0f);

#if GENERATE_EXPONENT_TABLES

	GenerateExponentTable<5, 1>();
	GenerateExponentTable<5, 2>();
	GenerateExponentTable<6, 1>();
	GenerateExponentTable<6, 2>();
	GenerateExponentTable<6, 3>();
#endif

	// manual exhaustive test
	using FixedPoint = fixpnt<8, 2, Saturate, uint8_t>;
	nrOfFailedTestCases += ReportTestResult(VerifyExp<FixedPoint>(reportTestCases), type_tag(FixedPoint), "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<FixedPoint>(reportTestCases), type_tag(FixedPoint) "exp2");
	
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyExp< fixpnt< 8, 2, Saturate, uint8_t> >(reportTestCases), "fixpnt<8,2>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp< fixpnt< 8, 3, Saturate, uint8_t> >(reportTestCases), "fixpnt<8,3>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp< fixpnt< 9, 2, Saturate, uint8_t> >(reportTestCases), "fixpnt<9,2>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp< fixpnt<10, 2, Saturate, uint8_t> >(reportTestCases), "fixpnt<10,2>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp< fixpnt<10, 3, Saturate, uint8_t> >(reportTestCases), "fixpnt<10,3>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp< fixpnt<12, 4, Saturate, uint8_t> >(reportTestCases), "fixpnt<12,4>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp< fixpnt<16, 5, Saturate, uint8_t> >(reportTestCases), "fixpnt<16,5>", "exp");

	// base-2 exponent testing
	nrOfFailedTestCases += ReportTestResult(VerifyExp2< fixpnt<8, 2, Saturate, uint8_t> >(reportTestCases), "fixpnt<8,2>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2< fixpnt<8, 3, Saturate, uint8_t> >(reportTestCases), "fixpnt<8,3>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2< fixpnt<9, 2, Saturate, uint8_t> >(reportTestCases), "fixpnt<9,2>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2< fixpnt<10, 2, Saturate, uint8_t> >(reportTestCases), "fixpnt<10,2>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2< fixpnt<10, 3, Saturate, uint8_t> >(reportTestCases), "fixpnt<10,3>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2< fixpnt<12, 4, Saturate, uint8_t> >(reportTestCases), "fixpnt<12,4>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2< fixpnt<16, 5, Saturate, uint8_t> >(reportTestCases), "fixpnt<16,5>", "exp2");

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
