// addition.cpp: test suite runner for posit complex addition
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// Configure the posit template environment
// first: enable general or specialized posit configurations
//#define POSIT_FAST_SPECIALIZATION
// second: enable/disable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 0
// third: enable tracing 
// when you define ALGORITHM_VERBOSE_OUTPUT executing an ADD the code will print intermediate results
//#define ALGORITHM_VERBOSE_OUTPUT
#define ALGORITHM_TRACE_ADD
#include <universal/number/posit1/posit1.hpp>
#include <universal/verification/posit_test_suite.hpp>
#include <universal/verification/posit_test_suite_randoms.hpp>
#include <universal/verification/posit_test_suite_mathlib.hpp>

#define FLOAT_TABLE_WIDTH 10

template<unsigned nbits, unsigned es>
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
template<typename TestType>
int VerifyComplexAddition(bool reportTestCases) {
	using namespace sw::universal;
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned es = TestType::es;
	const unsigned NR_POSITS = (unsigned(1) << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> ar, ai, br, bi;
	std::complex< posit<nbits, es> > a, b, result, ref;

	std::complex<double> da, db, dc;
	for (unsigned i = 0; i < NR_POSITS; ++i) {
		ar.setbits(i);
		for (unsigned j = 0; j < NR_POSITS; ++j) {
			ai.setbits(j);
			a = std::complex< posit<nbits, es> >(ar, ai);
			da = std::complex<double>(double(ar), double(ai));

			// generate all the right sides
			for (unsigned k = 0; k < NR_POSITS; ++k) {
				br.setbits(k);
				for (unsigned l = 0; l < NR_POSITS; ++l) {
					bi.setbits(l);
					b = std::complex< posit<nbits, es> >(br, bi);
					db = std::complex<double>(double(br), double(bi));

					result = a + b;
					dc = da + db;
					ref = std::complex< posit<nbits, es> >(dc.real(), dc.imag());

					if (result.real() != ref.real() || result.imag() != ref.imag()) {
						nrOfFailedTests++;
						if (reportTestCases)	ReportBinaryArithmeticError("FAIL", "+", a, b, ref, result);
					}
					else {
						//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "+", a, b, ref, result);
					}
				}
			}
		}
	}

	return nrOfFailedTests;
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

int main()
try {
	using namespace std;  // needed to get the imaginary literals
	using namespace sw::universal;

	std::string test_suite  = "posit complex addition verification";
	std::string test_tag    = "complex addition";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

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

	nrOfFailedTestCases += ReportTestResult(VerifyComplexAddition<posit<2, 0>>(reportTestCases), "posit<2,0>", "addition");
	// manual exhaustive test
	nrOfFailedTestCases += ReportTestResult(VerifyComplexAddition<posit<5, 0>>(reportTestCases), "complex<posit<5,0>>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexAddition<posit<5, 1>>(reportTestCases), "complex<posit<5,1>>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexAddition<posit<5, 2>>(reportTestCases), "complex<posit<5,2>>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexAddition<posit<5, 3>>(reportTestCases), "complex<posit<5,3>>", "addition");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyComplexAddition<posit<2, 0>>(reportTestCases), "posit<2,0>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyComplexAddition<posit<3, 0>>(reportTestCases), "posit<3,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexAddition<posit<3, 1>>(reportTestCases), "posit<3,1>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyComplexAddition<posit<4, 0>>(reportTestCases), "posit<4,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexAddition<posit<4, 1>>(reportTestCases), "posit<4,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexAddition<posit<4, 2>>(reportTestCases), "posit<4,2>", "addition");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyComplexAddition<posit<5, 0>>(reportTestCases), "posit<5,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexAddition<posit<5, 1>>(reportTestCases), "posit<5,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexAddition<posit<5, 2>>(reportTestCases), "posit<5,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexAddition<posit<5, 3>>(reportTestCases), "posit<5,3>", "addition");
#endif

#if REGRESSION_LEVEL_3
	//	nrOfFailedTestCases += ReportTestResult(VerifyComplexAddition<posit<6, 0>>(reportTestCases), "posit<6,0>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyComplexAddition<posit<6, 1>>(reportTestCases), "posit<6,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexAddition<posit<6, 2>>(reportTestCases), "posit<6,2>", "addition");
	//	nrOfFailedTestCases += ReportTestResult(VerifyComplexAddition<posit<6, 3>>(reportTestCases), "posit<6,3>", "addition");
	//	nrOfFailedTestCases += ReportTestResult(VerifyComplexAddition<posit<6, 4>>(reportTestCases), "posit<6,4>", "addition");

	//	nrOfFailedTestCases += ReportTestResult(VerifyComplexAddition<posit<7, 0>>(reportTestCases), "posit<7,0>", "addition");
	//	nrOfFailedTestCases += ReportTestResult(VerifyComplexAddition<posit<7, 1>>(reportTestCases), "posit<7,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexAddition<posit<7, 2>>(reportTestCases), "posit<7,2>", "addition");
	//	nrOfFailedTestCases += ReportTestResult(VerifyComplexAddition<posit<7, 3>>(reportTestCases), "posit<7,3>", "addition");
	//	nrOfFailedTestCases += ReportTestResult(VerifyComplexAddition<posit<7, 4>>(reportTestCases), "posit<7,4>", "addition");
	//	nrOfFailedTestCases += ReportTestResult(VerifyComplexAddition<posit<7, 5>>(reportTestCases), "posit<7,5>", "addition");
#endif

#if REGRESSION_LEVEL_4
//	nrOfFailedTestCases += ReportTestResult(VerifyComplexAddition<posit<8, 0>>(reportTestCases), "posit<8,0>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyComplexAddition<posit<8, 1>>(reportTestCases), "posit<8,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyComplexAddition<posit<8, 2>>(reportTestCases), "posit<8,2>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyComplexAddition<posit<8, 3>>(reportTestCases), "posit<8,3>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyComplexAddition<posit<8, 4>>(reportTestCases), "posit<8,4>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyComplexAddition<posit<8, 5>>(reportTestCases), "posit<8,5>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyComplexAddition<posit<8, 6>>(reportTestCases), "posit<8,6>", "addition");

#endif // REGRESSION_LEVEL_4

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
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
