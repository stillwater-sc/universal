// api.cpp: application programming interface tests for octal positional integer type
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

	std::string test_suite  = "octal positional integer API";
	std::string test_tag    = "oint api";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	/////////////////////////////////////////////////////////////////////////////////////
	// important behavioral traits

	{
		using TestType = positional<8, 8>;
		ReportTrivialityOfType<TestType>();
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// type tag

	{
		std::cout << "+---------    type tag\n";
		std::cout << type_tag(oi4())  << '\n';
		std::cout << type_tag(oi8())  << '\n';
		std::cout << type_tag(oi16()) << '\n';
		std::cout << type_tag(oi32()) << '\n';
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// construction and assignment

	{
		int start = nrOfFailedTestCases;
		std::cout << "+---------    construction and assignment\n";

		oi8 a(0), b(7), c(-7), d(100);
		if (int(a) != 0)   ++nrOfFailedTestCases;
		if (int(b) != 7)   ++nrOfFailedTestCases;
		if (int(c) != -7)  ++nrOfFailedTestCases;
		if (int(d) != 100) ++nrOfFailedTestCases;

		a = 42;
		if (int(a) != 42) ++nrOfFailedTestCases;

		a = -42;
		if (int(a) != -42) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: construction/assignment\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// basic arithmetic

	{
		int start = nrOfFailedTestCases;
		std::cout << "+---------    arithmetic operators\n";

		oi8 a(10), b(3), c;

		c = a + b;
		if (int(c) != 13)  ++nrOfFailedTestCases;
		c = a - b;
		if (int(c) != 7)   ++nrOfFailedTestCases;
		c = a * b;
		if (int(c) != 30)  ++nrOfFailedTestCases;
		c = a / b;
		if (int(c) != 3)   ++nrOfFailedTestCases;
		c = a % b;
		if (int(c) != 1)   ++nrOfFailedTestCases;

		// negation
		c = -a;
		if (int(c) != -10) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: arithmetic\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// comparison operators

	{
		int start = nrOfFailedTestCases;
		std::cout << "+---------    comparison operators\n";

		oi8 a(5), b(10), c(5), d(-3);

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

		oi8 a(0);
		a.setdigit(0, 7);  // 7 in octal
		a.setdigit(1, 3);  // 37 octal = 31 decimal
		if (int(a) != 31) ++nrOfFailedTestCases;
		if (a.digit(0) != 7) ++nrOfFailedTestCases;
		if (a.digit(1) != 3) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: digit access\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// extreme values

	{
		std::cout << "+---------    extreme values\n";
		ExtremeValues<oi4>();
		ExtremeValues<oi8>();
		ExtremeValues<oi16>();

		std::cout << positional_range(oi8()) << '\n';
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// increment/decrement

	{
		int start = nrOfFailedTestCases;
		std::cout << "+---------    increment/decrement\n";

		oi8 a(5);
		++a;
		if (int(a) != 6) ++nrOfFailedTestCases;
		a++;
		if (int(a) != 7) ++nrOfFailedTestCases;
		--a;
		if (int(a) != 6) ++nrOfFailedTestCases;
		a--;
		if (int(a) != 5) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: increment/decrement\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// digit shift

	{
		int start = nrOfFailedTestCases;
		std::cout << "+---------    digit shift\n";

		oi8 a(5);
		oi8 b = a << 1;  // shift left by 1 octal digit = multiply by 8
		if (int(b) != 40) ++nrOfFailedTestCases;
		b = a << 2;  // shift left by 2 octal digits = multiply by 64
		if (int(b) != 320) ++nrOfFailedTestCases;
		b = oi8(40) >> 1;  // shift right by 1 octal digit = divide by 8
		if (int(b) != 5) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: digit shift\n";
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
