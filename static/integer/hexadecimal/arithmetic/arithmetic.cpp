// arithmetic.cpp: arithmetic operator tests for hexadecimal positional integer type
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

	std::string test_suite  = "hexadecimal positional integer arithmetic";
	std::string test_tag    = "hint arithmetic";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using Hex = positional<8, 16>;

	// addition
	{
		int start = nrOfFailedTestCases;
		Hex a(0x10), b(0x20), c;

		c = a + b;
		if (int(c) != 0x30) ++nrOfFailedTestCases;

		c = a + Hex(-5);
		if (int(c) != 11) ++nrOfFailedTestCases;

		c = Hex(-0x10) + Hex(-0x20);
		if (int(c) != -0x30) ++nrOfFailedTestCases;

		c = a + Hex(0);
		if (int(c) != 0x10) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: addition\n";
	}

	// subtraction
	{
		int start = nrOfFailedTestCases;
		Hex a(0xFF), b(0x10), c;

		c = a - b;
		if (int(c) != 0xEF) ++nrOfFailedTestCases;

		c = b - a;
		if (int(c) != -0xEF) ++nrOfFailedTestCases;

		c = Hex(-5) - Hex(-3);
		if (int(c) != -2) ++nrOfFailedTestCases;

		c = a - a;
		if (int(c) != 0) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: subtraction\n";
	}

	// multiplication
	{
		int start = nrOfFailedTestCases;
		Hex a(0x10), b(0x10), c;

		c = a * b;
		if (int(c) != 256) ++nrOfFailedTestCases;

		c = Hex(0xF) * Hex(-3);
		if (int(c) != -45) ++nrOfFailedTestCases;

		c = Hex(-4) * Hex(-5);
		if (int(c) != 20) ++nrOfFailedTestCases;

		c = a * Hex(0);
		if (int(c) != 0) ++nrOfFailedTestCases;

		c = a * Hex(1);
		if (int(c) != 0x10) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: multiplication\n";
	}

	// division
	{
		int start = nrOfFailedTestCases;
		Hex a(0xFF), b(0xF), c;

		c = a / b;
		if (int(c) != 17) ++nrOfFailedTestCases;

		c = Hex(0x10) / Hex(3);
		if (int(c) != 5) ++nrOfFailedTestCases;  // 16/3 = 5

		c = Hex(-21) / Hex(7);
		if (int(c) != -3) ++nrOfFailedTestCases;

		c = Hex(-21) / Hex(-7);
		if (int(c) != 3) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: division\n";
	}

	// modulo
	{
		int start = nrOfFailedTestCases;
		Hex a(0xFF), b(0x10), c;

		c = a % b;
		if (int(c) != 0xF) ++nrOfFailedTestCases;

		c = Hex(17) % Hex(5);
		if (int(c) != 2) ++nrOfFailedTestCases;

		c = Hex(0x100) % Hex(0x10);
		if (int(c) != 0) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: modulo\n";
	}

	// digit shift (multiply/divide by 16)
	{
		int start = nrOfFailedTestCases;
		Hex a(5);

		Hex b = a << 1;
		if (int(b) != 80) ++nrOfFailedTestCases;

		b = a << 2;
		if (int(b) != 1280) ++nrOfFailedTestCases;

		Hex c(1280);
		Hex d = c >> 2;
		if (int(d) != 5) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: digit shift\n";
	}

	// in-place operators
	{
		int start = nrOfFailedTestCases;
		Hex a(0x10);

		a += Hex(0x05);
		if (int(a) != 0x15) ++nrOfFailedTestCases;

		a -= Hex(0x03);
		if (int(a) != 0x12) ++nrOfFailedTestCases;

		a *= Hex(2);
		if (int(a) != 0x24) ++nrOfFailedTestCases;

		a /= Hex(4);
		if (int(a) != 9) ++nrOfFailedTestCases;

		a %= Hex(4);
		if (int(a) != 1) ++nrOfFailedTestCases;

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
