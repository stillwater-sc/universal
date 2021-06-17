// hyperbolic.cpp: test suite runner for hyperbolic functions (sinh/cosh/tanh/atanh/acosh/asinh)
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// use default library configuration
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/cfloat_math_test_suite.hpp>

// generate specific test case that you can trace with the trace conditions in cfloat.h
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCaseSinh(Ty v) {
	Ty ref;
	sw::universal::cfloat<nbits, es, uint8_t> a, aref, asinh;
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

template<size_t nbits, size_t es, typename Ty>
void GenerateTestCaseCosh(Ty v) {
	Ty ref;
	sw::universal::cfloat<nbits, es> a, aref, acosh;
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

template<size_t nbits, size_t es, typename Ty>
void GenerateTestCaseTanh(Ty v) {
	Ty ref;
	sw::universal::cfloat<nbits, es> a, aref, atanh;
	a = v;
	ref = std::tanh(v);
	aref = ref;
	atanh = sw::universal::tanh(a);
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " -> tanh(" << a << ") = " << std::setw(nbits) << ref << std::endl;
	std::cout << sw::universal::to_binary(a) << " -> tanh( " << a << ") = " << sw::universal::to_binary(atanh) << " (reference: " << sw::universal::to_binary(aref) << ")   ";	std::cout << (aref == atanh ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
}

template<size_t nbits, size_t es, typename Ty>
void GenerateTestCaseAsinh(Ty v) {
	Ty ref;
	sw::universal::cfloat<nbits, es> a, aref, aasinh;
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

template<size_t nbits, size_t es, typename Ty>
void GenerateTestCaseAcosh(Ty v) {
	Ty ref;
	sw::universal::cfloat<nbits, es> a, aref, aacosh;
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

template<size_t nbits, size_t es, typename Ty>
void GenerateTestCaseAtanh(Ty v) {
	Ty ref;
	sw::universal::cfloat<nbits, es> a, aref, aatanh;
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
#define STRESS_TESTING 0

const double pi = 3.14159265358979323846;

int main()
try {
	using namespace std;
	using namespace sw::universal;

	//bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	std::string tag = "Addition failed: ";

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug
	GenerateTestCaseSinh<16, 1, double>(pi / 4.0);
	GenerateTestCaseCosh<16, 1, double>(pi / 4.0);
	GenerateTestCaseTanh<16, 1, double>(pi / 4.0);
	GenerateTestCaseAsinh<16, 1, double>(pi / 2.0);
	GenerateTestCaseAcosh<16, 1, double>(pi / 2.0);
	GenerateTestCaseAtanh<16, 1, double>(pi / 4.0);

	std::cout << endl;

	// manual exhaustive test
	nrOfFailedTestCases += ReportTestResult(VerifySinh< cfloat<8, 2, uint8_t> >(true), "cfloat<8,2>", "sinh");
	nrOfFailedTestCases += ReportTestResult(VerifyCosh< cfloat<8, 2, uint8_t> >(true), "cfloat<8,2>", "cosh");
	nrOfFailedTestCases += ReportTestResult(VerifyTanh< cfloat<8, 2, uint8_t> >(true), "cfloat<8,2>", "tanh");
	nrOfFailedTestCases += ReportTestResult(VerifyAtanh< cfloat<8, 2, uint8_t> >(true), "cfloat<8,2>", "atanh");
	nrOfFailedTestCases += ReportTestResult(VerifyAcosh< cfloat<8, 2, uint8_t> >(true), "cfloat<8,2>", "acosh");
	nrOfFailedTestCases += ReportTestResult(VerifyAsinh< cfloat<8, 2, uint8_t> >(true), "cfloat<8,2>", "asinh");
#else

	cout << "cfloat hyperbolic sine/cosine/tangent function validation" << endl;


#if STRESS_TESTING
	// nbits=64 requires long double compiler support
	// nrOfFailedTestCases += ReportTestResult(VerifyThroughRandoms<64, 2>(tag, bReportIndividualTestCases, OPCODE_SQRT, 1000), "cfloat<64,2>", "sinh");

	
#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::cfloat_arithmetic_exception& err) {
	std::cerr << "Uncaught cfloat arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::cfloat_quire_exception& err) {
	std::cerr << "Uncaught cfloat quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::cfloat_internal_exception& err) {
	std::cerr << "Uncaught cfloat internal exception: " << err.what() << std::endl;
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
