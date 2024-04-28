// fixpnt.cpp : examples of fixpnt ranges
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>

// print fixed-point ranges
int main()
try {
	using namespace sw::universal;

	// value ranges of interesting fixed-point configurations

	// integers to understand the range
	std::cout << "Fixed-points with only integer bits\n";
	std::cout << fixpnt_range(fixpnt< 4, 0, Modulo, uint16_t>()) << '\n';
	std::cout << fixpnt_range(fixpnt< 8, 0, Modulo, uint16_t>()) << '\n';
	std::cout << fixpnt_range(fixpnt<12, 0, Modulo, uint16_t>()) << '\n';
	std::cout << fixpnt_range(fixpnt<16, 0, Modulo, uint16_t>()) << '\n';
	std::cout << fixpnt_range(fixpnt<20, 0, Modulo, uint16_t>()) << '\n';
	std::cout << fixpnt_range(fixpnt<24, 0, Modulo, uint16_t>()) << '\n';
	std::cout << fixpnt_range(fixpnt<28, 0, Modulo, uint16_t>()) << '\n';
	std::cout << fixpnt_range(fixpnt<32, 0, Modulo, uint16_t>()) << '\n';
	std::cout << '\n';

	// divide half the bits to integer and half to fraction
	std::cout << "Fixed-points that evenly divide bits between integer and fraction bits\n";
	std::cout << fixpnt_range(fixpnt< 4, 2, Modulo, uint16_t>()) << '\n';
	std::cout << fixpnt_range(fixpnt< 8, 4, Modulo, uint16_t>()) << '\n';
	std::cout << fixpnt_range(fixpnt<12, 6, Modulo, uint16_t>()) << '\n';
	std::cout << fixpnt_range(fixpnt<16, 8, Modulo, uint16_t>()) << '\n';
	std::cout << fixpnt_range(fixpnt<20, 10, Modulo, uint16_t>()) << '\n';
	std::cout << fixpnt_range(fixpnt<24, 12, Modulo, uint16_t>()) << '\n';
	std::cout << fixpnt_range(fixpnt<28, 14, Modulo, uint16_t>()) << '\n';
	std::cout << fixpnt_range(fixpnt<32, 16, Modulo, uint16_t>()) << '\n';
	std::cout << '\n';

	// allocate all bits to the fraction
	std::cout << "Fixed-points with only fraction bits\n";
	std::cout << fixpnt_range(fixpnt< 4, 4, Modulo, uint16_t>()) << '\n';
	std::cout << fixpnt_range(fixpnt< 8, 8, Modulo, uint16_t>()) << '\n';
	std::cout << fixpnt_range(fixpnt<12, 12, Modulo, uint16_t>()) << '\n';
	std::cout << fixpnt_range(fixpnt<16, 16, Modulo, uint16_t>()) << '\n';
	std::cout << fixpnt_range(fixpnt<20, 20, Modulo, uint16_t>()) << '\n';
	std::cout << fixpnt_range(fixpnt<24, 24, Modulo, uint16_t>()) << '\n';
	std::cout << fixpnt_range(fixpnt<28, 28, Modulo, uint16_t>()) << '\n';
	std::cout << fixpnt_range(fixpnt<32, 32, Modulo, uint16_t>()) << '\n';
	std::cout << '\n';


	// a super accumulator for single precision posits: quire<512,240>\n";
	// unfortunately, this does not produce a readable range spec, :-(
    // std::cout << fixpnt_range(fixpnt<512, 240, Modulo, uint32_t>());

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
