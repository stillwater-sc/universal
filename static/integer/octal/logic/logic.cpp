// logic.cpp: logic operator tests for octal positional integer type
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

	std::string test_suite  = "octal positional integer logic operators";
	std::string test_tag    = "oint logic";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using Octal = positional<8, 8>;

	// equality and inequality
	{
		int start = nrOfFailedTestCases;
		Octal a(5), b(5), c(10), d(0), e(-5);

		if (!(a == b)) ++nrOfFailedTestCases;
		if (a != b)    ++nrOfFailedTestCases;
		if (a == c)    ++nrOfFailedTestCases;
		if (!(a != c)) ++nrOfFailedTestCases;
		if (a == e)    ++nrOfFailedTestCases;

		// zero equality
		Octal z1(0), z2(0);
		if (!(z1 == z2)) ++nrOfFailedTestCases;
		if (z1 != z2)    ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: equality/inequality\n";
	}

	// less than
	{
		int start = nrOfFailedTestCases;
		Octal a(3), b(7), c(-3), d(-7), z(0);

		// positive < positive
		if (!(a < b)) ++nrOfFailedTestCases;
		if (b < a)    ++nrOfFailedTestCases;

		// negative < positive
		if (!(c < a)) ++nrOfFailedTestCases;
		if (a < c)    ++nrOfFailedTestCases;

		// negative < negative
		if (!(d < c)) ++nrOfFailedTestCases;  // -7 < -3
		if (c < d)    ++nrOfFailedTestCases;

		// zero comparisons
		if (!(c < z)) ++nrOfFailedTestCases;   // -3 < 0
		if (z < c)    ++nrOfFailedTestCases;    // !(0 < -3)
		if (!(z < a)) ++nrOfFailedTestCases;   // 0 < 3

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: less than\n";
	}

	// greater than
	{
		int start = nrOfFailedTestCases;
		Octal a(7), b(3), c(-3);

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
		Octal a(5), b(5), c(10);

		if (!(a <= b)) ++nrOfFailedTestCases;
		if (!(a <= c)) ++nrOfFailedTestCases;
		if (c <= a)    ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: less than or equal\n";
	}

	// greater than or equal
	{
		int start = nrOfFailedTestCases;
		Octal a(5), b(5), c(3);

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
