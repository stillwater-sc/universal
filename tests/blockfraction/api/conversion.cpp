//  conversion.cpp : test suite runner for blockfraction construction and conversion of blockfraction
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>

#include <universal/internal/blockfraction/blockfraction.hpp>

int main()
try {
	using namespace std;
	using namespace sw::universal;

	std::string tag = "blockfraction storage class construction/conversion testing";

	{
		// scenario that happens in unrounded add/sub where blockfraction is used as storage type for fraction or significant
		constexpr size_t fbits = 8;
		constexpr size_t fhbits = fbits + 1;
		constexpr size_t abits = fhbits + 3;
		constexpr size_t sumbits = abits + 1;
		size_t msbMask = 1;
		blockfraction<fhbits, uint8_t> a;
		for (size_t i = 0; i < fbits; ++i) {
			a.set_raw_bits(msbMask);
			blockfraction<sumbits> b(a);
			cout << to_binary(a, true) << '\n';
			cout << to_binary(b, true) << '\n';
			msbMask <<= 1;
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
