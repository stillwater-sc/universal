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

	std::string tag = "blockfraction storage class construction/conversion testing";

	{
		// scenario that happens in unrounded add/sub where blockfraction is used as storage type for fraction or significant
		constexpr size_t fbits   = 8;
		constexpr size_t fhbits  = fbits + 1;
		constexpr size_t abits   = fhbits + 3;
		constexpr size_t sumbits = abits + 1;
		size_t msbMask = (1 << fbits);
		size_t frac = msbMask;
		blockfraction<fhbits, uint8_t> a;
		for (size_t i = 0; i < fbits; ++i) {
			a.set_raw_bits(frac);
			blockfraction<sumbits, uint8_t> b(a);   // blockfraction copies map MSB -> MSB
			cout << to_binary(a, true) << '\n';
			cout << to_binary(b, true) << '\n';
			msbMask >>= 1;
			frac |= msbMask;
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
