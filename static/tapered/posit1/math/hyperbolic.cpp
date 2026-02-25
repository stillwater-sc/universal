// function_hyperbolic.cpp: test suite runner for hyperbolic functions (sinh/cosh/tanh/atanh/acosh/asinh)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// when you define ALGORITHM_VERBOSE_OUTPUT the code will print intermediate results for selected arithmetic operations
//#define ALGORITHM_VERBOSE_OUTPUT
#define ALGORITHM_TRACE_SQRT

// use default number system library configuration
#include <universal/number/posit1/posit1.hpp>
#include <universal/verification/posit_test_suite_mathlib.hpp>

// generate specific test case that you can trace with the trace conditions in posit.hpp
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCaseSinh(Ty a) {
	Ty ref;
	sw::universal::posit<nbits, es> pa, pref, psinh;
	pa = a;
	ref = std::sinh(a);
	pref = ref;
	psinh = sw::universal::sinh(pa);
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " -> sinh(" << a << ") = " << std::setw(nbits) << ref << std::endl;
	std::cout << pa.get() << " -> sinh( " << pa << ") = " << psinh.get() << " (reference: " << pref.get() << ")   " ;
	std::cout << (pref == psinh ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
}

template<size_t nbits, size_t es, typename Ty>
void GenerateTestCaseCosh(Ty a) {
	Ty ref;
	sw::universal::posit<nbits, es> pa, pref, pcosh;
	pa = a;
	ref = std::cosh(a);
	pref = ref;
	pcosh = sw::universal::cosh(pa);
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " -> cosh(" << a << ") = " << std::setw(nbits) << ref << std::endl;
	std::cout << pa.get() << " -> cosh( " << pa << ") = " << pcosh.get() << " (reference: " << pref.get() << ")   ";
	std::cout << (pref == pcosh ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
}

template<size_t nbits, size_t es, typename Ty>
void GenerateTestCaseTanh(Ty a) {
	Ty ref;
	sw::universal::posit<nbits, es> pa, pref, ptanh;
	pa = a;
	ref = std::tanh(a);
	pref = ref;
	ptanh = sw::universal::tanh(pa);
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " -> tanh(" << a << ") = " << std::setw(nbits) << ref << std::endl;
	std::cout << pa.get() << " -> tanh( " << pa << ") = " << ptanh.get() << " (reference: " << pref.get() << ")   ";
	std::cout << (pref == ptanh ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
}

template<size_t nbits, size_t es, typename Ty>
void GenerateTestCaseAsinh(Ty a) {
	Ty ref;
	sw::universal::posit<nbits, es> pa, pref, pasinh;
	pa = a;
	ref = std::asinh(a);
	pref = ref;
	pasinh = sw::universal::asinh(pa);
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " -> asinh(" << a << ") = " << std::setw(nbits) << ref << std::endl;
	std::cout << pa.get() << " -> asinh( " << pa << ") = " << pasinh.get() << " (reference: " << pref.get() << ")   ";
	std::cout << (pref == pasinh ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
}

template<size_t nbits, size_t es, typename Ty>
void GenerateTestCaseAcosh(Ty a) {
	Ty ref;
	sw::universal::posit<nbits, es> pa, pref, pacosh;
	pa = a;
	ref = std::acosh(a);
	pref = ref;
	pacosh = sw::universal::acosh(pa);
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " -> acosh(" << a << ") = " << std::setw(nbits) << ref << std::endl;
	std::cout << pa.get() << " -> acosh( " << pa << ") = " << pacosh.get() << " (reference: " << pref.get() << ")   ";
	std::cout << (pref == pacosh ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
}

template<size_t nbits, size_t es, typename Ty>
void GenerateTestCaseAtanh(Ty a) {
	Ty ref;
	sw::universal::posit<nbits, es> pa, pref, patanh;
	pa = a;
	ref = std::atanh(a);
	pref = ref;
	patanh = sw::universal::atanh(pa);
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " -> atanh(" << a << ") = " << std::setw(nbits) << ref << std::endl;
	std::cout << pa.get() << " -> atanh( " << pa << ") = " << patanh.get() << " (reference: " << pref.get() << ")   ";
	std::cout << (pref == patanh ? "PASS" : "FAIL") << std::endl << std::endl;
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

	std::string test_suite  = "posit hyperbolic sine/cosine/tangent function validation";
	std::string test_tag    = "classification failed: ";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug
	GenerateTestCaseSinh<16, 1, double>(pi / 4.0);
	GenerateTestCaseCosh<16, 1, double>(pi / 4.0);
	GenerateTestCaseTanh<16, 1, double>(pi / 4.0);
	GenerateTestCaseAsinh<16, 1, double>(pi / 2.0);
	GenerateTestCaseAcosh<16, 1, double>(pi / 2.0);
	GenerateTestCaseAtanh<16, 1, double>(pi / 4.0);

	// manual exhaustive test
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<2, 0>>(reportTestCases), "posit<2,0>", "sinh");

	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<3, 0>>(reportTestCases), "posit<3,0>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<3, 1>>(reportTestCases), "posit<3,1>", "sinh");

	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<4, 0>>(reportTestCases), "posit<4,0>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<4, 1>>(reportTestCases), "posit<4,1>", "sinh");

	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<5, 0>>(reportTestCases), "posit<5,0>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<5, 1>>(reportTestCases), "posit<5,1>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<5, 2>>(reportTestCases), "posit<5,2>", "sinh");

	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<8, 0>>(reportTestCases), "posit<8,0>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifyCosh< posit<8, 0>>(reportTestCases), "posit<8,0>", "cosh");
	nrOfFailedTestCases += ReportTestResult(VerifyTanh< posit<8, 0>>(reportTestCases), "posit<8,0>", "tanh");
	nrOfFailedTestCases += ReportTestResult(VerifyAtanh< posit<8, 0>>(reportTestCases), "posit<8,0>", "atanh");
	nrOfFailedTestCases += ReportTestResult(VerifyAcosh< posit<8, 0>>(reportTestCases), "posit<8,0>", "acosh");
	nrOfFailedTestCases += ReportTestResult(VerifyAsinh< posit<8, 0>>(reportTestCases), "posit<8,0>", "asinh");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<2, 0>>(reportTestCases), "posit<2,0>", "sinh");

	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<3, 0>>(reportTestCases), "posit<3,0>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<3, 1>>(reportTestCases), "posit<3,1>", "sinh");

	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<4, 0>>(reportTestCases), "posit<4,0>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<4, 1>>(reportTestCases), "posit<4,1>", "sinh");

	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<5, 0>>(reportTestCases), "posit<5,0>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<5, 1>>(reportTestCases), "posit<5,1>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<5, 2>>(reportTestCases), "posit<5,2>", "sinh");

	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<6, 0>>(reportTestCases), "posit<6,0>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<6, 1>>(reportTestCases), "posit<6,1>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<6, 2>>(reportTestCases), "posit<6,2>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<6, 3>>(reportTestCases), "posit<6,3>", "sinh");

	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<7, 0>>(reportTestCases), "posit<7,0>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<7, 1>>(reportTestCases), "posit<7,1>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<7, 2>>(reportTestCases), "posit<7,2>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<7, 3>>(reportTestCases), "posit<7,3>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<7, 4>>(reportTestCases), "posit<7,4>", "sinh");

//	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<8, 0>>(reportTestCases), "posit<8,0>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<8, 1>>(reportTestCases), "posit<8,1>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<8, 2>>(reportTestCases), "posit<8,2>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<8, 3>>(reportTestCases), "posit<8,3>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<8, 4>>(reportTestCases), "posit<8,4>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<8, 5>>(reportTestCases), "posit<8,5>", "sinh");

	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<8, 0>>(reportTestCases), "posit<8,0>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifyCosh<posit<8, 0>>(reportTestCases), "posit<8,0>", "cosh");
	nrOfFailedTestCases += ReportTestResult(VerifyTanh<posit<8, 0>>(reportTestCases), "posit<8,0>", "tanh");
	nrOfFailedTestCases += ReportTestResult(VerifyAtanh<posit<8, 0>>(reportTestCases), "posit<8,0>", "atanh");
	nrOfFailedTestCases += ReportTestResult(VerifyAcosh<posit<8, 0>>(reportTestCases), "posit<8,0>", "acosh");
	nrOfFailedTestCases += ReportTestResult(VerifyAsinh<posit<8, 0>>(reportTestCases), "posit<8,0>", "asinh");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<9, 0>>(reportTestCases), "posit<9,0>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<9, 1>>(reportTestCases), "posit<9,1>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<9, 2>>(reportTestCases), "posit<9,2>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<9, 3>>(reportTestCases), "posit<9,3>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<9, 4>>(reportTestCases), "posit<9,4>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<9, 5>>(reportTestCases), "posit<9,5>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<9, 6>>(reportTestCases), "posit<9,6>", "sinh");
	
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<10, 0>>(reportTestCases), "posit<10,0>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<10, 1>>(reportTestCases), "posit<10,1>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<10, 2>>(reportTestCases), "posit<10,2>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<10, 7>>(reportTestCases), "posit<10,7>", "sinh");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<12, 0>>(reportTestCases), "posit<12,0>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<12, 1>>(reportTestCases), "posit<12,1>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<12, 2>>(reportTestCases), "posit<12,2>", "sinh");

	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<16, 0>>(reportTestCases), "posit<16,0>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<16, 1>>(reportTestCases), "posit<16,1>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<16, 2>>(reportTestCases), "posit<16,2>", "sinh");
#endif

#if REGRESSION_LEVEL_4
	// nbits=64 requires long double compiler support
	// nrOfFailedTestCases += ReportTestResult(VerifyThroughRandoms<64, 2>>(reportTestCases, OPCODE_SQRT, 1000), "posit<64,2>", "sinh");

	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<10, 1>>(reportTestCases), "posit<10,1>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<12, 1>>(reportTestCases), "posit<12,1>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<14, 1>>(reportTestCases), "posit<14,1>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifySinh<posit<16, 1>>(reportTestCases), "posit<16,1>", "sinh");
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
