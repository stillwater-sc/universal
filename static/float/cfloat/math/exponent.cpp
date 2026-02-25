// exponent.cpp: test suite runner for exponent (exp, exp2, exp10) functions
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
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/cfloat_test_suite_mathlib.hpp>

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

// generate specific test case that you can trace with the trace conditions in cfloat.h
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCase(Ty a) {
	Ty ref;
	sw::universal::cfloat<nbits, es> pa, pref, pexp;
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

#define MANUAL_TESTING 0
#define GENERATE_EXPONENT_TABLES 0

int main()
try {
	using namespace sw::universal;

//	GenerateEulersNumber();  // 9000 digits of e

	std::string test_suite  = "cfloat<> mathlib exponentiation validation";
	std::string test_tag    = "exponentiation";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug
	GenerateTestCase<16, 1, float>(4.0f);

#if GENERATE_EXPONENT_TABLES

	GenerateExponentTable<5, 1>();
	GenerateExponentTable<5, 2>();
	GenerateExponentTable<6, 1>();
	GenerateExponentTable<6, 2>();
	GenerateExponentTable<6, 3>();
#endif

	cfloat<8, 2> a, aexp2, aref;
	a.setbits(0xFF);
	aexp2 = sw::universal::exp2(a);
	// generate reference
	double da = double(a);
	double dref = std::exp2(da);
	aref = dref;
	cout << to_binary(aref) << " : " << aref << " : " << to_binary(dref) << endl;
	cout << to_binary(ieee754_parameter<double>::fmask) << endl;
	cout << to_binary(ieee754_parameter<double>::snanmask) << endl;

	cout << endl;

	// manual exhaustive test
	nrOfFailedTestCases += ReportTestResult(VerifyExp< cfloat<8, 2, uint8_t> >(reportTestCases), "cfloat<8,2>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2< cfloat<8, 4, uint8_t> >(reportTestCases), "cfloat<8,4>", "exp2");

#else

	std::cout << "classic floating-point cfloat exponential function validation\n";

	nrOfFailedTestCases += ReportTestResult(VerifyExp< cfloat< 8, 2, uint8_t> >(reportTestCases), "cfloat<8,2>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp< cfloat< 8, 3, uint8_t> >(reportTestCases), "cfloat<8,3>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp< cfloat< 9, 2, uint8_t> >(reportTestCases), "cfloat<9,2>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp< cfloat<10, 2, uint8_t> >(reportTestCases), "cfloat<10,2>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp< cfloat<10, 3, uint8_t> >(reportTestCases), "cfloat<10,3>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp< cfloat<12, 4, uint8_t> >(reportTestCases), "cfloat<12,4>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp< cfloat<16, 5, uint8_t> >(reportTestCases), "cfloat<16,5>", "exp");

	// base-2 exponent testing
	nrOfFailedTestCases += ReportTestResult(VerifyExp2< cfloat<8, 2, uint8_t> >(reportTestCases), "cfloat<8,2>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2< cfloat<8, 3, uint8_t> >(reportTestCases), "cfloat<8,3>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2< cfloat<9, 2, uint8_t> >(reportTestCases), "cfloat<9,2>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2< cfloat<10, 2, uint8_t> >(reportTestCases), "cfloat<10,2>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2< cfloat<10, 3, uint8_t> >(reportTestCases), "cfloat<10,3>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2< cfloat<12, 4, uint8_t> >(reportTestCases), "cfloat<12,4>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2< cfloat<16, 5, uint8_t> >(reportTestCases), "cfloat<16,5>", "exp2");


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
