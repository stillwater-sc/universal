// api.cpp: application programming interface tests for blockdigit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#include <universal/internal/blockdigit/blockdigit.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "blockdigit API";
	std::string test_tag    = "blockdigit";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	/////////////////////////////////////////////////////////////////////
	// triviality checks for all three radixes
	{
		std::cout << "+---------    Triviality checks\n";
		ReportTrivialityOfType< blockdigit<8, 8> >();
		ReportTrivialityOfType< blockdigit<8, 10> >();
		ReportTrivialityOfType< blockdigit<8, 16> >();
	}

	/////////////////////////////////////////////////////////////////////
	// type_tag checks
	{
		std::cout << "+---------    type_tag checks\n";
		std::cout << type_tag(blockoctal<8>()) << '\n';
		std::cout << type_tag(blockdecimal_t<8>()) << '\n';
		std::cout << type_tag(blockhexadecimal<8>()) << '\n';
		std::cout << type_tag(blockdigit<8, 3>()) << '\n';
	}

	/////////////////////////////////////////////////////////////////////
	// construction and assignment
	{
		std::cout << "+---------    Construction and assignment\n";

		// decimal
		blockdecimal_t<8> d;
		d = 12345;
		std::cout << "blockdecimal<8>(12345) = " << d << " binary: " << to_binary(d) << '\n';
		if (static_cast<long long>(d) != 12345) {
			std::cerr << "FAIL: blockdecimal<8>(12345) conversion\n";
			++nrOfFailedTestCases;
		}

		// negative
		d = -42;
		std::cout << "blockdecimal<8>(-42) = " << d << " binary: " << to_binary(d) << '\n';
		if (static_cast<long long>(d) != -42) {
			std::cerr << "FAIL: blockdecimal<8>(-42) conversion\n";
			++nrOfFailedTestCases;
		}

		// octal
		blockoctal<8> o;
		o = 255;
		std::cout << "blockoctal<8>(255) = " << o << " binary: " << to_binary(o) << '\n';
		if (static_cast<long long>(o) != 255) {
			std::cerr << "FAIL: blockoctal<8>(255) conversion\n";
			++nrOfFailedTestCases;
		}

		// hexadecimal
		blockhexadecimal<8> h;
		h = 255;
		std::cout << "blockhexadecimal<8>(255) = " << h << " binary: " << to_binary(h) << '\n';
		if (static_cast<long long>(h) != 255) {
			std::cerr << "FAIL: blockhexadecimal<8>(255) conversion\n";
			++nrOfFailedTestCases;
		}

		// zero
		d = 0;
		if (!d.iszero()) {
			std::cerr << "FAIL: blockdecimal zero check\n";
			++nrOfFailedTestCases;
		}
	}

	/////////////////////////////////////////////////////////////////////
	// basic arithmetic spot checks
	{
		std::cout << "+---------    Basic arithmetic spot checks\n";
		blockdecimal_t<8> a, b, c;

		a = 123; b = 456;
		c = a + b;
		std::cout << a << " + " << b << " = " << c << '\n';
		if (static_cast<int>(c) != 579) {
			std::cerr << "FAIL: 123 + 456 = " << static_cast<int>(c) << '\n';
			++nrOfFailedTestCases;
		}

		a = 100; b = 37;
		c = a - b;
		std::cout << a << " - " << b << " = " << c << '\n';
		if (static_cast<int>(c) != 63) {
			std::cerr << "FAIL: 100 - 37 = " << static_cast<int>(c) << '\n';
			++nrOfFailedTestCases;
		}

		a = 12; b = 34;
		c = a * b;
		std::cout << a << " * " << b << " = " << c << '\n';
		if (static_cast<int>(c) != 408) {
			std::cerr << "FAIL: 12 * 34 = " << static_cast<int>(c) << '\n';
			++nrOfFailedTestCases;
		}

		a = 100; b = 7;
		c = a / b;
		std::cout << a << " / " << b << " = " << c << '\n';
		if (static_cast<int>(c) != 14) {
			std::cerr << "FAIL: 100 / 7 = " << static_cast<int>(c) << '\n';
			++nrOfFailedTestCases;
		}

		c = a % b;
		std::cout << a << " % " << b << " = " << c << '\n';
		if (static_cast<int>(c) != 2) {
			std::cerr << "FAIL: 100 % 7 = " << static_cast<int>(c) << '\n';
			++nrOfFailedTestCases;
		}
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
