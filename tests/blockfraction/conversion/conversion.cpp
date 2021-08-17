//  conversion.cpp : test suite runner for blockfraction construction and conversion from float/double
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>

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
int main()
try {
	using namespace std;
	using namespace sw::universal;

	std::string tag = "blockfraction storage class value conversion testing";

	// we have deprecated the blockfraction copy constructor to catch any
	// unsuspecting conversion copies in blockfraction use-cases
	{
		// scenario that happens in unrounded add/sub
		//  0b0'10.00'0000 : 2
		//  0b0'11.00'0000 : 3
		//	0b0'11.10'0000 : 3.5
		//	0b0'11.11'0000 : 3.75
		//	0b0'11.11'1000 : 3.875
		//	0b0'11.11'1100 : 3.9375
		//	0b0'11.11'1110 : 3.96875
		//	0b0'11.11'1111 : 3.98438
		// for add and sub the significant uses a 2's complement format 00h.ffff
		constexpr size_t fbits   = 8;
		constexpr size_t fhbits  = fbits + 1;
		//constexpr size_t abits   = fhbits + 3;
		//constexpr size_t sumbits = abits + 1;
		size_t msbMask = (1 << (fbits-1));
		size_t frac = msbMask;
		blockfraction<fhbits, uint8_t, BitEncoding::Twos> a;
		a.setradix(fhbits - 3);
		for (size_t i = 0; i < fbits; ++i) {
			a.setbits(frac);
			cout << to_binary(a, true) << " : " << a << '\n';
			msbMask >>= 1;
			frac |= msbMask;
		}
		// negative values
		//	0b1'00.00'0000 : -0
		//	0b1'10.00'0000 : -2
		//	0b1'11.00'0000 : -1
		//	0b1'11.10'0000 : -0.5
		//	0b1'11.11'0000 : -0.25
		//	0b1'11.11'1000 : -0.125
		//	0b1'11.11'1100 : -0.0625
		//	0b1'11.11'1110 : -0.03125
		msbMask = (1 << fbits);
		frac = msbMask;
		for (size_t i = 0; i < fbits; ++i) {
			a.setbits(frac);
			cout << to_binary(a, true) << " : " << -double(a) << '\n';
			msbMask >>= 1;
			frac |= msbMask;
		}
	}

	{
		//	0b1111111.1 : 127.5
		//	0b111111.11 : 63.75
		// 	0b11111.111 : 31.875
		//	0b1111.1111 : 15.9375
		//	0b111.11111 : 7.96875
		//	0b11.111111 : 3.98438
		//	0b1.1111111 : 1.99219
		constexpr size_t nbits = 8;
		blockfraction<nbits, uint8_t, BitEncoding::Ones> a(0xff, 1);
		for (int radix = 1; radix < nbits; ++radix) {
			a.setradix(radix);
			cout << to_binary(a) << " : " << a << '\n';
		}
	}
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
