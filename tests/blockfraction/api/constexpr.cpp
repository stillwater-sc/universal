//  constexpr.cpp : compile-time tests for constexpr of blockfraction type
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>

//#include <universal/integer/integer.hpp>
#include <universal/internal/blockfraction/blockfraction.hpp>

template<typename Blockfraction>
void ConstexprBlockConstructor(uint64_t pattern) {
	constexpr Blockfraction bf(pattern);
	std::cout << to_binary(bf) << " : " << bf << '\n';
}

int main()
try {
	using namespace std;
	using namespace sw::universal;

	std::string tag = "blockfraction storage class constexpr compile-time testing";

	{
		constexpr blockfraction<8, uint8_t, BitEncoding::Twos> b8_1w( 0x21, 5 );
		constexpr blockfraction<8, uint16_t, BitEncoding::Twos> b8_2b( 0x21, 5 );
		constexpr blockfraction<8, uint32_t, BitEncoding::Twos> b8_4b( 0x21, 5 );
		cout << to_binary(b8_1w, true) << " : " << b8_1w << '\n';
		cout << to_binary(b8_2b, true) << " : " << b8_2b << '\n';
		cout << to_binary(b8_4b, true) << " : " << b8_4b << '\n';
	}

	{
		constexpr blockfraction<16, uint8_t, BitEncoding::Twos> b16_2b( 0xff, 13 );  // subnormal
		constexpr blockfraction<16, uint16_t, BitEncoding::Twos> b16_1w( 0x2001, 13 );
		constexpr blockfraction<16, uint32_t, BitEncoding::Twos> b16_4b( 0x2001, 13 );

		cout << to_binary(b16_2b, true) << " : " << b16_2b << '\n';
		cout << to_binary(b16_1w, true) << " : " << b16_1w << '\n';
		cout << to_binary(b16_4b, true) << " : " << b16_4b << '\n';
	}

	{
		constexpr blockfraction<32, uint8_t, BitEncoding::Twos> b32_4b( 0xff, 29 );
		constexpr blockfraction<32, uint16_t, BitEncoding::Twos> b32_2w( 0x2001, 29 );
		constexpr blockfraction<32, uint32_t, BitEncoding::Twos> b32_1w( 0x30000001, 29 ); // == 1.5

		cout << to_binary(b32_4b, true) << " : " << b32_4b << '\n';
		cout << to_binary(b32_2w, true) << " : " << b32_2w << '\n';
		cout << to_binary(b32_1w, true) << " : " << b32_1w << '\n';
	}

	{
		constexpr blockfraction<32, uint8_t, BitEncoding::Twos> bf(0xAAAA'AAAA'5AAA'AAAA, 29);
		std::cout << to_binary(bf, true) << " : " << bf << '\n';
	}
	{
		constexpr blockfraction<32, uint16_t, BitEncoding::Twos> bf(0xAAAA'AAAA'5AAA'AAAA, 29);
		std::cout << to_binary(bf, true) << " : " << bf << '\n';
	}
	{
		constexpr blockfraction<32, uint32_t, BitEncoding::Twos> bf(0xAAAA'AAAA'5AAA'AAAA, 29);
		std::cout << to_binary(bf, true) << " : " << bf << '\n';
	}
	{
		constexpr blockfraction<32, uint64_t, BitEncoding::Twos> bf(0xAAAA'AAAA'5AAA'AAAA, 29);
		std::cout << to_binary(bf, true) << " : " << bf << '\n';
	}

	return EXIT_SUCCESS;
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
