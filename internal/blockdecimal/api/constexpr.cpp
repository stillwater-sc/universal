//  constexpr.cpp : compile-time tests for constexpr of blockdecimal type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <iostream>

#include <universal/internal/blockdecimal/blockdecimal.hpp>

int main()
try {
	using namespace sw::universal;

	std::string tag = "blockdecimal storage class constexpr compile-time testing";

	{
//		constexpr blockdecimal<8> dec;

//		std::cout << dec << '\n';
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
