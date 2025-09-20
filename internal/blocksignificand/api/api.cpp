//  api.cpp : test suite runner for blocksignificand application programming interface tests
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <iostream>

#include <universal/native/integers.hpp>
#include <universal/internal/blocksignificand/blocksignificand.hpp>
#include <universal/verification/test_suite.hpp>

/*

A blocksignificand is a 2's complement binary encoding with a radix point that is aligned
with the hidden bit of the fraction encoding in a floating-point representation.

The main goal of the blocksignificand abstraction is to support arbitrary floating-point 
number systems with a high-quality, high-performance arithmetic engine.

The expensive part in these abstractions is the need to receive, expand, and align
bit strings, so special attention is given to fast implementations using copies.
This is acceptable, and leads to cleaner code, for small representations. However,
for very large representations these copies become prohibitive, and for those situations
the blocksignificand is not a good solution.

*/

template<typename blocksignificand>
void PrintRoundingDirection(const blocksignificand& a, unsigned targetLsb) {
	std::cout << to_binary(a) << " target lsb = " << targetLsb << " ->rounding mode is " << (a.roundingDirection(targetLsb) ? "up" : "down") << '\n';
}

void TestRounding()
{
	using namespace sw::universal;

	std::cout << "\n---------------------- \nblocksignificand ad-hoc rounding test\n";

	// ad-hoc rounding test 
	// 0001'0000 
	//      | lsb target at 3
	blocksignificand<8, uint8_t> a;
	constexpr int radix = 5;
	a.setradix(radix);
	unsigned lsbTarget = 3;
	a.setbits(0x2F);	// 0b001.01111  up
	std::cout << to_binary(a) << " : " << a << '\n';
	unsigned bit = lsbTarget;
	std::cout << "lsb target = " << lsbTarget << '\n';
	std::cout << " = a[" << bit << "] = " << (a.test(bit) ? '1' : '0') << '\n'; --bit;
	std::cout << " = a[" << bit << "] = " << (a.test(bit) ? '1' : '0') << '\n'; --bit;
	std::cout << " = a[" << bit << "] = " << (a.test(bit) ? '1' : '0') << '\n'; --bit;
	std::cout << " = a[" << bit << "] = " << (a.test(bit) ? '1' : '0') << '\n';
	PrintRoundingDirection(a, lsbTarget);
	a.setbits(0x27); 	// 0b001.00111  up
	PrintRoundingDirection(a, lsbTarget);
	a.setbits(0x23);	// 0b001.00011  down
	PrintRoundingDirection(a, lsbTarget);
	a.setbits(0x24); 	// 0b00100100  up: tie, round to even, which is down in this case
	PrintRoundingDirection(a, lsbTarget);
	a.setbits(0x2C); 	// 0b001.01100  up: tie, round to even, which is up in this case
	PrintRoundingDirection(a, lsbTarget);
}

void AdditionSetup()
{
	using namespace sw::universal;

	std::cout <<  "\n---------------------- \nblocksignificand addition setup\n";

	{
		blocksignificand<7, uint8_t> a, b, c; // give it three extra bits beyond the radix
		constexpr int radix = 4;
		a.setbits(0x11); // 0b001.0001 = 1.0625 in 7-bit blocksignificand form
		a.setradix(radix);
		b.setbits(0x11);
		b.setradix(radix);
		std::cout << to_binary(a) << " : " << a << '\n';
		std::cout << to_binary(b) << " : " << b << '\n';
		c.add(a, b);
		c.setradix(radix); // adds are aligned and radix of input is the same on output
		std::cout << to_binary(c) << " : " << c << '\n';
		uint64_t fractionBits = c.fraction_ull();
		std::cout << to_binary(fractionBits, true, radix) << '\n';
	}

	{
		// a cfloat<8,2> has 5 fraction bits
		// a 00h.fffff format is thus 8 bits
		// By design, the 00h.fffff format contains all the valid values
		// for addition and subtraction.
		blocksignificand<8, uint8_t> a, b, c; // give it three extra bits beyond the radix
		constexpr int radix = 5;
		a.setbits(0x21); // 0b001.0'0001 = 1.0 in 8-bit blocksignificand form
		a.setradix(radix);
		b.setbits(0x21);
		b.setradix(radix);
		std::cout << to_binary(a) << " : " << a << '\n';
		std::cout << to_binary(b) << " : " << b << '\n';
		c.add(a, b);
		c.setradix(radix);
		std::cout << to_binary(c) << " : " << c << '\n';
		uint64_t fractionBits = c.fraction_ull();
		std::cout << to_binary(fractionBits, true, radix) << '\n';
	}

	{
		blocksignificand<12, uint8_t> a, b, c; // give it a couple of extra bits beyond the radix
		constexpr int radix = 8;
		a.setbits(0x100);  // 0b0001.0000'0000 = 1.0 in 12-bit/radix@8
		a.setradix(radix);
		b.setbits(0x200);  // 0b0010.0000'0000 = 2.0 in 12-bit/radix@8
		b.setradix(radix);
		b.twosComplement(); // -> -2.0
		std::cout << to_binary(a) << " : " << a << '\n';
		std::cout << to_binary(b) << " : " << b << '\n';
		c.add(a, b);
		c.setradix(radix);
		std::cout << to_binary(c) << " : " << c << '\n';
		uint64_t fractionBits = c.fraction_ull();
		std::cout << to_binary(fractionBits, true, radix) << '\n';
	}
}

