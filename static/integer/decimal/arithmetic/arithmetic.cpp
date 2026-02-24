// arithmetic.cpp: arithmetic operator tests for decimal positional integer type
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

	std::string test_suite  = "decimal positional integer arithmetic";
	std::string test_tag    = "dint arithmetic";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using Decimal = positional<8, 10>;

	// addition
	{
		int start = nrOfFailedTestCases;
		Decimal a(100), b(200), c;

		c = a + b;
		if (int(c) != 300) ++nrOfFailedTestCases;

		c = a + Decimal(-50);
		if (int(c) != 50) ++nrOfFailedTestCases;

		c = Decimal(-100) + Decimal(-200);
		if (int(c) != -300) ++nrOfFailedTestCases;

		c = a + Decimal(0);
		if (int(c) != 100) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: addition\n";
	}

	// subtraction
	{
		int start = nrOfFailedTestCases;
		Decimal a(200), b(70), c;

		c = a - b;
		if (int(c) != 130) ++nrOfFailedTestCases;

		c = b - a;
		if (int(c) != -130) ++nrOfFailedTestCases;

		c = Decimal(-50) - Decimal(-30);
		if (int(c) != -20) ++nrOfFailedTestCases;

		c = a - a;
		if (int(c) != 0) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: subtraction\n";
	}

	// multiplication
	{
		int start = nrOfFailedTestCases;
		Decimal a(12), b(11), c;

		c = a * b;
		if (int(c) != 132) ++nrOfFailedTestCases;

		c = a * Decimal(-3);
		if (int(c) != -36) ++nrOfFailedTestCases;

		c = Decimal(-4) * Decimal(-5);
		if (int(c) != 20) ++nrOfFailedTestCases;

		c = a * Decimal(0);
		if (int(c) != 0) ++nrOfFailedTestCases;

		c = a * Decimal(1);
		if (int(c) != 12) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: multiplication\n";
	}

	// division
	{
		int start = nrOfFailedTestCases;
		Decimal a(100), b(10), c;

		c = a / b;
		if (int(c) != 10) ++nrOfFailedTestCases;

		c = Decimal(17) / Decimal(3);
		if (int(c) != 5) ++nrOfFailedTestCases;  // integer division

		c = Decimal(-21) / Decimal(7);
		if (int(c) != -3) ++nrOfFailedTestCases;

		c = Decimal(-21) / Decimal(-7);
		if (int(c) != 3) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: division\n";
	}

	// modulo
	{
		int start = nrOfFailedTestCases;
		Decimal a(17), b(5), c;

		c = a % b;
		if (int(c) != 2) ++nrOfFailedTestCases;

		c = Decimal(100) % Decimal(30);
		if (int(c) != 10) ++nrOfFailedTestCases;

		c = Decimal(15) % Decimal(5);
		if (int(c) != 0) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: modulo\n";
	}

	// digit shift (multiply/divide by 10)
	{
		int start = nrOfFailedTestCases;
		Decimal a(5);

		Decimal b = a << 1;
		if (int(b) != 50) ++nrOfFailedTestCases;

		b = a << 2;
		if (int(b) != 500) ++nrOfFailedTestCases;

		Decimal c(500);
		Decimal d = c >> 2;
		if (int(d) != 5) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: digit shift\n";
	}

	// in-place operators
	{
		int start = nrOfFailedTestCases;
		Decimal a(100);

		a += Decimal(50);
		if (int(a) != 150) ++nrOfFailedTestCases;

		a -= Decimal(30);
		if (int(a) != 120) ++nrOfFailedTestCases;

		a *= Decimal(2);
		if (int(a) != 240) ++nrOfFailedTestCases;

		a /= Decimal(4);
		if (int(a) != 60) ++nrOfFailedTestCases;

		a %= Decimal(7);
		if (int(a) != 4) ++nrOfFailedTestCases;

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
