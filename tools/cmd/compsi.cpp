// compsi.cpp: components of a signed integer: cli to show the sign/scale/fraction components of a signed integer  
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <typeinfo>
#include <universal/number/integer/integer>

// receive an unsigned integer and print its components
int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	if (argc != 2) {
		cerr << "compsi : components of a signed integer\n";
		cerr << "Show the sign/scale/fraction components of a signed integer.\n";
		cerr << "Usage: compsi integer_value\n";
		cerr << "Example: compsi 1234567890123456789012345\n";
		cerr << "class sw::universal::integer<128,unsigned int>         : 1234567890123456789012345 (+,80,00000101011011100000111100110110101001100100010000111101111000101101111101111001)";
		cerr << endl;
		return EXIT_SUCCESS;  // signal successful completion for ctest
	}
	// integer attributes
	string arg = argv[1];
	size_t max_digits10 = arg.length() + 1;

	// integer size check
	integer<1032, uint32_t> ref;
	integer<1032, uint32_t> value;
	if (!parse(arg, value)) {
		cout << "Unable to parse the value: " << arg << ". Exiting..." << endl;
		return EXIT_FAILURE;
	}

	// TODO: need to honor format manipulator
	constexpr size_t columnWidth = 50;
	value >= 0 ? parse("0x7F", ref) : parse("-128", ref);
	if (value < ref) {
		integer<8, uint8_t> int8;
		parse(arg, int8);
		cout << setw(columnWidth) << left << typeid(int8).name() << ": " << setprecision(max_digits10) << right << int8 << " " << to_triple(int8) << endl;
		return EXIT_SUCCESS;
	}
	parse("0x7FFF", ref);
	if (value < ref) {
		integer<16, uint16_t> int16;
		parse(arg, int16);
		cout << setw(columnWidth) << left << typeid(int16).name()  << ": " << setprecision(max_digits10) << right << int16 << " " << to_triple(int16) << endl;
		return EXIT_SUCCESS;
	}
	parse("0x7FFFFFFF", ref);
	if (value < ref) {
		integer<32, uint32_t> int32;
		parse(arg, int32);
		cout << setw(columnWidth) << left << typeid(int32).name()  << ": " << setprecision(max_digits10) << right << int32 << " " << to_triple(int32) << endl;
		return EXIT_SUCCESS;
	}
	parse("0x7FFFFFFFFFFFFFFF", ref);
	if (value < ref) {
		integer<64, uint32_t> int64;
		parse(arg, int64);
		cout << setw(columnWidth) << left << typeid(int64).name()  << ": " << setprecision(max_digits10) << right << int64 << " " << to_triple(int64) << endl;
		return EXIT_SUCCESS;
	}
	parse("0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF", ref);
	if (value < ref) {
		integer<128, uint32_t> int128;
		parse(arg, int128);
		cout << setw(columnWidth) << left << typeid(int128).name() << ": " << setprecision(max_digits10) << right << int128 << " " << to_triple(int128) << endl;
		return EXIT_SUCCESS;
	}
	parse("0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF", ref);
	if (value < ref) {
		integer<256, uint32_t> int256;
		parse(arg, int256);
        cout << setw(columnWidth) << left << typeid(int256).name() << ": " << setprecision(max_digits10) << right << int256 << " " << to_triple(int256) << endl;
		return EXIT_SUCCESS;
	}
	parse("0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF", ref);
	if (value < ref) {
		integer<512, uint32_t> int512;
		parse(arg, int512);
		cout << setw(columnWidth) << left << typeid(int512).name() << ": " << setprecision(max_digits10) << right << int512 << " " << to_triple(int512) << endl;
		return EXIT_SUCCESS;
	}
	parse("0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF", ref);
	if (value < ref) {
		integer<1024, uint32_t> int1024;
		parse(arg, int1024);
		cout << setw(columnWidth) << left << typeid(int1024).name() << ": " << setprecision(max_digits10) << right << int1024 << " " << to_triple(int1024) << endl;
		return EXIT_SUCCESS;
	}
	cout << "The value " << arg << " is too large to be represented by a 1024 bit integer or smaller\n";
	cout << endl;

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
