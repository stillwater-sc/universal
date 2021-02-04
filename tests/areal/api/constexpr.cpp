// constexpr.cpp: compile time tests for areal constexpr
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// Configure the areal template environment
// first: enable general or specialized fixed-point configurations
#define AREAL_FAST_SPECIALIZATION
// second: enable/disable areal arithmetic exceptions
#define AREAL_THROW_ARITHMETIC_EXCEPTION 1

// minimum set of include files to reflect source code dependencies
#include <universal/number/areal/areal.hpp>
// fixed-point type manipulators such as pretty printers
#include <universal/number/areal/manipulators.hpp>
#include <universal/number/areal/math_functions.hpp>

// conditional compile flags
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

constexpr double pi = 3.14159265358979323846;

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;

	cout << "AREAL constexpr tests" << endl;
	

	{
		// decorated constructors
		{
			constexpr areal<8, 4> a(1l);  // signed long
			cout << a << endl;
		}
		{
			constexpr areal<8, 4> a(1ul);  // unsigned long
			cout << a << endl;
		}
#if BIT_CAST_SUPPORT
		{
			CONSTEXPRESSION areal<8, 4> a(1.0f);  // float
			cout << a << endl;
		}
		{
			CONSTEXPRESSION areal<8, 4> a(1.0);   // double
			cout << a << endl;
		}
		{
			CONSTEXPRESSION areal<8, 4> a(1.0l);  // long double
			cout << a << endl;
		}
#endif // BIT_CAST_SUPPORT
	}
	{
		// assignment operators
		{
			constexpr areal<8, 4> a = 1l;  // signed long
			cout << a << endl;
		}
		{
			constexpr areal<8, 4> a = 1ul;  // unsigned long
			cout << a << endl;
		}
#if BIT_CAST_SUPPORT
		{
			CONSTEXPRESSION areal<8, 4> a = 1.0f;  // float
			cout << a << endl;
		}
		{
			CONSTEXPRESSION areal<8, 4> a = 1.0;   // double
			cout << a << endl;
		}
		{
			CONSTEXPRESSION areal<8, 4> a = 1.0l;  // long double
			cout << a << endl;
		}
#endif
	}

	if (nrOfFailedTestCases > 0) {
		cout << "FAIL" << endl;
	}
	else {
		cout << "PASS" << endl;
	}
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::areal_arithmetic_exception& err) {
	std::cerr << "Uncaught areal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::areal_internal_exception& err) {
	std::cerr << "Uncaught areal internal exception: " << err.what() << std::endl;
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
