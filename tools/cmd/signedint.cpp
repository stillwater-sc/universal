// signedint.cpp: components of a signed integer: cli to show the sign/scale/fraction components of a signed integer  
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>
#include <typeinfo>
#include <universal/number/integer/integer.hpp>

template<typename Scalar>
void ParseAndReport(const std::string& arg) {
	Scalar v;
	sw::universal::parse(arg, v);
	std::cout << "The input    : " << arg << " can be represented by " << typeid(v).name() << '\n';
	std::cout << " value       : " << v << '\n';
	std::cout << " binary form : " << sw::universal::to_binary(v, true) << '\n';
	std::cout << " triple form : " << sw::universal::to_triple(v) << '\n';
}

// receive an unsigned integer and print its components
int main(int argc, char** argv)
try {
	using namespace sw::universal;

	if (argc != 2) {
		std::cerr << "signedint : components of a signed integer\n";
		std::cerr << "Show the sign/scale/fraction components of a signed integer.\n";
		std::cerr << "Usage: signedint value\n";
		std::cerr << "Example: signedint 1234567890123456789012345\n";
		std::cerr << "class sw::universal::integer<128,unsigned int>         : 1234567890123456789012345 (+,80,00000101011011100000111100110110101001100100010000111101111000101101111101111001)\n";

		return EXIT_SUCCESS;  // signal successful completion for ctest
	}
	// integer attributes
	std::string arg = argv[1];
//	size_t max_digits10 = arg.length() + 1;

	// integer size check
	integer<1032, uint32_t> ref;
	integer<1032, uint32_t> value;
	if (!parse(arg, value)) {
		std::cout << "Unable to parse the value: " << arg << ". Exiting...\n";
		return EXIT_FAILURE;
	}
	integer<1032, uint32_t> absValue = value.isneg() ? -value : value;
	parse("0x7F", ref);
	if (absValue < ref) {
		using Scalar = integer<8, uint8_t>;
		ParseAndReport<Scalar>(arg);
		return EXIT_SUCCESS;
	}
	parse("0x7FFF", ref);
	if (absValue < ref) {
		using Scalar = integer<16, uint16_t>;
		ParseAndReport<Scalar>(arg);
		return EXIT_SUCCESS;
	}
	parse("0x7FFFFFFF", ref);
	if (absValue < ref) {
		using Scalar = integer<32, uint32_t>;
		ParseAndReport<Scalar>(arg);
		return EXIT_SUCCESS;
	}
	parse("0x7FFFFFFFFFFFFFFF", ref);
	if (absValue < ref) {
		using Scalar = integer<64, uint32_t>;
		ParseAndReport<Scalar>(arg);
		return EXIT_SUCCESS;
	}
	parse("0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF", ref);
	if (absValue < ref) {
		using Scalar = integer<128, uint32_t>;
		ParseAndReport<Scalar>(arg);
		return EXIT_SUCCESS;
	}
	parse("0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF", ref);
	if (absValue < ref) {
		using Scalar = integer<256, uint32_t>;
		ParseAndReport<Scalar>(arg);
		return EXIT_SUCCESS;
	}
	parse("0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF", ref);
	if (absValue < ref) {
		using Scalar = integer<512, uint32_t>;
		ParseAndReport<Scalar>(arg);
		return EXIT_SUCCESS;
	}
	parse("0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF", ref);
	if (absValue < ref) {
		using Scalar = integer<1024, uint32_t>;
		ParseAndReport<Scalar>(arg);
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
