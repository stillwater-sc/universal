// complex_add.cpp: functional tests for posit complex addition
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// Configure the posit template environment
// first: enable general or specialized posit configurations
//#define POSIT_FAST_SPECIALIZATION
// second: enable/disable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 0
// third: enable tracing 
// when you define POSIT_VERBOSE_OUTPUT executing an ADD the code will print intermediate results
//#define POSIT_VERBOSE_OUTPUT
#define POSIT_TRACE_ADD

// minimum set of include files to reflect source code dependencies
#include <universal/posit/posit.hpp>
#include <universal/posit/numeric_limits.hpp>
#include <universal/posit/specializations.hpp>
#include <universal/posit/manipulators.hpp>
#include <universal/posit/math_functions.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/verification/posit_test_suite.hpp>
#include <universal/verification/posit_test_randoms.hpp>

// generate specific test case that you can trace with the trace conditions in posit.h
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCase(Ty a, Ty b) {
	Ty ref;
	sw::universal::posit<nbits, es> pa, pb, pref, psum;
	pa = a;
	pb = b;
	ref = a + b;
	pref = ref;
	psum = pa + pb;
	std::streamsize oldPrecision = std::cout.precision();
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " + " << std::setw(nbits) << b << " = " << std::setw(nbits) << ref << std::endl;
	std::cout << pa.get() << " + " << pb.get() << " = " << psum.get() << " (reference: " << pref.get() << ")   " ;
	std::cout << (pref == psum ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::dec << std::setprecision(oldPrecision);
}

#define FLOAT_TABLE_WIDTH 10

template<size_t nbits, size_t es>
void ReportBinaryArithmeticError(const std::string& test_case, const std::string& op, 
	const std::complex<sw::universal::posit<nbits, es>>& lhs, 
	const std::complex<sw::universal::posit<nbits, es>>& rhs, 
	const std::complex<sw::universal::posit<nbits, es>>& ref, 
	const std::complex<sw::universal::posit<nbits, es>>& result) {
	std::cerr << test_case << " "
		<< std::setprecision(20)
		<< std::setw(FLOAT_TABLE_WIDTH) << lhs
		<< " " << op << " "
		<< std::setw(FLOAT_TABLE_WIDTH) << rhs
		<< " != "
		<< std::setw(FLOAT_TABLE_WIDTH) << ref << " instead it yielded "
		<< std::setw(FLOAT_TABLE_WIDTH) << result
//		<< " " << ref.get() << " vs " << result.get()
		<< std::setprecision(5)
		<< std::endl;
}

// enumerate all addition cases for a posit configuration
template<size_t nbits, size_t es>
int ValidateComplexAddition(const std::string& tag, bool bReportIndividualTestCases) {
	using namespace std;
	using namespace sw::universal;
	const size_t NR_POSITS = (size_t(1) << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> ar, ai, br, bi;
	complex<posit<nbits, es>> a, b, result, ref;

	complex<double> da, db, dc;
	for (size_t i = 0; i < NR_POSITS; ++i) {
		ar.set_raw_bits(i);
		for (size_t j = 0; j < NR_POSITS; ++j) {
			ai.set_raw_bits(j);
			a = complex<posit<nbits, es>>(ar, ai);
			da = complex<double>(double(ar), double(ai));

			// generate all the right sides
			for (size_t k = 0; k < NR_POSITS; ++k) {
				br.set_raw_bits(k);
				for (size_t l = 0; l < NR_POSITS; ++l) {
					bi.set_raw_bits(l);
					b = complex<posit<nbits, es>>(br, bi);
					db = complex<double>(double(br), double(bi));

					result = a + b;
					dc = da + db;
					ref = complex<posit<nbits, es>>(dc.real(), dc.imag());

					if (result.real() != ref.real() || result.imag() != ref.imag()) {
						nrOfFailedTests++;
						if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "+", a, b, ref, result);
					}
					else {
						//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "+", a, b, ref, result);
					}
				}
			}
		}
	}

	return nrOfFailedTests;
}

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "posit complex addition failed: ";

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

	{
		using Real = double;
		std::complex<Real> z4 = 1. + 2i, z5 = 1. - 2i; // conjugates
		std::cout << "(1+2i)*(1-2i) = " << z4 * z5 << '\n';
	}

	{
		using Real = posit<16,1>;
#if defined(__GNUG__)
/* TODO: this doesn't compile under g++
 error: conversion from ‘__complex__ double’ to non-scalar type ‘std::complex<sw::universal::posit<16, 1> >’ requested
   std::complex<Real> z4 = 1. + 2i, z5 = 1. - 2i; // conjugates
                           ~~~^~~~
 error: conversion from ‘__complex__ double’ to non-scalar type ‘std::complex<sw::universal::posit<16, 1> >’ requested
   std::complex<Real> z4 = 1. + 2i, z5 = 1. - 2i; // conjugates
                                         ~~~^~~~
*/
		std::complex<Real> z4{1.0, +2.0}, z5{1.0, -2.0}; // conjugates
		cout << "(1+2i)*(1-2i) = " << z4 * z5 << '\n';
#else
		std::complex<Real> z4 = 1. + 2i, z5 = 1. - 2i; // conjugates
		cout << "(1+2i)*(1-2i) = " << z4 * z5 << '\n';
#endif

		auto z0 = std::complex<Real>(1.0f, 1.0f);
		cout << z0 << endl;
		auto z1 = std::complex<Real>(1.0, 0.0);
		cout << z1 << endl;
	}

	nrOfFailedTestCases += ReportTestResult(ValidateComplexAddition<2, 0>(tag, bReportIndividualTestCases), "posit<2,0>", "addition");
	// manual exhaustive test
	nrOfFailedTestCases += ReportTestResult(ValidateComplexAddition<5, 0>("Manual Testing", true), "complex<posit<5,0>>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateComplexAddition<5, 1>("Manual Testing", true), "complex<posit<5,1>>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateComplexAddition<5, 2>("Manual Testing", true), "complex<posit<5,2>>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateComplexAddition<5, 3>("Manual Testing", true), "complex<posit<5,3>>", "addition");

//	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<16, 1>(tag, true, OPCODE_ADD, 1000), "posit<16,1>", "addition");
//	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<64, 2>(tag, true, OPCODE_ADD, 1000), "posit<64,2>", "addition");

#else

	cout << "Posit complex addition validation" << endl;

	nrOfFailedTestCases += ReportTestResult(ValidateComplexAddition<2, 0>(tag, bReportIndividualTestCases), "posit<2,0>", "addition");

	nrOfFailedTestCases += ReportTestResult(ValidateComplexAddition<3, 0>(tag, bReportIndividualTestCases), "posit<3,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateComplexAddition<3, 1>(tag, bReportIndividualTestCases), "posit<3,1>", "addition");

	nrOfFailedTestCases += ReportTestResult(ValidateComplexAddition<4, 0>(tag, bReportIndividualTestCases), "posit<4,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateComplexAddition<4, 1>(tag, bReportIndividualTestCases), "posit<4,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateComplexAddition<4, 2>(tag, bReportIndividualTestCases), "posit<4,2>", "addition");

	nrOfFailedTestCases += ReportTestResult(ValidateComplexAddition<5, 0>(tag, bReportIndividualTestCases), "posit<5,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateComplexAddition<5, 1>(tag, bReportIndividualTestCases), "posit<5,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateComplexAddition<5, 2>(tag, bReportIndividualTestCases), "posit<5,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateComplexAddition<5, 3>(tag, bReportIndividualTestCases), "posit<5,3>", "addition");

	nrOfFailedTestCases += ReportTestResult(ValidateComplexAddition<6, 0>(tag, bReportIndividualTestCases), "posit<6,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateComplexAddition<6, 1>(tag, bReportIndividualTestCases), "posit<6,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateComplexAddition<6, 2>(tag, bReportIndividualTestCases), "posit<6,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateComplexAddition<6, 3>(tag, bReportIndividualTestCases), "posit<6,3>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateComplexAddition<6, 4>(tag, bReportIndividualTestCases), "posit<6,4>", "addition");

	nrOfFailedTestCases += ReportTestResult(ValidateComplexAddition<7, 0>(tag, bReportIndividualTestCases), "posit<7,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateComplexAddition<7, 1>(tag, bReportIndividualTestCases), "posit<7,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateComplexAddition<7, 2>(tag, bReportIndividualTestCases), "posit<7,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateComplexAddition<7, 3>(tag, bReportIndividualTestCases), "posit<7,3>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateComplexAddition<7, 4>(tag, bReportIndividualTestCases), "posit<7,4>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateComplexAddition<7, 5>(tag, bReportIndividualTestCases), "posit<7,5>", "addition");

	nrOfFailedTestCases += ReportTestResult(ValidateComplexAddition<8, 0>(tag, bReportIndividualTestCases), "posit<8,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateComplexAddition<8, 1>(tag, bReportIndividualTestCases), "posit<8,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateComplexAddition<8, 2>(tag, bReportIndividualTestCases), "posit<8,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateComplexAddition<8, 3>(tag, bReportIndividualTestCases), "posit<8,3>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateComplexAddition<8, 4>(tag, bReportIndividualTestCases), "posit<8,4>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateComplexAddition<8, 5>(tag, bReportIndividualTestCases), "posit<8,5>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateComplexAddition<8, 6>(tag, bReportIndividualTestCases), "posit<8,6>", "addition");

	nrOfFailedTestCases += ReportTestResult(ValidateBinaryOperatorThroughRandoms<16, 1>(tag, bReportIndividualTestCases, OPCODE_ADD, 1000), "posit<16,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateBinaryOperatorThroughRandoms<24, 1>(tag, bReportIndividualTestCases, OPCODE_ADD, 1000), "posit<24,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateBinaryOperatorThroughRandoms<32, 1>(tag, bReportIndividualTestCases, OPCODE_ADD, 1000), "posit<32,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateBinaryOperatorThroughRandoms<32, 2>(tag, bReportIndividualTestCases, OPCODE_ADD, 1000), "posit<32,2>", "addition");

#if STRESS_TESTING
	// nbits=48 also shows failures
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<48, 2>(tag, bReportIndividualTestCases, OPCODE_ADD, 1000), "posit<48,2>", "addition");

	// nbits=64 requires long double compiler support
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<64, 2>(tag, bReportIndividualTestCases, OPCODE_ADD, 1000), "posit<64,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<64, 3>(tag, bReportIndividualTestCases, OPCODE_ADD, 1000), "posit<64,3>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<64, 4>(tag, bReportIndividualTestCases, OPCODE_ADD, 1000), "posit<64,4>", "addition");


	nrOfFailedTestCases += ReportTestResult(ValidateComplexAddition<10, 1>(tag, bReportIndividualTestCases), "posit<10,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateComplexAddition<12, 1>(tag, bReportIndividualTestCases), "posit<12,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateComplexAddition<14, 1>(tag, bReportIndividualTestCases), "posit<14,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateComplexAddition<16, 1>(tag, bReportIndividualTestCases), "posit<16,1>", "addition");
#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
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
