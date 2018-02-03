// numeric_limits.cpp: show the numeric limits of the compiler environment
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include <value>

using namespace std;
using namespace sw::unum;

// receive a float and print the components of a IEEE float representations
int main(int argc, char** argv)
try {
	// long double attributes
	constexpr int f_prec = std::numeric_limits<float>::max_digits10;
	constexpr int d_prec = std::numeric_limits<double>::max_digits10;
	constexpr int q_prec = std::numeric_limits<long double>::max_digits10;

	constexpr int uint8_bits = std::numeric_limits<unsigned char>::digits;
	constexpr int uint16_bits = std::numeric_limits<unsigned short>::digits;
	constexpr int uint32_bits = std::numeric_limits<unsigned int>::digits;
	constexpr int uint64_bits = std::numeric_limits<unsigned long long>::digits;
	constexpr int int8_bits = std::numeric_limits<signed char>::digits;
	constexpr int int16_bits = std::numeric_limits<short>::digits;
	constexpr int int32_bits = std::numeric_limits<int>::digits;
	constexpr int int64_bits = std::numeric_limits<long long>::digits;
	constexpr int f_fbits = std::numeric_limits<float>::digits;
	constexpr int d_fbits = std::numeric_limits<double>::digits;
	constexpr int q_fbits = std::numeric_limits<long double>::digits;

	cout << "Number of bits in native types" << endl;
	cout << "unsigned char        " << setw(4) << uint8_bits  << " bits" << endl;
	cout << "unsigned short       " << setw(4) << uint16_bits << " bits" << endl;
	cout << "unsigned int         " << setw(4) << uint32_bits << " bits" << endl;
	cout << "unsigned long long   " << setw(4) << uint64_bits << " bits" << endl;
	cout << "  signed char        " << setw(4) << int8_bits   << " bits" << endl;
	cout << "  signed short       " << setw(4) << int16_bits  << " bits" << endl;
	cout << "  signed int         " << setw(4) << int32_bits  << " bits" << endl;
	cout << "  signed long long   " << setw(4) << int64_bits  << " bits" << endl;
	cout << "         float       " << setw(4) << f_fbits     << " bits" << endl;
	cout << "         double      " << setw(4) << d_fbits     << " bits" << endl;
	cout << "         long double " << setw(4) << q_fbits     << " bits" << endl;

	union {
		long double da;
		char        bytes[16];
	} ld;
	bool        sign = false;
	int         exponent = 0;
	long double fraction = 0;
	ld.da = 1.234567890123456789;
	sw::unum::extract_fp_components(ld.da, sign, &exponent, fraction);
	cout << "value    " << setprecision(q_prec) << ld.da << setprecision(f_prec) << endl;
	cout << "hex      ";
	cout << hex << setfill('0');
	for (int i = 15; i >= 0; i--) {
		cout << setw(2) << (int)(uint8_t)ld.bytes[i] << " ";
	}
	cout << dec << endl;
	cout << "sign     " << (sign ? "-" : "+") << endl;
	cout << "exponent " << exponent << endl;
	cout << "fraction " << fraction << endl;


	return EXIT_SUCCESS;
}
catch (char const* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
catch (...) {
	cerr << "Caught unknown exception" << endl;
	return EXIT_FAILURE;
}
