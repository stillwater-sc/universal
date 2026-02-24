// api.cpp: application programming interface tests for hexadecimal positional integer type
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

	std::string test_suite  = "hexadecimal positional integer API";
	std::string test_tag    = "hint api";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	/////////////////////////////////////////////////////////////////////////////////////
	// important behavioral traits

	{
		using TestType = positional<8, 16>;
		ReportTrivialityOfType<TestType>();
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// type tag

	{
		std::cout << "+---------    type tag\n";
		std::cout << type_tag(hi4())  << '\n';
		std::cout << type_tag(hi8())  << '\n';
		std::cout << type_tag(hi16()) << '\n';
		std::cout << type_tag(hi32()) << '\n';
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// construction and assignment

	{
		int start = nrOfFailedTestCases;
		std::cout << "+---------    construction and assignment\n";

		hi8 a(0), b(15), c(-15), d(255);
		if (int(a) != 0)   ++nrOfFailedTestCases;
		if (int(b) != 15)  ++nrOfFailedTestCases;
		if (int(c) != -15) ++nrOfFailedTestCases;
		if (int(d) != 255) ++nrOfFailedTestCases;

		a = 0xFF;
		if (int(a) != 255) ++nrOfFailedTestCases;

		a = -256;
		if (int(a) != -256) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: construction/assignment\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// basic arithmetic

	{
		int start = nrOfFailedTestCases;
		std::cout << "+---------    arithmetic operators\n";

		hi8 a(16), b(15), c;

		c = a + b;
		if (int(c) != 31)  ++nrOfFailedTestCases;
		c = a - b;
		if (int(c) != 1)   ++nrOfFailedTestCases;
		c = a * b;
		if (int(c) != 240) ++nrOfFailedTestCases;
		c = hi8(240) / b;
		if (int(c) != 16)  ++nrOfFailedTestCases;
		c = hi8(17) % b;
		if (int(c) != 2)   ++nrOfFailedTestCases;

		// negation
		c = -a;
		if (int(c) != -16) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: arithmetic\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// comparison operators

	{
		int start = nrOfFailedTestCases;
		std::cout << "+---------    comparison operators\n";

		hi8 a(10), b(255), c(10), d(-5);

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

		hi8 a(0);
		a.setdigit(0, 0xF);  // F hex = 15 decimal
		a.setdigit(1, 0xA);  // AF hex = 175 decimal
		if (int(a) != 175) ++nrOfFailedTestCases;
		if (a.digit(0) != 0xF) ++nrOfFailedTestCases;
		if (a.digit(1) != 0xA) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: digit access\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// extreme values

	{
		std::cout << "+---------    extreme values\n";
		ExtremeValues<hi4>();
		ExtremeValues<hi8>();
		ExtremeValues<hi16>();

		std::cout << positional_range(hi8()) << '\n';
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// increment/decrement

	{
		int start = nrOfFailedTestCases;
		std::cout << "+---------    increment/decrement\n";

		hi8 a(15);
		++a;
		if (int(a) != 16) ++nrOfFailedTestCases;
		a++;
		if (int(a) != 17) ++nrOfFailedTestCases;
		--a;
		if (int(a) != 16) ++nrOfFailedTestCases;
		a--;
		if (int(a) != 15) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: increment/decrement\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// digit shift (multiply/divide by 16)

	{
		int start = nrOfFailedTestCases;
		std::cout << "+---------    digit shift\n";

		hi8 a(5);
		hi8 b = a << 1;  // shift left by 1 hex digit = multiply by 16
		if (int(b) != 80) ++nrOfFailedTestCases;
		b = a << 2;  // shift left by 2 hex digits = multiply by 256
		if (int(b) != 1280) ++nrOfFailedTestCases;
		b = hi8(80) >> 1;  // shift right by 1 hex digit = divide by 16
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
