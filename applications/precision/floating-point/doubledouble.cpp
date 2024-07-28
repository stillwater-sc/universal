// doubledouble.cpp: experiments with double-double floating-point arithmetic
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <limits>
#include <utility>
#include <numbers>    // high-precision numbers

#include <universal/benchmark/performance_runner.hpp>
#include <universal/verification/test_suite.hpp>

#include <universal/number/dd/dd.hpp>         // the double-double format
//#include <universal/number/qd/qd.hpp>         // the quad-double format
#include <universal/number/cfloat/cfloat.hpp> // the classic floating-point reference


/*
Definition of FAITHFUL arithmetic
   For a t-digit number a and b, and op element {+,-,*,/}, let c = a op b exactly.
   Suppose x and y are consecutive t-digit floating-point numbers with the same 
   sign as c such at |x| <= |c| < |y|. Then the floating-point arithmetic is
   called faithful if fl(a op b) = x whenever c = x and fl(a op b) is either x or y
   whenever c != x.

 */

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "experiment with double-double floating-point arithmetic";
	std::string test_tag    = "double-double";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	std::streamsize precision = std::cout.precision();
	
	{
		using doubledouble = dd;

		doubledouble a, b{}, c{};
		a = 0.5;
		b = 2.0;
		c = a * b;
		ReportBinaryOperation(a, "*", b, c);
	}

	{
		dd a;
		a = 1.0f;
		std::cout << to_binary(a) << " : " << a << '\n';
	}

	// important behavioral traits
	{
		using TestType = dd;
		ReportTrivialityOfType<TestType>();
	}

	// default behavior
	std::cout << "+---------    Default doubledouble has subnormals, but no supernormals\n";
	{
		using Real = dd;

		Real a(1.0f), b(0.5f);
		ArithmeticOperators(a, b);
	}

	// report on the dynamic range of some standard configurations
	std::cout << "+---------    Dynamic ranges of standard double configurations   --------+\n";
	{
		dd a; // uninitialized

		a.maxpos();
		std::cout << "maxpos  bfloat16 : " << to_binary(a) << " : " << a << '\n';
		a.setbits(0x0080);  // positive min normal
		std::cout << "minnorm bfloat16 : " << to_binary(a) << " : " << a << '\n';
		a.minpos();
		std::cout << "minpos  bfloat16 : " << to_binary(a) << " : " << a << '\n';
		a.zero();
		std::cout << "zero             : " << to_binary(a) << " : " << a << '\n';
		a.minneg();
		std::cout << "minneg  bfloat16 : " << to_binary(a) << " : " << a << '\n';
		a.setbits(0x8080);  // negative min normal
		std::cout << "minnegnorm       : " << to_binary(a) << " : " << a << '\n';
		a.maxneg();
		std::cout << "maxneg  bfloat16 : " << to_binary(a) << " : " << a << '\n';

		std::cout << "---\n";
	}

	std::cout << std::setprecision(precision);
	std::cout << std::endl;
	
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
