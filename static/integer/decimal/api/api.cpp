// api.cpp: application programming interface tests for decimal positional integer type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#define POSITIONAL_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/positional/positional.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "decimal positional integer API";
	std::string test_tag    = "dint api";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	/////////////////////////////////////////////////////////////////////////////////////
	// important behavioral traits

	{
		using TestType = positional<8, 10>;
		ReportTrivialityOfType<TestType>();
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// type tag

	{
		std::cout << "+---------    type tag\n";
		std::cout << type_tag(di4())  << '\n';
		std::cout << type_tag(di8())  << '\n';
		std::cout << type_tag(di16()) << '\n';
		std::cout << type_tag(di32()) << '\n';
		std::cout << type_tag(di64()) << '\n';
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// construction and assignment

	{
		int start = nrOfFailedTestCases;
		std::cout << "+---------    construction and assignment\n";

		di8 a(0), b(9), c(-9), d(12345678);
		if (int(a) != 0)        ++nrOfFailedTestCases;
		if (int(b) != 9)        ++nrOfFailedTestCases;
		if (int(c) != -9)       ++nrOfFailedTestCases;
		if (int(d) != 12345678) ++nrOfFailedTestCases;

		a = 99;
		if (int(a) != 99) ++nrOfFailedTestCases;

		a = -99;
		if (int(a) != -99) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: construction/assignment\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// basic arithmetic

	{
		int start = nrOfFailedTestCases;
		std::cout << "+---------    arithmetic operators\n";

		di8 a(100), b(37), c;

		c = a + b;
		if (int(c) != 137) ++nrOfFailedTestCases;
		c = a - b;
		if (int(c) != 63) ++nrOfFailedTestCases;
		c = a * b;
		if (int(c) != 3700) ++nrOfFailedTestCases;
		c = a / b;
		if (int(c) != 2) ++nrOfFailedTestCases;
		c = a % b;
		if (int(c) != 26) ++nrOfFailedTestCases;

		// negation
		c = -a;
		if (int(c) != -100) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: arithmetic\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// comparison operators

	{
		int start = nrOfFailedTestCases;
		std::cout << "+---------    comparison operators\n";

		di8 a(50), b(100), c(50), d(-30);

		if (!(a == c)) ++nrOfFailedTestCases;
		if (!(a != b)) ++nrOfFailedTestCases;
		if (!(a < b))  ++nrOfFailedTestCases;
		if (!(b > a))  ++nrOfFailedTestCases;
		if (!(a <= c)) ++nrOfFailedTestCases;
		if (!(a >= c)) ++nrOfFailedTestCases;
		if (!(d < a))  ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: comparison\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// digit-level access

	{
		int start = nrOfFailedTestCases;
		std::cout << "+---------    digit-level access\n";

		di8 a(0);
		a.setdigit(0, 5);  // ones = 5
		a.setdigit(1, 3);  // tens = 3 -> 35
		if (int(a) != 35) ++nrOfFailedTestCases;
		if (a.digit(0) != 5) ++nrOfFailedTestCases;
		if (a.digit(1) != 3) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: digit access\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// extreme values

	{
		std::cout << "+---------    extreme values\n";
		ExtremeValues<di4>();
		ExtremeValues<di8>();
		ExtremeValues<di16>();

		std::cout << positional_range(di8()) << '\n';
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// increment/decrement

	{
		int start = nrOfFailedTestCases;
		std::cout << "+---------    increment/decrement\n";

		di8 a(99);
		++a;
		if (int(a) != 100) ++nrOfFailedTestCases;
		a++;
		if (int(a) != 101) ++nrOfFailedTestCases;
		--a;
		if (int(a) != 100) ++nrOfFailedTestCases;
		a--;
		if (int(a) != 99) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: increment/decrement\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// digit shift (multiply/divide by radix)

	{
		int start = nrOfFailedTestCases;
		std::cout << "+---------    digit shift\n";

		di8 a(5);
		di8 b = a << 1;  // shift left by 1 decimal digit = multiply by 10
		if (int(b) != 50) ++nrOfFailedTestCases;
		b = a << 2;  // shift left by 2 decimal digits = multiply by 100
		if (int(b) != 500) ++nrOfFailedTestCases;
		b = di8(500) >> 2;  // shift right by 2 decimal digits = divide by 100
		if (int(b) != 5) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: digit shift\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// mixed native-type arithmetic with literals

	{
		int start = nrOfFailedTestCases;
		std::cout << "+---------    mixed-type arithmetic\n";

		di8 a(10), c;
		int x = -3;

		c = a + x;
		if (int(c) != 7) ++nrOfFailedTestCases;
		c = a - x;
		if (int(c) != 13) ++nrOfFailedTestCases;
		c = a * x;
		if (int(c) != -30) ++nrOfFailedTestCases;

		c = x + a;
		if (int(c) != 7) ++nrOfFailedTestCases;
		c = x - a;
		if (int(c) != -13) ++nrOfFailedTestCases;
		c = x * a;
		if (int(c) != -30) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: mixed-type arithmetic\n";
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
