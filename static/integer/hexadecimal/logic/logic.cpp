// logic.cpp: logic operator tests for hexadecimal positional integer type
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

	std::string test_suite  = "hexadecimal positional integer logic operators";
	std::string test_tag    = "hint logic";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using Hex = positional<8, 16>;

	// equality and inequality
	{
		int start = nrOfFailedTestCases;
		Hex a(0xAB), b(0xAB), c(0xFF), d(0), e(-0xAB);

		if (!(a == b)) ++nrOfFailedTestCases;
		if (a != b)    ++nrOfFailedTestCases;
		if (a == c)    ++nrOfFailedTestCases;
		if (!(a != c)) ++nrOfFailedTestCases;
		if (a == e)    ++nrOfFailedTestCases;

		// zero equality
		Hex z1(0), z2(0);
		if (!(z1 == z2)) ++nrOfFailedTestCases;
		if (z1 != z2)    ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: equality/inequality\n";
	}

	// less than
	{
		int start = nrOfFailedTestCases;
		Hex a(0x10), b(0xFF), c(-0x10), d(-0xFF), z(0);

		// positive < positive
		if (!(a < b)) ++nrOfFailedTestCases;
		if (b < a)    ++nrOfFailedTestCases;

		// negative < positive
		if (!(c < a)) ++nrOfFailedTestCases;
		if (a < c)    ++nrOfFailedTestCases;

		// negative < negative
		if (!(d < c)) ++nrOfFailedTestCases;  // -0xFF < -0x10
		if (c < d)    ++nrOfFailedTestCases;

		// zero comparisons
		if (!(c < z)) ++nrOfFailedTestCases;   // -0x10 < 0
		if (z < c)    ++nrOfFailedTestCases;    // !(0 < -0x10)
		if (!(z < a)) ++nrOfFailedTestCases;   // 0 < 0x10

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: less than\n";
	}

	// greater than
	{
		int start = nrOfFailedTestCases;
		Hex a(0xFF), b(0x10), c(-0x10);

		if (!(a > b)) ++nrOfFailedTestCases;
		if (b > a)    ++nrOfFailedTestCases;
		if (!(a > c)) ++nrOfFailedTestCases;
		if (c > a)    ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: greater than\n";
	}

	// less than or equal
	{
		int start = nrOfFailedTestCases;
		Hex a(0x50), b(0x50), c(0xFF);

		if (!(a <= b)) ++nrOfFailedTestCases;
		if (!(a <= c)) ++nrOfFailedTestCases;
		if (c <= a)    ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: less than or equal\n";
	}

	// greater than or equal
	{
		int start = nrOfFailedTestCases;
		Hex a(0x50), b(0x50), c(0x10);

		if (!(a >= b)) ++nrOfFailedTestCases;
		if (!(a >= c)) ++nrOfFailedTestCases;
		if (c >= a)    ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: greater than or equal\n";
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
