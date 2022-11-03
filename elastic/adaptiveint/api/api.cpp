// api.cpp: application programming interface tests for adaptiveint
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// minimum set of include files to reflect source code dependencies
// Configure the cfloat template environment
// second: enable/disable arithmetic exceptions
#define ADAPTIVEINT_THROW_ARITHMETIC_EXCEPTION 0
// third: enable trace conversion
#define TRACE_CONVERSION 0
#include <universal/number/adaptiveint/adaptiveint.hpp>
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

	std::string test_suite = "adaptiveint Application Programming Interface tests";
	int nrOfFailedTestCases = 0;

	// default behavior
	std::cout << "Default adaptiveint expands and contracts as needed\n";
	ArithmeticOperations<adaptiveint<std::uint8_t>>(4ll, -2ll);
	// ArithmeticOperations<adaptiveint<std::int8_t>>(4, -2); // shouldn't use signed building blocks
	ArithmeticOperations<adaptiveint<std::uint8_t>>(256ll, -64ll);

	{
		using Integer = adaptiveint<std::uint32_t>;
		Integer a = -20000000.0f;
		std::cout << (long long)a << " : " << to_binary(a) << " : " << a << '\n';
	}

	// TODO: remove leading zeros
	std::cout << "Bringing in large values through floating-point\n";
	{
		using Integer = adaptiveint<std::uint8_t>;
		for (int i = 1; i < 40; ++i) {
			float target = 2.0f * pow(10.0f, float(i));
			Integer a = target;
			std::cout << a << " : " << to_binary(a) << " : " << std::setw(15) << float(a) << " : reference " << target << '\n';
		}
	}
	{
		using Integer = adaptiveint<std::uint8_t>;
		for (int i = 1; i < 40; ++i) {
			float target = -2.0f * pow(10.0f, float(i));
			Integer a = target;
			std::cout << a << " : " << to_binary(a) << " : " << std::setw(15) << float(a) << " : reference " << target << '\n';
		}
	}
	// TODO: conversions using 4 byte blocks fails
	{
		using Integer = adaptiveint<std::uint32_t>;
		for (int i = 1; i < 40; ++i) {
			float target = 2.0f * pow(10.0f, float(i));
			Integer a = target;
			std::cout << a << " : " << to_binary(a) << " : " << std::setw(15) << float(a) << " : reference " << target << '\n';
		}
	}

	// set bit patterns
	std::cout << "set bit patterns API\n";
	{
		using Integer = adaptiveint<std::uint32_t>;

		Integer a;
		std::cout << type_tag(a) << '\n';

		a.setbits(0x0000);
		std::cout << to_binary(a) << " : " << a << '\n';

		a.setbits(0xAAAA);
		std::cout << to_binary(a) << " : " << a << '\n';

		a.assign(std::string("0b1'0101'1010'1010'10"));
		std::cout << to_binary(a) << " : " << a << '\n';

		a.assign("1234567890123456789012345");
		std::cout << a << '\n';
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
