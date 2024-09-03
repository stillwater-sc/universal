// unsignedint.cpp: components of an unsigned integer: cli to show the sign/scale/fraction components of an unsigned integer  
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <typeinfo>
#include <universal/number/integer/integer.hpp>

// receive an unsigned integer and print its components
int main(int argc, char** argv)
try {
	using namespace sw::universal;

	if (argc != 2) {
		std::cerr << "unsignedint : components of an unsigned integer\n";
		std::cerr << "Show the sign/scale/fraction components of an unsigned integer.\n";
		std::cerr << "Usage: unsignedint integer_value\n";
		std::cerr << "Example: unsignedint 123456789012345670\n";
		std::cerr << "TBD: ";
		std::cerr << std::endl;
		return EXIT_SUCCESS;  // signal successful completion for ctest
	}
	// integer attributes
	std::string arg = argv[1];
	size_t max_digits10 = arg.length() + 1;

	// integer size check
	integer<1032, uint32_t> ref;
	integer<1032, uint32_t> value;
	if (!parse(arg, value)) {
		std::cout << "Unable to parse the value: " << arg << ". Exiting...\n";
		return EXIT_FAILURE;
	}

	// TODO: need to honor format manipulator
	constexpr size_t columnWidth = 50;
	parse("0x7F", ref);
	if (value < ref) {
		integer<8, uint8_t> int8;
		parse(arg, int8);
		std::cout << std::setw(columnWidth) << std::left << typeid(int8).name() << ": " << std::setprecision(max_digits10) << std::right << int8 << " " << to_triple(int8) << '\n';
		return EXIT_SUCCESS;
	}
	parse("0x7FFF", ref);
	if (value < ref) {
		integer<16, uint16_t> int16;
		parse(arg, int16);
		std::cout << std::setw(columnWidth) << std::left << typeid(int16).name()  << ": " << std::setprecision(max_digits10) << std::right << int16 << " " << to_triple(int16) << '\n';
		return EXIT_SUCCESS;
	}
	parse("0x7FFFFFFF", ref);
	if (value < ref) {
		integer<32, uint32_t> int32;
		parse(arg, int32);
		std::cout << std::setw(columnWidth) << std::left << typeid(int32).name()  << ": " << std::setprecision(max_digits10) << std::right << int32 << " " << to_triple(int32) << '\n';
		return EXIT_SUCCESS;
	}
	parse("0x7FFFFFFFFFFFFFFF", ref);
	if (value < ref) {
		integer<64, uint32_t> int64;
		parse(arg, int64);
		std::cout << std::setw(columnWidth) << std::left << typeid(int64).name()  << ": " << std::setprecision(max_digits10) << std::right << int64 << " " << to_triple(int64) << '\n';
		return EXIT_SUCCESS;
	}
	parse("0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF", ref);
	if (value < ref) {
		integer<128, uint32_t> int128;
		parse(arg, int128);
		std::cout << std::setw(columnWidth) << std::left << typeid(int128).name() << ": " << std::setprecision(max_digits10) << std::right << int128 << " " << to_triple(int128) << '\n';
		return EXIT_SUCCESS;
	}
	parse("0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF", ref);
	if (value < ref) {
		integer<256, uint32_t> int256;
		parse(arg, int256);
        std::cout << std::setw(columnWidth) << std::left << typeid(int256).name() << ": " << std::setprecision(max_digits10) << std::right << int256 << " " << to_triple(int256) << '\n';
		return EXIT_SUCCESS;
	}
	parse("0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF", ref);
	if (value < ref) {
		integer<512, uint32_t> int512;
		parse(arg, int512);
		std::cout << std::setw(columnWidth) << std::left << typeid(int512).name() << ": " << std::setprecision(max_digits10) << std::right << int512 << " " << to_triple(int512) << '\n';
		return EXIT_SUCCESS;
	}
	parse("0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF", ref);
	if (value < ref) {
		integer<1024, uint32_t> int1024;
		parse(arg, int1024);
		std::cout << std::setw(columnWidth) << std::left << typeid(int1024).name() << ": " << std::setprecision(max_digits10) << std::right << int1024 << " " << to_triple(int1024) << '\n';
		return EXIT_SUCCESS;
	}
	std::cout << "The value " << arg << " is too large to be represented by a 1024 bit integer or smaller\n";
	std::cout << std::endl;

	return EXIT_SUCCESS;
}
catch (const char* const msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
