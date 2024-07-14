// lns.cpp : enumeration of different lns ranges
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/lns/lns.hpp>

// print fixed-point ranges
int main()
try {
	using namespace sw::universal;

	// value ranges of interesting fixed-point configurations

	// integers to understand the range
	std::cout << "logarithmic number systems with only integer bits\n";
	std::cout << lns_range(lns< 4, 0, uint16_t>()) << '\n';
	std::cout << lns_range(lns< 8, 0, uint16_t>()) << '\n';
	std::cout << lns_range(lns<12, 0, uint16_t>()) << '\n';
	std::cout << lns_range(lns<16, 0, uint16_t>()) << '\n';
	std::cout << lns_range(lns<20, 0, uint16_t>()) << '\n';
	std::cout << lns_range(lns<24, 0, uint16_t>()) << '\n';
	std::cout << lns_range(lns<28, 0, uint16_t>()) << '\n';
	std::cout << lns_range(lns<32, 0, uint16_t>()) << '\n';
	std::cout << '\n';

	// divide half the bits to integer and half to fraction
	std::cout << "logarithmic number systems that evenly divide bits between integer and fraction bits\n";
	std::cout << lns_range(lns< 4, 2, uint16_t>()) << '\n';
	std::cout << lns_range(lns< 8, 4, uint16_t>()) << '\n';
	std::cout << lns_range(lns<12, 6, uint16_t>()) << '\n';
	std::cout << lns_range(lns<16, 8, uint16_t>()) << '\n';
	std::cout << lns_range(lns<20, 10, uint16_t>()) << '\n';
	std::cout << lns_range(lns<24, 12, uint16_t>()) << '\n';
	std::cout << lns_range(lns<28, 14, uint16_t>()) << '\n';
	std::cout << lns_range(lns<32, 16, uint16_t>()) << '\n';
	std::cout << '\n';

	// allocate all bits to the fraction
	std::cout << "logarithmic number systems with only fraction bits\n";
	std::cout << lns_range(lns< 4, 3, uint16_t>()) << '\n';
	std::cout << lns_range(lns< 8, 7, uint16_t>()) << '\n';
	std::cout << lns_range(lns<12, 11, uint16_t>()) << '\n';
	std::cout << lns_range(lns<16, 15, uint16_t>()) << '\n';
	std::cout << lns_range(lns<20, 19, uint16_t>()) << '\n';
	std::cout << lns_range(lns<24, 23, uint16_t>()) << '\n';
	std::cout << lns_range(lns<28, 27, uint16_t>()) << '\n';
	std::cout << lns_range(lns<32, 31, uint16_t>()) << '\n';
	std::cout << '\n';


	// a super accumulator for single precision posits: quire<512,240>\n";
	// unfortunately, this does not produce a readable range spec, :-(
    // std::cout << lns_range(lns<512, 240, uint32_t>());

	std::cout.flush();
	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
