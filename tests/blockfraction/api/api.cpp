//  api.cpp : test suite runner for blockfraction application programming interface tests
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
		blockfraction<7, uint8_t> a, b;
		blockfraction<8, uint8_t> c;
		a.setbits(0x21); // 1.0 in 7-bit blockfraction form
		b.setbits(0x21);
		cout << ' ' << a << "\n " << b << '\n';
		c.uradd(a, b);
		// this moves the radix from nbits-2 to nbits-3
		cout << c << endl;
	}

	{
		blockfraction<7, uint8_t> a, b, c;
		a.setbits(0x21); // 1.0 in 7-bit blockfraction form
		b.setbits(0x21);
		cout << ' ' << a << "\n " << b << '\n';
		c.add(a, b);
		// add() keeps the radix point at nbits-2
		cout << ' ' << c << endl;
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
