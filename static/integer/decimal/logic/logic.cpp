// logic.cpp: logic operator tests for decimal positional integer type
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

	std::string test_suite  = "decimal positional integer logic operators";
	std::string test_tag    = "dint logic";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using Decimal = positional<8, 10>;

	// equality and inequality
	{
		int start = nrOfFailedTestCases;
		Decimal a(50), b(50), c(100), d(0), e(-50);

		if (!(a == b)) ++nrOfFailedTestCases;
		if (a != b)    ++nrOfFailedTestCases;
		if (a == c)    ++nrOfFailedTestCases;
		if (!(a != c)) ++nrOfFailedTestCases;
		if (a == e)    ++nrOfFailedTestCases;

		// zero equality
		Decimal z1(0), z2(0);
		if (!(z1 == z2)) ++nrOfFailedTestCases;
		if (z1 != z2)    ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: equality/inequality\n";
	}

	// less than
	{
		int start = nrOfFailedTestCases;
		Decimal a(30), b(70), c(-30), d(-70), z(0);

		// positive < positive
		if (!(a < b)) ++nrOfFailedTestCases;
		if (b < a)    ++nrOfFailedTestCases;

		// negative < positive
		if (!(c < a)) ++nrOfFailedTestCases;
		if (a < c)    ++nrOfFailedTestCases;

		// negative < negative
		if (!(d < c)) ++nrOfFailedTestCases;  // -70 < -30
		if (c < d)    ++nrOfFailedTestCases;

		// zero comparisons
		if (!(c < z)) ++nrOfFailedTestCases;   // -30 < 0
		if (z < c)    ++nrOfFailedTestCases;    // !(0 < -30)
		if (!(z < a)) ++nrOfFailedTestCases;   // 0 < 30

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: less than\n";
	}

	// greater than
	{
		int start = nrOfFailedTestCases;
		Decimal a(70), b(30), c(-30);

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
		Decimal a(50), b(50), c(100);

		if (!(a <= b)) ++nrOfFailedTestCases;
		if (!(a <= c)) ++nrOfFailedTestCases;
		if (c <= a)    ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: less than or equal\n";
	}

	// greater than or equal
	{
		int start = nrOfFailedTestCases;
		Decimal a(50), b(50), c(30);

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
