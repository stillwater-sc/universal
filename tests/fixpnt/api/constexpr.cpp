// constexpr.cpp: compile time tests for fixed-point constexpr
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// Configure the fixpnt template environment
// first: enable general or specialized fixed-point configurations
#define FIXPNT_FAST_SPECIALIZATION
// second: enable/disable fixpnt arithmetic exceptions
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 1

// minimum set of include files to reflect source code dependencies
#include <universal/number/fixpnt/fixpnt_impl.hpp>
// fixed-point type manipulators such as pretty printers
#include <universal/number/fixpnt/manipulators.hpp>
#include <universal/number/fixpnt/mathlib.hpp>

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

constexpr double pi = 3.14159265358979323846;

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	if (argc > 0) { std::cout << argv[0] << std::endl; }

	int nrOfFailedTestCases = 0;

	std::cout << "fixed-point constexpr tests\n";
	
	{
		fixpnt<8, 4> a(pi);
		std::cout << a << '\n';
	}

// TODO: make fixpnt constexpr
#ifdef CONSTEXPRESSION
	{
		// decorated constructors
		{
			constexpr fixpnt<8, 4> a(1l);  // signed long
			std::cout << a << '\n';
		}
		{
			constexpr fixpnt<8, 4> a(1ul);  // unsigned long
			std::cout << a << '\n';
		}
		{
			constexpr fixpnt<8, 4> a(1.0f);  // float
			std::cout << a << '\n';
		}
		{
			constexpr fixpnt<8, 4> a(1.0);   // double
			std::cout << a << '\n';
		}
		{
			constexpr fixpnt<8, 4> a(1.0l);  // long double
			std::cout << a << '\n';
		}
	}
	{
		// assignment operators
		{
			constexpr fixpnt<8, 4> a = 1l;  // signed long
			std::cout << a << '\n';
		}
		{
			constexpr fixpnt<8, 4> a = 1ul;  // unsigned long
			std::cout << a << '\n';
		}
		{
			constexpr fixpnt<8, 4> a = 1.0f;  // float
			std::cout << a << '\n';
		}
		{
			constexpr fixpnt<8, 4> a = 1.0;   // double
			std::cout << a << '\n';
		}
		{
			constexpr fixpnt<8, 4> a = 1.0l;  // long double
			std::cout << a << '\n';
		}
	}
#endif

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
