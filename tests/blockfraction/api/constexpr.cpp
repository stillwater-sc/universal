//  constexpr.cpp : compile-time tests for constexpr of blockfraction type
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>

//#include <universal/integer/integer.hpp>
#include <universal/internal/blockfraction/blockfraction.hpp>

int main()
try {
	using namespace std;
	using namespace sw::universal;

	std::string tag = "blockfraction storage class constexpr compile-time testing";

	// TODO: does it make sense to test constexpression-ness for blockfraction?
	// blockfraction doesn't have decorated constructors or assignment initializers
	// to set a non-zero value.
	{
		constexpr blockfraction<8, uint8_t> b8_1w{ 0x21, blockfraction<8, uint8_t>::rbit };
		constexpr blockfraction<8, uint16_t> b8_2b(0x21, blockfraction<8, uint8_t>::rbit);
		constexpr blockfraction<8, uint32_t> b8_4b(0x21);
		cout << to_binary(b8_1w, true) << " : " << b8_1w << '\n';
		cout << to_binary(b8_2b, true) << " : " << b8_2b << '\n';
		cout << to_binary(b8_4b, true) << " : " << b8_4b << '\n';
	}

	{
		constexpr blockfraction<16, uint8_t> b16_2b{ 0xff, blockfraction<16, uint8_t>::rbit };
		constexpr blockfraction<16, uint16_t> b16_1w{ 0x2001 };
		constexpr blockfraction<16, uint32_t> b16_4b( 0x2001);

		cout << to_binary(b16_2b, true) << " : " << b16_2b << '\n';
		cout << to_binary(b16_1w, true) << " : " << b16_1w << '\n';
		cout << to_binary(b16_4b, true) << " : " << b16_4b << '\n';
	}

	{
		constexpr blockfraction<32, uint8_t> b32_4b{ 0xff, blockfraction<32, uint8_t>::rbit };
		constexpr blockfraction<32, uint16_t> b32_2w{ 0x2001 };
		constexpr blockfraction<32, uint32_t> b32_1w(0x30000001); // == 1.5

		cout << to_binary(b32_4b, true) << " : " << b32_4b << '\n';
		cout << to_binary(b32_2w, true) << " : " << b32_2w << '\n';
		cout << to_binary(b32_1w, true) << " : " << b32_1w << '\n';
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
