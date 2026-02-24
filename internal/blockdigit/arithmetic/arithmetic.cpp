// arithmetic.cpp: arithmetic tests for blockdigit (add, sub, mul, div, mod)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#include <universal/internal/blockdigit/blockdigit.hpp>
#include <universal/verification/test_suite.hpp>

template<unsigned ndigits, unsigned radix>
int VerifyAddition(bool reportTestCases) {
	using namespace sw::universal;
	int nrOfFailedTestCases = 0;

	struct TestCase { int a, b, expected; };
	TestCase tests[] = {
		{ 0, 0, 0 },
		{ 1, 0, 1 },
		{ 0, 1, 1 },
		{ 5, 7, 12 },
		{ 123, 456, 579 },
		{ -5, 3, -2 },
		{ 3, -5, -2 },
		{ -5, -3, -8 },
		{ 100, -100, 0 },
		{ -42, 42, 0 },
		{ 999, 1, 1000 },
	};
	for (const auto& t : tests) {
		blockdigit<ndigits, radix> a(t.a), b(t.b), c;
		c = a + b;
		int result = static_cast<int>(c);
		if (result != t.expected) {
			if (reportTestCases)
				std::cerr << "FAIL: base-" << radix << " add: " << t.a << " + " << t.b
				          << " = " << result << " expected " << t.expected << '\n';
			++nrOfFailedTestCases;
		}
	}
	return nrOfFailedTestCases;
}

template<unsigned ndigits, unsigned radix>
int VerifySubtraction(bool reportTestCases) {
	using namespace sw::universal;
	int nrOfFailedTestCases = 0;

	struct TestCase { int a, b, expected; };
	TestCase tests[] = {
		{ 0, 0, 0 },
		{ 5, 3, 2 },
		{ 3, 5, -2 },
		{ 100, 37, 63 },
		{ -5, -3, -2 },
		{ -3, -5, 2 },
		{ 10, 10, 0 },
		{ -10, -10, 0 },
	};
	for (const auto& t : tests) {
		blockdigit<ndigits, radix> a(t.a), b(t.b), c;
		c = a - b;
		int result = static_cast<int>(c);
		if (result != t.expected) {
			if (reportTestCases)
				std::cerr << "FAIL: base-" << radix << " sub: " << t.a << " - " << t.b
				          << " = " << result << " expected " << t.expected << '\n';
			++nrOfFailedTestCases;
		}
	}
	return nrOfFailedTestCases;
}

template<unsigned ndigits, unsigned radix>
int VerifyMultiplication(bool reportTestCases) {
	using namespace sw::universal;
	int nrOfFailedTestCases = 0;

	struct TestCase { int a, b, expected; };
	TestCase tests[] = {
		{ 0, 0, 0 },
		{ 1, 0, 0 },
		{ 0, 1, 0 },
		{ 1, 1, 1 },
		{ 12, 34, 408 },
		{ -3, 7, -21 },
		{ 3, -7, -21 },
		{ -3, -7, 21 },
		{ 99, 11, 1089 },
	};
	for (const auto& t : tests) {
		blockdigit<ndigits, radix> a(t.a), b(t.b), c;
		c = a * b;
		int result = static_cast<int>(c);
		if (result != t.expected) {
			if (reportTestCases)
				std::cerr << "FAIL: base-" << radix << " mul: " << t.a << " * " << t.b
				          << " = " << result << " expected " << t.expected << '\n';
			++nrOfFailedTestCases;
		}
	}
	return nrOfFailedTestCases;
}

template<unsigned ndigits, unsigned radix>
int VerifyDivision(bool reportTestCases) {
	using namespace sw::universal;
	int nrOfFailedTestCases = 0;

	struct TestCase { int a, b, expected_q, expected_r; };
	TestCase tests[] = {
		{ 100, 7, 14, 2 },
		{ 10, 3, 3, 1 },
		{ 0, 5, 0, 0 },
		{ 99, 11, 9, 0 },
		{ -100, 7, -14, -2 },
		{ 100, -7, -14, 2 },
		{ -100, -7, 14, -2 },
		{ 5, 10, 0, 5 },
		{ 1234, 56, 22, 2 },
	};
	for (const auto& t : tests) {
		blockdigit<ndigits, radix> a(t.a), b(t.b), q, r;
		q = a / b;
		r = a % b;
		int result_q = static_cast<int>(q);
		int result_r = static_cast<int>(r);
		if (result_q != t.expected_q) {
			if (reportTestCases)
				std::cerr << "FAIL: base-" << radix << " div: " << t.a << " / " << t.b
				          << " = " << result_q << " expected " << t.expected_q << '\n';
			++nrOfFailedTestCases;
		}
		if (result_r != t.expected_r) {
			if (reportTestCases)
				std::cerr << "FAIL: base-" << radix << " mod: " << t.a << " % " << t.b
				          << " = " << result_r << " expected " << t.expected_r << '\n';
			++nrOfFailedTestCases;
		}
	}
	return nrOfFailedTestCases;
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "blockdigit arithmetic";
	std::string test_tag    = "blockdigit/arithmetic";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	std::cout << "+---------    Addition: octal\n";
	nrOfFailedTestCases += VerifyAddition<8, 8>(reportTestCases);
	std::cout << "+---------    Addition: decimal\n";
	nrOfFailedTestCases += VerifyAddition<8, 10>(reportTestCases);
	std::cout << "+---------    Addition: hexadecimal\n";
	nrOfFailedTestCases += VerifyAddition<8, 16>(reportTestCases);

	std::cout << "+---------    Subtraction: octal\n";
	nrOfFailedTestCases += VerifySubtraction<8, 8>(reportTestCases);
	std::cout << "+---------    Subtraction: decimal\n";
	nrOfFailedTestCases += VerifySubtraction<8, 10>(reportTestCases);
	std::cout << "+---------    Subtraction: hexadecimal\n";
	nrOfFailedTestCases += VerifySubtraction<8, 16>(reportTestCases);

	std::cout << "+---------    Multiplication: octal\n";
	nrOfFailedTestCases += VerifyMultiplication<8, 8>(reportTestCases);
	std::cout << "+---------    Multiplication: decimal\n";
	nrOfFailedTestCases += VerifyMultiplication<8, 10>(reportTestCases);
	std::cout << "+---------    Multiplication: hexadecimal\n";
	nrOfFailedTestCases += VerifyMultiplication<8, 16>(reportTestCases);

	std::cout << "+---------    Division: octal\n";
	nrOfFailedTestCases += VerifyDivision<8, 8>(reportTestCases);
	std::cout << "+---------    Division: decimal\n";
	nrOfFailedTestCases += VerifyDivision<8, 10>(reportTestCases);
	std::cout << "+---------    Division: hexadecimal\n";
	nrOfFailedTestCases += VerifyDivision<8, 16>(reportTestCases);

	// digit shift tests
	std::cout << "+---------    Digit shift\n";
	{
		blockdecimal_t<8> a(123);
		a <<= 2;
		if (static_cast<int>(a) != 12300) {
			std::cerr << "FAIL: 123 <<= 2 = " << static_cast<int>(a) << " expected 12300\n";
			++nrOfFailedTestCases;
		}
		a = 12300;
		a >>= 2;
		if (static_cast<int>(a) != 123) {
			std::cerr << "FAIL: 12300 >>= 2 = " << static_cast<int>(a) << " expected 123\n";
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
