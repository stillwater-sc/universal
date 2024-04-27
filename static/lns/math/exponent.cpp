// exponent.cpp: test suite runner for exponent (exp, exp2, exp10) functions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/verification/lns_test_suite_mathlib.hpp>
#include <universal/verification/test_suite_randoms.hpp>

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

// generate specific test case that you can trace with the trace conditions in lns.hpp
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t rbits, typename Ty>
void GenerateTestCase(Ty a) {
	Ty ref;
	sw::universal::lns<nbits, rbits> pa, pref, pexp;
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

	std::string test_suite  = "lns<> mathlib exponentiation validation";
	std::string test_tag    = "exponentiation";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

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

	lns<8, 2, std::uint8_t> a, aexp2, aref;
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
	nrOfFailedTestCases += ReportTestResult(VerifyExp< lns<8, 2, uint8_t> >(reportTestCases), "lns<8,2>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2< lns<8, 4, uint8_t> >(reportTestCases), "lns<8,4>", "exp2");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyExp< lns< 8, 2, uint8_t> >(reportTestCases), "lns<8,2>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp< lns< 8, 3, uint8_t> >(reportTestCases), "lns<8,3>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp< lns< 9, 2, uint8_t> >(reportTestCases), "lns<9,2>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp< lns<10, 2, uint8_t> >(reportTestCases), "lns<10,2>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp< lns<10, 3, uint8_t> >(reportTestCases), "lns<10,3>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp< lns<12, 4, uint8_t> >(reportTestCases), "lns<12,4>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyExp< lns<16, 5, uint8_t> >(reportTestCases), "lns<16,5>", "exp");

	// base-2 exponent testing
	nrOfFailedTestCases += ReportTestResult(VerifyExp2< lns<8, 2, uint8_t> >(reportTestCases), "lns<8,2>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2< lns<8, 3, uint8_t> >(reportTestCases), "lns<8,3>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2< lns<9, 2, uint8_t> >(reportTestCases), "lns<9,2>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2< lns<10, 2, uint8_t> >(reportTestCases), "lns<10,2>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2< lns<10, 3, uint8_t> >(reportTestCases), "lns<10,3>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2< lns<12, 4, uint8_t> >(reportTestCases), "lns<12,4>", "exp2");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2< lns<16, 5, uint8_t> >(reportTestCases), "lns<16,5>", "exp2");
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyElementaryFunctionThroughRandoms< lns<32, 27, std::uint32_t, Behavior::Wrapping > >(true, RandomsOp::OPCODE_EXP, 1000), "lns<32,27>", "exp");
	nrOfFailedTestCases += ReportTestResult(VerifyElementaryFunctionThroughRandoms< lns<64, 59, std::uint32_t, Behavior::Wrapping > >(true, RandomsOp::OPCODE_EXP2, 1000), "lns<64,59>", "exp2");
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
