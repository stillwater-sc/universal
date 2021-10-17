//  api.cpp : test suite runner for blockfraction application programming interface tests
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <iostream>

#include <universal/native/integers.hpp>
#include <universal/internal/blockfraction/blockfraction.hpp>

/*
A blockfraction is a 2's complement binary encoding with a radix point that is aligned
with the hidden bit of the fraction encoding in a floating-point representation.

The main goal of the blockfraction abstraction is to support arbitrary floating-point 
number systems with a high-quality, high-performance arithmetic engine.

The expensive part in these abstractions is the need to receive, expand, and align
bit strings, so special attention is given to fast implementations using copies.
This is acceptable, and leads to cleaner code, for small representations. However,
for very large representations these copies become prohibitive, and for those situations
the blockfraction is not a good solution.

*/

template<typename BlockFraction>
void PrintRoundingMode(const BlockFraction& a, size_t targetLsb) {
	std::cout << to_binary(a) << " target lsb = " << targetLsb << " ->rounding mode is " << (a.roundingMode(targetLsb) ? "up" : "down") << '\n';
}

int main()
try {
	using namespace sw::universal;

	std::string tag = "blockfraction storage class construction/conversion testing";

	{
		blockfraction<7, uint8_t, BitEncoding::Twos> a, b, c;
		a.setbits(0x11); // 1.0 in 7-bit blockfraction form
		b.setbits(0x11);
		std::cout << to_binary(a) << " : " << a << '\n';
		std::cout << to_binary(b) << " : " << b << '\n';
		c.add(a, b);
		std::cout << to_binary(c) << " : " << c << '\n';
		uint64_t fractionBits = c.fraction_ull();
		std::cout << to_binary(fractionBits, 4) << '\n';
	}

	{
		// a cfloat<8,2> has 5 fraction bits
		// a 00h.fffff format is thus 8 bits
		// By design, the 00h.fffff format contains all the valid values
		// for addition and subtraction.
		blockfraction<8, uint8_t, BitEncoding::Twos> a, b, c;
		a.setbits(0x21); // 1.0 in 8-bit blockfraction form
		b.setbits(0x21);
		std::cout << to_binary(a) << " : " << a << '\n';
		std::cout << to_binary(b) << " : " << b << '\n';
		c.add(a, b);
		std::cout << to_binary(c) << " : " << c << '\n';
		uint64_t fractionBits = c.fraction_ull();
		std::cout << to_binary(fractionBits, 5) << '\n';
	}

	{
		blockfraction<12, uint8_t, BitEncoding::Twos> a, b, c;
		a.setbits(0x100);
		b.setbits(0x200);
		b.twosComplement();
		std::cout << to_binary(a) << " : " << a << '\n';
		std::cout << to_binary(b) << " : " << b << '\n';
		c.add(a, b);
		std::cout << to_binary(c) << " : " << c << '\n';
		uint64_t fractionBits = c.fraction_ull();
		std::cout << to_binary(fractionBits, 9) << '\n';
	}

	// rounding 
	// 0000'0000  lsb target is at(3)
	blockfraction<8, uint8_t, BitEncoding::Ones> a;
	size_t lsbTarget = 3;
	a.setbits(0x0F);	// 00001111  up
	PrintRoundingMode(a, lsbTarget);
	a.setbits(0x07); 	// 00000111  up
	PrintRoundingMode(a, lsbTarget);
	a.setbits(0x03);	// 00000011  down
	PrintRoundingMode(a, lsbTarget);
	a.setbits(0x04); 	// 00000100  up: tie, round to even, which is down in this case
	PrintRoundingMode(a, lsbTarget);
	a.setbits(0x0C); 	// 00001100  up: tie, round to even, which is up in this case
	PrintRoundingMode(a, lsbTarget);
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