void MultiplicationSetup()
{
	using namespace sw::universal;

	std::cout << "\n---------------------- \nblocksignificand multiplication setup\n";

	{
		constexpr size_t fbits = 3;
		constexpr size_t fhbits = fbits + 1;
		constexpr size_t mbits = 2 * fhbits;
		blocksignificand<mbits, uint8_t> a, b, c;

		int inputRadix = fbits;
		a.setbits(0x09); // 0b0000'1.001 = 1.125 in 8-bit blocksignificand form with radix=3
		a.setradix(inputRadix);
		b.setbits(0x09);
		b.setradix(inputRadix);
		std::cout << to_binary(a) << " : " << a << '\n';
		std::cout << to_binary(b) << " : " << b << '\n';
		c.mul(a, b);
		int outputRadix = 2 * fbits;
		c.setradix(outputRadix); // multiply output radix is 2*fbits
		std::cout << to_binary(c) << " : " << c << '\n';
		uint64_t fractionBits = c.fraction_ull();
		std::cout << to_binary(fractionBits, true, outputRadix) << '\n';
	}

}

void DivisionSetup()
{
	using namespace sw::universal;

	std::cout << "\n---------------------- \nblocksignificand division setup\n";

	{
		constexpr size_t fbits = 3;
		constexpr size_t fhbits = fbits + 1;
		constexpr size_t divbits = 2 * fhbits;
		blocksignificand<divbits, uint8_t> a, b, c;

		int inputRadix = 2 * fbits;
		a.setbits(0x48); // 0b01.00'1000 = 1.125 in 8-bit blocksignificand form with radix=6
		a.setradix(inputRadix);
		b.setbits(0x48);
		b.setradix(inputRadix);
		std::cout << to_binary(a) << " : " << a << '\n';
		std::cout << to_binary(b) << " : " << b << '\n';
		c.div(a, b);
		int outputRadix = 2 * fbits;
		c.setradix(outputRadix); // divide output radix is 2*fbits
		std::cout << to_binary(c) << " : " << c << '\n';
		uint64_t fractionBits = c.fraction_ull();
		std::cout << to_binary(fractionBits, true, outputRadix) << '\n';
	}

	{
		constexpr size_t fbits = 3;
		constexpr size_t fhbits = fbits + 1;
		constexpr size_t divbits = 2 * fhbits;
		blocksignificand<divbits, uint8_t> a, b, c;

		int inputRadix = 2 * fbits;
		a.setbits(0x48); // 0b01.00'1000 = 1.125 in 8-bit blocksignificand form with radix=6
		a.setradix(inputRadix);
		b.setbits(0x40); // 0b01.00'0000 = 1.0
		b.setradix(inputRadix);
		std::cout << to_binary(a) << " : " << a << '\n';
		std::cout << to_binary(b) << " : " << b << '\n';
		c.div(a, b);
		int outputRadix = 2 * fbits;
		c.setradix(outputRadix); // divide output radix is 2*fbits
		std::cout << to_binary(c) << " : " << c << '\n';
		uint64_t fractionBits = c.fraction_ull();
		std::cout << to_binary(fractionBits, true, outputRadix) << '\n';
	}
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "blocksignificand API examples";
	std::string test_tag    = "API";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	TestRounding();
	AdditionSetup();
	MultiplicationSetup();
	DivisionSetup();

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
