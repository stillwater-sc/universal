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

	{
		constexpr blockfraction<8, uint8_t> b8_1b;
		constexpr blockfraction<8, uint16_t> b8_2b;
		constexpr blockfraction<8, uint32_t> b8_4b;

		cout << b8_1b << '\n' << b8_2b << '\n' << b8_4b << endl;
	}

	{
		constexpr blockfraction<16, uint8_t> b16_2;
		constexpr blockfraction<16, uint16_t> b16_1;
		constexpr blockfraction<16, uint32_t> b16_4b;

		cout << b16_1 << '\n' << b16_2 << '\n' << b16_4b << endl;
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
