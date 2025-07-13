// api.cpp : class API tests for adaptive precision decimal rational number system type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <string>
#include <universal/number/erational/erational.hpp>
#include <universal/verification/test_suite.hpp>

/*
   The goal of the rational number system is to provide a flexible
   and easy to use rational arithmetic type.
*/

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
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "erational class API ";
	std::string test_tag    = "API";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	using Rational = sw::universal::erational;

	Rational a, b, c, d;
	a = -1;
	b = 8;
	c = a / b;
	std::cout << a << " / " << b << " = " << c << '\n';
	a = 3;
	d = a / b;
	std::cout << a << " / " << b << " = " << d << '\n';
	a = c;
	b = d;
	c = a + b;
	std::cout << a << " + " << b << " = " << c << '\n';
	c = a - b;
	std::cout << a << " - " << b << " = " << c << '\n';
	d = a * b;
	std::cout << a << " * " << b << " = " << d << '\n';
	a = -a;
	b = -b;
	c = a + b;
	std::cout << a << " + " << b << " = " << c << '\n';
	c = a - b;
	std::cout << a << " - " << b << " = " << c << '\n';
	d = a * b;
	std::cout << a << " * " << b << " = " << d << '\n';

	{
		edecimal a, b, remainder;
		a = 3; b = 9;
		while (a % b > 0) {
			remainder = a % b;
			a = b;
			b = remainder;
		}
		std::cout << "gcd of (3, 9) = " << remainder << '\n';
	}

	a = 1;
	b = 10;
	c = a / b;
	std::cout << "c = " << c << " : " << double(c) << " : " << to_binary(double(c)) << '\n';
	b = 7;
	c = a / b;
	std::cout << "c = " << c << " : " << double(c) << " : " << to_binary(double(c)) << '\n';


	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else // !MANUAL_TESTING

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
catch (const sw::universal::erational_arithmetic_exception& err) {
	std::cerr << "Uncaught arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::erational_internal_exception& err) {
	std::cerr << "Uncaught internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
