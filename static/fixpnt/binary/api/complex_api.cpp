// complex_api.cpp: api to use fixpnt type in complex arithmetic operations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include<universal/utility/directives.hpp>
#include <iostream>
#include <bitset>
// According to the C++ ISO spec, paragraph 26.2/2:
//    The effect of instantiating the template complex for any type other than float, double or long double is unspecified.
#include <complex>
// Configure the fixpnt template environment
// first: enable general or specialized fixed-point configurations
#define FIXPNT_FAST_SPECIALIZATION
// second: enable fixpnt arithmetic exceptions
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/verification/fixpnt_test_suite.hpp>

namespace sw::universal::complex_literals {

	std::complex<fixpnt<8, 4>> operator""_i(long double _Val)
	{	// return imaginary _Val
		return (std::complex<fixpnt<8, 4>>(0.0, static_cast<fixpnt<8, 4>>(double(_Val))));
	}

	std::complex<fixpnt<8, 4>> operator""_i(unsigned long long _Val)
	{	// return imaginary _Val
		return (std::complex<fixpnt<8, 4>>(0.0, static_cast<fixpnt<8, 4>>(_Val)));
	}

} // namespace sw::universal::complex_literals


// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
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
	using namespace std; // needed to get imaginary literals
	using namespace sw::universal;

	std::string test_suite  = "fixpnt complex arithmetic operations ";
	std::string test_tag    = "complex arithmetic";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

#if defined(__GNUG__)
	{
		// reference using native double precision floating point type
		using namespace std::complex_literals;
		std::cout << std::fixed << std::setprecision(1);

		std::complex<double> z1 = 1i * 1i;     // imaginary unit squared
		std::cout << "i * i = " << z1 << '\n';

//		std::complex<double> z2 = std::pow(1.0i, 2.0); // imaginary unit squared
//		std::cout << "pow(i, 2) = " << z2 << '\n';

//		double PI = std::acos(-1);
//		std::complex<double> z3 = std::exp(1i * PI); // Euler's formula
//		std::cout << "exp(i * pi) = " << z3 << '\n';

		std::complex<double> z4{1.0, +2.0}, z5{1.0, -2.0}; // conjugates
		std::cout << "(1+2i)*(1-2i) = " << z4 * z5 << '\n';
	}

#undef GPP_FIX
#ifdef GPP_FIX
	// for some reason the g++ doesn't compile this section as it is 
	// casting the constants differently than other compilers.
	// TODO: fix the code below to make it compile with g++
	{
		using namespace sw::universal::complex_literals;
		using Real = sw::universal::fixpnt<8, 4>;
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
#endif // !GPP_FIX
/*
	error: conversion from '__complex__ int' to non - scalar type 'std::complex<sw::universal::fixpnt<8, 4> >' requested
		std::complex<Real> z1 = 1i * 1i;     // imaginary unit squared
		                        ~~~^~~~
	error : conversion from '__complex__ double' to non - scalar type 'std::complex<sw::universal::fixpnt<8, 4> >' requested
        std::complex<Real> z4 = 1.0 + 2i, z5 = 1.0 - 2i; // conjugates
		                        ~~~~^~~~
	error : conversion from '__complex__ double' to non - scalar type 'std::complex<sw::universal::fixpnt<8, 4> >' requested
        std::complex<Real> z4 = 1.0 + 2i, z5 = 1.0 - 2i; // conjugates
		                                       ~~~~^~~~
*/
    // furthermore, the pow and exp functions don't match the correct complex<double> arguments in g++

#else  // !__GNUG__

	{
		// reference using native double precision floating point type
		using namespace std::complex_literals;
		std::cout << std::fixed << std::setprecision(1);

		std::complex<double> z1 = 1i * 1i;     // imaginary unit squared
		std::cout << "i * i = " << z1 << '\n';

		// the pow and exp functions don't match in g++
		// no idea how to fix the code below to make it compile with g++
		std::complex<double> z2 = std::pow(1.0i, 2.0); // imaginary unit squared
		std::cout << "pow(i, 2) = " << z2 << '\n';

		double PI = std::acos(-1);
		std::complex<double> z3 = std::exp(1i * PI); // Euler's formula
		std::cout << "exp(i * pi) = " << z3 << '\n';

		std::complex<double> z4 = 1. + 2i, z5 = 1. - 2i; // conjugates
		std::cout << "(1+2i)*(1-2i) = " << z4 * z5 << '\n';
	}

	{
		// all the literals are marshalled through the std library double native type for complex literals
		// defining them for fixpnt is syntactically unattractive as the "i" literal is reserved for native types,
		// so our literal type would need to be non-standard anyway as "_i"
		using namespace sw::universal::complex_literals;
		using Real = sw::universal::fixpnt<8, 4>;
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

	{
		using FixedPoint = sw::universal::fixpnt<4, 3, sw::universal::Saturate, uint8_t>;
		FixedPoint one = 1;
		FixedPoint minus_one = -1;
		FixedPoint fp = 1.0f;
		std::complex<FixedPoint> z1{ 1.0f, 1.0f }, z2{ minus_one, minus_one }, z3;
		std::cout << "z1 : " << z1 << '\n';
		std::cout << "z2 : " << z2 << '\n';
		z3 = std::complex<FixedPoint>(0.0f, 0.0f);
		std::cout << "z3 : " << z3 << '\n';
		fp = copysign(one, minus_one);
		std::cout << "copysign(0.875, -1) : " << fp << '\n';
		z3 = copysign(z1.real(), z2.imag());
		std::cout << "z3 : " << z3 << '\n';
	}

	{
		using FixedPoint = sw::universal::fixpnt<8, 4>;
		FixedPoint fp = 1.0f;
		if (isinf(fp)) std::cout << "fp is infinite\n"; else std::cout << "fp is not infinite: " << fp << '\n';
		if (isnan(fp)) std::cout << "fp is NaN\n"; else std::cout << "fp is not NaN: " << fp << '\n';
	}


	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1

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
