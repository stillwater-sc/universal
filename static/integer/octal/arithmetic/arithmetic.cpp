// arithmetic.cpp: arithmetic operator tests for octal positional integer type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#define POSITIONAL_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/positional/positional.hpp>
#include <universal/verification/test_reporters.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "octal positional integer arithmetic";
	std::string test_tag    = "oint arithmetic";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using Octal = positional<8, 8>;

	// addition
	{
		int start = nrOfFailedTestCases;
		Octal a(10), b(20), c;

		c = a + b;
		if (int(c) != 30) ++nrOfFailedTestCases;

		c = a + Octal(-5);
		if (int(c) != 5) ++nrOfFailedTestCases;

		c = Octal(-10) + Octal(-20);
		if (int(c) != -30) ++nrOfFailedTestCases;

		c = a + Octal(0);
		if (int(c) != 10) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: addition\n";
	}

	// subtraction
	{
		int start = nrOfFailedTestCases;
		Octal a(20), b(7), c;

		c = a - b;
		if (int(c) != 13) ++nrOfFailedTestCases;

		c = b - a;
		if (int(c) != -13) ++nrOfFailedTestCases;

		c = Octal(-5) - Octal(-3);
		if (int(c) != -2) ++nrOfFailedTestCases;

		c = a - a;
		if (int(c) != 0) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: subtraction\n";
	}

	// multiplication
	{
		int start = nrOfFailedTestCases;
		Octal a(7), b(8), c;

		c = a * b;
		if (int(c) != 56) ++nrOfFailedTestCases;

		c = a * Octal(-3);
		if (int(c) != -21) ++nrOfFailedTestCases;

		c = Octal(-4) * Octal(-5);
		if (int(c) != 20) ++nrOfFailedTestCases;

		c = a * Octal(0);
		if (int(c) != 0) ++nrOfFailedTestCases;

		c = a * Octal(1);
		if (int(c) != 7) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: multiplication\n";
	}

	// division
	{
		int start = nrOfFailedTestCases;
		Octal a(63), b(7), c;

		c = a / b;
		if (int(c) != 9) ++nrOfFailedTestCases;

		c = Octal(10) / Octal(3);
		if (int(c) != 3) ++nrOfFailedTestCases;  // integer division

		c = Octal(-21) / Octal(7);
		if (int(c) != -3) ++nrOfFailedTestCases;

		c = Octal(-21) / Octal(-7);
		if (int(c) != 3) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: division\n";
	}

	// modulo
	{
		int start = nrOfFailedTestCases;
		Octal a(17), b(5), c;

		c = a % b;
		if (int(c) != 2) ++nrOfFailedTestCases;

		c = Octal(10) % Octal(3);
		if (int(c) != 1) ++nrOfFailedTestCases;

		c = Octal(15) % Octal(5);
		if (int(c) != 0) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: modulo\n";
	}

	// digit shift
	{
		int start = nrOfFailedTestCases;
		Octal a(5);

		Octal b = a << 1;
		if (int(b) != 40) ++nrOfFailedTestCases;

		b = a << 2;
		if (int(b) != 320) ++nrOfFailedTestCases;

		Octal c(320);
		Octal d = c >> 2;
		if (int(d) != 5) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: digit shift\n";
	}

	// in-place operators
	{
		int start = nrOfFailedTestCases;
		Octal a(10);

		a += Octal(5);
		if (int(a) != 15) ++nrOfFailedTestCases;

		a -= Octal(3);
		if (int(a) != 12) ++nrOfFailedTestCases;

		a *= Octal(2);
		if (int(a) != 24) ++nrOfFailedTestCases;

		a /= Octal(4);
		if (int(a) != 6) ++nrOfFailedTestCases;

		a %= Octal(4);
		if (int(a) != 2) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: in-place operators\n";
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
