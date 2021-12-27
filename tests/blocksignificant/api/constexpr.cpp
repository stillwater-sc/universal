//  constexpr.cpp : compile-time tests for constexpr of blocksignificant type
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <iostream>

#include <universal/internal/blocksignificant/blocksignificant.hpp>

template<typename blocksignificant>
void ConstexprBlockConstructor(uint64_t pattern) {
	constexpr blocksignificant bf(pattern);
	std::cout << to_binary(bf) << " : " << bf << '\n';
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "blocksignificant storage class constexpr compile-time testing";

	std::cout << test_suite << '\n';

	{
		constexpr blocksignificant<8, uint8_t, BitEncoding::Twos> b8_1w( 0x21, 5 );  // == 0b001.0'0001  = 1.03125
		constexpr blocksignificant<8, uint16_t, BitEncoding::Twos> b8_2b( 0x21, 5 ); // == 0b001.0'0001  = 1.03125
		constexpr blocksignificant<8, uint32_t, BitEncoding::Twos> b8_4b( 0x21, 5 ); // == 0b001.0'0001  = 1.03125
		std::cout << to_binary(b8_1w, true) << " : " << b8_1w << '\n';
		std::cout << to_binary(b8_2b, true) << " : " << b8_2b << '\n';
		std::cout << to_binary(b8_4b, true) << " : " << b8_4b << '\n';
	}

	{
		constexpr blocksignificant<12, uint8_t, BitEncoding::Twos>  b12_1w(0x210, 9); // == 0b001.0'0001'0000  = 1.03125
		constexpr blocksignificant<12, uint16_t, BitEncoding::Twos> b12_2b(0x210, 9); // == 0b001.0'0001'0000  = 1.03125
		constexpr blocksignificant<12, uint32_t, BitEncoding::Twos> b12_4b(0x210, 9); // == 0b001.0'0001'0000  = 1.03125
		std::cout << to_binary(b12_1w, true) << " : " << b12_1w << '\n';
		std::cout << to_binary(b12_2b, true) << " : " << b12_2b << '\n';
		std::cout << to_binary(b12_4b, true) << " : " << b12_4b << '\n';
	}

	{
		constexpr blocksignificant<16, uint8_t, BitEncoding::Twos> b16_2b( 0xff, 13 );  // subnormal
		constexpr blocksignificant<16, uint16_t, BitEncoding::Twos> b16_1w( 0x2001, 13 );
		constexpr blocksignificant<16, uint32_t, BitEncoding::Twos> b16_4b( 0x2001, 13 );

		std::cout << to_binary(b16_2b, true) << " : " << b16_2b << '\n';
		std::cout << to_binary(b16_1w, true) << " : " << b16_1w << '\n';
		std::cout << to_binary(b16_4b, true) << " : " << b16_4b << '\n';
	}

	{
		constexpr blocksignificant<32, uint8_t, BitEncoding::Twos> b32_4b( 0xff, 29 );
		constexpr blocksignificant<32, uint16_t, BitEncoding::Twos> b32_2w( 0x2001, 29 );
		constexpr blocksignificant<32, uint32_t, BitEncoding::Twos> b32_1w( 0x30000001, 29 ); // == 1.5

		std::cout << to_binary(b32_4b, true) << " : " << b32_4b << '\n';
		std::cout << to_binary(b32_2w, true) << " : " << b32_2w << '\n';
		std::cout << to_binary(b32_1w, true) << " : " << b32_1w << '\n';
	}

	{
		constexpr blocksignificant<32, uint8_t, BitEncoding::Twos> bf(0xAAAA'AAAA'5AAA'AAAA, 29);
		std::cout << to_binary(bf, true) << " : " << bf << '\n';
	}
	{
		constexpr blocksignificant<32, uint16_t, BitEncoding::Twos> bf(0xAAAA'AAAA'5AAA'AAAA, 29);
		std::cout << to_binary(bf, true) << " : " << bf << '\n';
	}
	{
		constexpr blocksignificant<32, uint32_t, BitEncoding::Twos> bf(0xAAAA'AAAA'5AAA'AAAA, 29);
		std::cout << to_binary(bf, true) << " : " << bf << '\n';
	}
	{
		constexpr blocksignificant<32, uint64_t, BitEncoding::Twos> bf(0xAAAA'AAAA'5AAA'AAAA, 29);
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
