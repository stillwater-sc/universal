// complex_mod_addition.cpp: functional tests for arbitrary configuration fixed-point complex addition
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <bitset>
#include <complex>

// Configure the fixpnt template environment
// first: enable general or specialized fixed-point configurations
#define FIXPNT_FAST_SPECIALIZATION
// second: enable/disable fixpnt arithmetic exceptions
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 1

// minimum set of include files to reflect source code dependencies
#include <universal/fixpnt/fixed_point.hpp>
// fixed-point type manipulators such as pretty printers
#include <universal/fixpnt/fixpnt_manipulators.hpp>
#include <universal/fixpnt/math_functions.hpp>
#include "../utils/fixpnt_test_suite.hpp"

// generate specific test case that you can trace with the trace conditions in fixed_point.hpp
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t rbits, typename Ty>
void GenerateTestCase(Ty _a, Ty _b) {
	Ty ref;
	sw::unum::fixpnt<nbits, rbits> a, b, cref, result;
	a = _a;
	b = _b;
	result = a + b;
	ref = _a + _b;
	cref = ref;
	std::streamsize oldPrecision = std::cout.precision();
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << _a << " + " << std::setw(nbits) << _b << " = " << std::setw(nbits) << ref << std::endl;
	std::cout << a << " + " << b << " = " << result << " (reference: " << cref << ")   " ;
	std::cout << (cref == result ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::dec << std::setprecision(oldPrecision);
}

namespace sw {
namespace unum {
namespace complex_literals {
	std::complex<sw::unum::fixpnt<8, 4>> operator""_i(long double _Val)
	{	// return imaginary _Val
		return (std::complex<sw::unum::fixpnt<8, 4>>(0.0, static_cast<sw::unum::fixpnt<8, 4>>(_Val)));
	}

	std::complex<sw::unum::fixpnt<8, 4>> operator""_i(unsigned long long _Val)
	{	// return imaginary _Val
		return (std::complex<sw::unum::fixpnt<8, 4>>(0.0, static_cast<sw::unum::fixpnt<8, 4>>(_Val)));
	}
} // namespace complex_literals
} // namespace unum
} // namespace sw


// conditional compile flags
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	//bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "modular addition failed: ";

#if MANUAL_TESTING

	{
		using namespace std::complex_literals;
		std::cout << std::fixed << std::setprecision(1);

		std::complex<double> z1 = 1i * 1i;     // imaginary unit squared
		std::cout << "i * i = " << z1 << '\n';

#if defined(__GNUG__)
		// the pow and exp functions don't match in g++
		// no idea how to fix the code below to make it compile with g++
#else
		std::complex<double> z2 = std::pow(1.0i, 2.0); // imaginary unit squared
		std::cout << "pow(i, 2) = " << z2 << '\n';

		double PI = std::acos(-1);
		std::complex<double> z3 = std::exp(1i * PI); // Euler's formula
		std::cout << "exp(i * pi) = " << z3 << '\n';
#endif

		std::complex<double> z4 = 1. + 2i, z5 = 1. - 2i; // conjugates
		std::cout << "(1+2i)*(1-2i) = " << z4 * z5 << '\n';
	}

#if defined(__GNUG__)
	// for some reason the g++ doesn't compile this section as it is casting the constants differently
	// than other compilers.
			// no idea how to fix the code below to make it compile with g++
/*
		using namespace sw::unum::complex_literals;
		using Real = sw::unum::fixpnt<8, 4>;
		std::cout << std::fixed << std::setprecision(1);

		std::complex<Real> z1 = 1i * 1i;     // imaginary unit squared
		std::cout << "i * i = " << z1 << '\n';

		std::complex<double> z2 = std::pow(1.0i, 2.0); // imaginary unit squared
		std::cout << "pow(i, 2) = " << z2 << '\n';

		double PI = std::acos(-1);
		std::complex<Real> z3 = std::exp(1.0i * PI); // Euler's formula
		std::cout << "exp(i * pi) = " << z3 << '\n';

		std::complex<Real> z4 = 1.0 + 2i, z5 = 1.0 - 2i; // conjugates
		std::cout << "(1+2i)*(1-2i) = " << z4 * z5 << '\n';

	error: conversion from '__complex__ int' to non - scalar type 'std::complex<sw::unum::fixpnt<8, 4> >' requested
		std::complex<Real> z1 = 1i * 1i;     // imaginary unit squared
		                        ~~~^~~~
	error : conversion from '__complex__ double' to non - scalar type 'std::complex<sw::unum::fixpnt<8, 4> >' requested
        std::complex<Real> z4 = 1.0 + 2i, z5 = 1.0 - 2i; // conjugates
		                        ~~~~^~~~
	error : conversion from '__complex__ double' to non - scalar type 'std::complex<sw::unum::fixpnt<8, 4> >' requested
        std::complex<Real> z4 = 1.0 + 2i, z5 = 1.0 - 2i; // conjugates
		                                       ~~~~^~~~
*/
    // furthermore, the pow and exp functions don't match the correct complex<double> arguments in g++

#else

	{
		// all the literals are marshalled through the std library double native type for complex literals
		// defining them for fixpnt is syntactically unattractive as the "i" literal is reserved for native types,
		// so our literal type would need to be non-standard anyway as "_i"
		using namespace sw::unum::complex_literals;
		using Real = sw::unum::fixpnt<8, 4>;
		std::cout << std::fixed << std::setprecision(1);

		std::complex<Real> z1 = 1i * 1i;     // imaginary unit squared
		std::cout << "i * i = " << z1 << '\n';

		std::complex<double> z2 = std::pow(1.0i, 2.0); // imaginary unit squared
		std::cout << "pow(i, 2) = " << z2 << '\n';

		double PI = std::acos(-1);
		std::complex<Real> z3 = std::exp(1.0i * PI); // Euler's formula
		std::cout << "exp(i * pi) = " << z3 << '\n';

		std::complex<Real> z4 = 1.0 + 2i, z5 = 1.0 - 2i; // conjugates
		std::cout << "(1+2i)*(1-2i) = " << z4 * z5 << '\n';
	}
#endif

	nrOfFailedTestCases += ReportTestResult(VerifyComplexAddition<4, 1, Modulo, uint8_t>("Manual Testing", true), "fixpnt<4,1,Modulo,uint8_t>", "addition");


#if STRESS_TESTING
	// manual exhaustive test
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<4, 0, Modulo, uint8_t>("Manual Testing", true), "fixpnt<4,0,Modulo,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<4, 1, Modulo, uint8_t>("Manual Testing", true), "fixpnt<4,1,Modulo,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<4, 2, Modulo, uint8_t>("Manual Testing", true), "fixpnt<4,2,Modulo,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<4, 3, Modulo, uint8_t>("Manual Testing", true), "fixpnt<4,3,Modulo,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<4, 4, Modulo, uint8_t>("Manual Testing", true), "fixpnt<4,4,Modulo,uint8_t>", "addition");
#endif

