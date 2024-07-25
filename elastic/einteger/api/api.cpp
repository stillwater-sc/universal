// api.cpp: application programming interface tests for einteger
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//  SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#define EINTEGER_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/einteger/einteger.hpp>
#include <universal/verification/test_suite.hpp>

template<typename Integer>
void ArithmeticOperations(std::int64_t _a, std::int64_t _b) {
	Integer a(_a), b(_b), c(0);
	std::cout << type_tag(a) << '\n';
	c = a + b;
	std::cout << a << " + " << b << " = " << c << '\n';
	c = a - b;
	std::cout << a << " - " << b << " = " << c << '\n';
	c = a * b;
	std::cout << a << " * " << b << " = " << c << '\n';
	c = a / b;
	std::cout << a << " / " << b << " = " << c << '\n';
	c = a % b;
	std::cout << a << " % " << b << " = " << c << '\n';
	std::cout << "---\n";
}

template<typename Integer>
void AddSubPermutations() {
	Integer a, b, c;
	a = +4; b = +5; c = a + b;  std::cout << " 4 +  5  = " << int(c) << '\n';
	a = +4; b = +5; c = a - b;  std::cout << " 4 -  5  = " << int(c) << '\n';
	a = -4; b = -5; c = a + b;  std::cout << "-4 + -5  = " << int(c) << '\n';
	a = +4; b = -5; c = a - b;  std::cout << " 4 - -5  = " << int(c) << '\n';
	a = -4; b = -5; c = a - b;  std::cout << "-4 - -5  = " << int(c) << '\n';
	a = +4; b = +5; a += b;     std::cout << " 4 +=  5 : " << int(a) << '\n';
	a = +4; b = -5; a += b;     std::cout << " 4 += -5 : " << int(a) << '\n';
	a = -4; b = -5; a += b;     std::cout << "-4 += -5 : " << int(a) << '\n';
	a = +4; b = +5; a -= b;     std::cout << " 4 -=  5 : " << int(a) << '\n';
	a = +4; b = -5; a -= b;     std::cout << " 4 -= -5 : " << int(a) << '\n';
	a = -4; b = -5; a -= b;     std::cout << "-4 -= -5 : " << int(a) << '\n';
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "einteger Application Programming Interface tests";
	int nrOfFailedTestCases = 0;

	// default behavior
	std::cout << "Default einteger expands and contracts as needed\n";
	ArithmeticOperations<einteger<std::uint8_t>>(4ll, -2ll);
	// ArithmeticOperations<einteger<std::int8_t>>(4, -2); // shouldn't use signed building blocks
	ArithmeticOperations<einteger<std::uint8_t>>(256ll, -64ll);

	{
		using Integer = einteger<std::uint32_t>;
		Integer a = -20000000.0f;
		std::cout << (long long)a << " : " << to_binary(a) << " : " << a << '\n';
	}

	// TODO: remove leading zeros
	std::cout << "Bringing in large values through floating-point\n";
	{
		using ElasticInteger = einteger<std::uint8_t>;
		ElasticInteger a;
		std::cout << type_tag(a) << '\n';
		for (int i = 1; i < 40; ++i) {
			float target = 2.0f * pow(10.0f, float(i));
			a = target;
			std::cout << a << " : " << to_binary(a) << " : " << std::setw(15) << float(a) << " : reference " << target << '\n';
		}
	}
	{
		using ElasticInteger = einteger<std::uint16_t>;
		ElasticInteger a;
		std::cout << type_tag(a) << '\n';
		for (int i = 1; i < 40; ++i) {
			float target = -2.0f * pow(10.0f, float(i));
			a = target;
			std::cout << a << " : " << to_binary(a) << " : " << std::setw(15) << float(a) << " : reference " << target << '\n';
		}
	}
	{
		using ElasticInteger = einteger<std::uint32_t>;
		ElasticInteger a;
		std::cout << type_tag(a) << '\n';
		for (int i = 1; i < 40; ++i) {
			float target = 2.0f * pow(10.0f, float(i));
			a = target;
			std::cout << a << " : " << to_binary(a) << " : " << std::setw(15) << float(a) << " : reference " << target << '\n';
		}
	}

	// set bit patterns
	std::cout << "set bit patterns API\n";
	{
		using ElasticInteger = einteger<std::uint16_t>;

		ElasticInteger a;
		std::cout << type_tag(a) << '\n';

		a.setbits(0x0000);
		std::cout << to_binary(a) << " : " << a << " : " << to_hex(a) << '\n';

		a.setbits(0xAAAA);
		std::cout << to_binary(a) << " : " << a << " : " << to_hex(a) << '\n';

		a.assign(std::string("0b1'0101'1010'1010'1010"));
		std::cout << to_binary(a) << " : " << a << " : " << to_hex(a) << '\n';

		a.assign("1234567890123456789012345");
		std::cout << to_binary(a) << " : " << a << " : " << to_hex(a) << '\n';
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
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
