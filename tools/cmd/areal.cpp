// areal.cpp: show the components of different areal configuration
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/areal/areal.hpp>

typedef std::numeric_limits< double > dbl;
const char* msg = "\n\
                                                      class sw::universal::areal< 8, 2,unsigned char>\n\
10100011 : b1010'0011: (-1.0625, -1.125)\n\
                                                      class sw::universal::areal< 8, 3, unsigned char>\n\
10110001 : b1011'0001: (-1, -1.125)\n\
                                                      class sw::universal::areal< 9, 2, unsigned char>\n\
101000111 : b1'0100'0111: (-1.09375, -1.125)\n\
                                                      class sw::universal::areal< 9, 3, unsigned char>\n\
101100011 : b1'0110'0011: (-1.0625, -1.125)\n\
                                                      class sw::universal::areal<16, 2, unsigned char>\n\
1010001111110011 : b1010'0011'1111'0011: (-1.12329, -1.12354)\n\
                                                      class sw::universal::areal<16, 3, unsigned char>\n\
1011000111111001 : b1011'0001'1111'1001: (-1.12305, -1.12354)\n\
                                                      class sw::universal::areal<16, 4, unsigned char>\n\
1011100011111101 : b1011'1000'1111'1101: (-1.12305, -1.12402)\n\
                                                      class sw::universal::areal<16, 5, unsigned char>\n\
1011110001111111 : b1011'1100'0111'1111: (-1.12305, -1.125)\n\
                                                      class sw::universal::areal<16, 6, unsigned char>\n\
1011111000111111 : b1011'1110'0011'1111: (-1.12109, -1.125)\n\
                                                      class sw::universal::areal<32, 4, unsigned char>\n\
10111000111111001101011011101001 : b1011'1000'1111'1100'1101'0110'1110'1001: (-1.12346, -1.12346)\n\
                                                      class sw::universal::areal<32, 5, unsigned char>\n\
10111100011111100110101101110101 : b1011'1100'0111'1110'0110'1011'0111'0101: (-1.12346, -1.12346)\n\
                                                      class sw::universal::areal<32, 6, unsigned char>\n\
10111110001111110011010110111011 : b1011'1110'0011'1111'0011'0101'1011'1011: (-1.12346, -1.12346)\n";

template<typename Real>
void attributes(const Real& v) {
	std::cout << "                                                      " << typeid(Real).name() << '\n' << color_print(v) << " : " << to_binary(v, true) << " : " << v << std::endl;
}

// receive a float and print its components
int main(int argc, char** argv)
try {
	using namespace sw::universal;

	if (argc != 2) {
		std::cerr << "areal : areal components\n";
		std::cerr << "Show the sign/scale/fraction components of an areal.\n";
		std::cerr << "Usage: areal float_value\n";
		std::cerr << "Example: areal -1.123456789\n";
		std::cerr <<  msg << '\n';
		return EXIT_SUCCESS;  // signal successful completion for ctest
	}
	double d = atof(argv[1]);
	attributes(areal<8, 2>(d));
	attributes(areal<8, 3>(d));
	attributes(areal<9, 2>(d));
	attributes(areal<9, 3>(d));
	attributes(areal<16, 2>(d));
	attributes(areal<16, 3>(d));
	attributes(areal<16, 4>(d));
	attributes(areal<16, 5>(d));
	attributes(areal<16, 6>(d));
	attributes(areal<32, 4>(d));
	attributes(areal<32, 5>(d));
	attributes(areal<32, 6>(d));

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