#else

	cout << "Fixed-point modular addition validation" << endl;

	nrOfFailedTestCases += ReportTestResult(VerifyAddition<4, 0, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<4,0,Modulo,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<4, 1, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<4,1,Modulo,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<4, 2, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<4,2,Modulo,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<4, 3, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<4,3,Modulo,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<4, 4, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<4,4,Modulo,uint8_t>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyAddition<8, 0, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<8,0,Modulo,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<8, 1, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<8,1,Modulo,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<8, 2, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<8,2,Modulo,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<8, 3, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<8,3,Modulo,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<8, 4, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<8,4,Modulo,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<8, 5, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<8,5,Modulo,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<8, 6, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<8,6,Modulo,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<8, 7, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<8,7,Modulo,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<8, 8, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<8,8,Modulo,uint8_t>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyAddition<10, 3, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<10,3,Modulo,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<10, 5, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<10,5,Modulo,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<10, 7, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<10,7,Modulo,uint8_t>", "addition");

#if STRESS_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyAddition<11, 3, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<11,3,Modulo,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<11, 5, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<11,5,Modulo,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<11, 7, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<11,7,Modulo,uint8_t>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyAddition<12, 0, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<12,0,Modulo,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<12, 4, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<12,4,Modulo,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<12, 8, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<12,8,Modulo,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<12, 12, Modulo, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<12,12,Modulo,uint8_t>", "addition");

#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::unum::fixpnt_arithmetic_exception& err) {
	std::cerr << "Uncaught fixpnt arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::unum::fixpnt_internal_exception& err) {
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
