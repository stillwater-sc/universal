// hyperbolic.cpp: test suite runner for hyperbolic functions (sinh/cosh/tanh/atanh/acosh/asinh) using logarithmic floats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// use default library configuration
#include <universal/number/lns/lns.hpp>
#include <universal/verification/lns_test_suite_mathlib.hpp>

// generate specific test case that you can trace with the trace conditions in lns.hpp
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t rbits, typename bt, typename Ty>
void GenerateTestCaseSinh(Ty v) {
	Ty ref;
	sw::universal::lns<nbits, rbits, bt> a, aref, asinh;
	a = v;
	ref = std::sinh(v);
	aref = ref;
	asinh = sw::universal::sinh(a);
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " -> sinh(" << a << ") = " << std::setw(nbits) << ref << std::endl;
	std::cout << sw::universal::to_binary(a) << " -> sinh( " << a << ") = " << sw::universal::to_binary(asinh) << " (reference: " << sw::universal::to_binary(aref) << ")   " ;
	std::cout << (aref == asinh ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
}

template<size_t nbits, size_t rbits, typename bt, typename Ty>
void GenerateTestCaseCosh(Ty v) {
	Ty ref;
	sw::universal::lns<nbits, rbits, bt> a, aref, acosh;
	a = v;
	ref = std::cosh(v);
	aref = ref;
	acosh = sw::universal::cosh(a);
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " -> cosh(" << a << ") = " << std::setw(nbits) << ref << std::endl;
	std::cout << sw::universal::to_binary(a) << " -> cosh( " << a << ") = " << sw::universal::to_binary(acosh) << " (reference: " << sw::universal::to_binary(aref) << ")   ";
	std::cout << (aref == acosh ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
}

template<size_t nbits, size_t rbits, typename bt, typename Ty>
void GenerateTestCaseTanh(Ty v) {
	Ty ref;
	sw::universal::lns<nbits, rbits, bt> a, aref, atanh;
	a = v;
	ref = std::tanh(v);
	aref = ref;
	atanh = sw::universal::tanh(a);
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " -> tanh(" << a << ") = " << std::setw(nbits) << ref << std::endl;
	std::cout << sw::universal::to_binary(a) << " -> tanh( " << a << ") = " << sw::universal::to_binary(atanh) << " (reference: " << sw::universal::to_binary(aref) << ")   ";	std::cout << (aref == atanh ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
}

template<size_t nbits, size_t rbits, typename bt, typename Ty>
void GenerateTestCaseAsinh(Ty v) {
	Ty ref;
	sw::universal::lns<nbits, rbits, bt> a, aref, aasinh;
	a = v;
	ref = std::asinh(v);
	aref = ref;
	aasinh = sw::universal::asinh(a);
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " -> asinh(" << a << ") = " << std::setw(nbits) << ref << std::endl;
	std::cout << sw::universal::to_binary(a) << " -> asinh( " << a << ") = " << sw::universal::to_binary(aasinh) << " (reference: " << sw::universal::to_binary(aref) << ")   ";	std::cout << (aref == aasinh ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << (aref == aasinh ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
}

template<size_t nbits, size_t rbits, typename bt, typename Ty>
void GenerateTestCaseAcosh(Ty v) {
	Ty ref;
	sw::universal::lns<nbits, rbits, bt> a, aref, aacosh;
	a = v;
	ref = std::acosh(v);
	aref = ref;
	aacosh = sw::universal::acosh(a);
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " -> acosh(" << a << ") = " << std::setw(nbits) << ref << std::endl;
	std::cout << sw::universal::to_binary(a) << " -> acosh( " << a << ") = " << sw::universal::to_binary(aacosh) << " (reference: " << sw::universal::to_binary(aref) << ")   ";	std::cout << (aref == aacosh ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << (aref == aacosh ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
}

template<size_t nbits, size_t rbits, typename bt, typename Ty>
void GenerateTestCaseAtanh(Ty v) {
	Ty ref;
	sw::universal::lns<nbits, rbits, bt> a, aref, aatanh;
	a = v;
	ref = std::atanh(v);
	aref = ref;
	aatanh = sw::universal::atanh(a);
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " -> atanh(" << a << ") = " << std::setw(nbits) << ref << std::endl;
	std::cout << sw::universal::to_binary(a) << " -> atanh( " << a << ") = " << sw::universal::to_binary(aatanh) << " (reference: " << sw::universal::to_binary(aref) << ")   ";	std::cout << (aref == aatanh ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << (aref == aatanh ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
}


#define MANUAL_TESTING 1

const double pi = 3.14159265358979323846;

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "lns<> mathlib hyperbolic function validation";
	std::string test_tag    = "hyperbolic";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug
	GenerateTestCaseSinh<16, 1, uint16_t, double>(pi / 4.0);
	GenerateTestCaseCosh<16, 1, uint16_t, double>(pi / 4.0);
	GenerateTestCaseTanh<16, 1, uint16_t, double>(pi / 4.0);
	GenerateTestCaseAsinh<16, 1, uint16_t, double>(pi / 2.0);
	GenerateTestCaseAcosh<16, 1, uint16_t, double>(pi / 2.0);
	GenerateTestCaseAtanh<16, 1, uint16_t, double>(pi / 4.0);

	std::cout << '\n';

	// manual exhaustive test
	nrOfFailedTestCases += ReportTestResult(VerifySinh< lns<8, 2, uint8_t> >(reportTestCases), "lns<8,2>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifyCosh< lns<8, 2, uint8_t> >(reportTestCases), "lns<8,2>", "cosh");
	nrOfFailedTestCases += ReportTestResult(VerifyTanh< lns<8, 2, uint8_t> >(reportTestCases), "lns<8,2>", "tanh");
	nrOfFailedTestCases += ReportTestResult(VerifyAtanh< lns<8, 2, uint8_t> >(reportTestCases), "lns<8,2>", "atanh");
	nrOfFailedTestCases += ReportTestResult(VerifyAcosh< lns<8, 2, uint8_t> >(reportTestCases), "lns<8,2>", "acosh");
	nrOfFailedTestCases += ReportTestResult(VerifyAsinh< lns<8, 2, uint8_t> >(reportTestCases), "lns<8,2>", "asinh");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

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
