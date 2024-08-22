// fixpnt.cpp: components of a fixed-point: cli to show the sign/scale/fraction components of a fixed-point value 
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>
#include <typeinfo>
#include <universal/number/fixpnt/fixpnt.hpp>

const char* msg = "\n\
                                                              class sw::universal::fixpnt < 8, 4, 1, unsigned char>\n\
1.0625 \n\
b0001.0001 \n\
                                                              class sw::universal::fixpnt < 12, 4, 1, unsigned char>\n\
1.0625\n\
b0000'0001.0001\n\
                                                              class sw::universal::fixpnt < 16, 8, 1, unsigned char>\n\
1.06250000\n\
b0000'0001.0001'0000\n\
                                                              class sw::universal::fixpnt < 32, 16, 1, unsigned char>\n\
1.0625000000000000\n\
b0000'0000'0000'0001.0001'0000'0000'0000\n\
                                                              class sw::universal::fixpnt < 64, 32, 1, unsigned char>\n\
1.06250000000000000000000000000000\n\
b0000'0000'0000'0000'0000'0000'0000'0001.0001'0000'0000'0000'0000'0000'0000'0000\n";

template<typename FixedPoint>
void attributes(const FixedPoint& v) {
	constexpr int max_digits10 = std::numeric_limits<FixedPoint>::max_digits10;
	std::cout << "                                                      " << typeid(FixedPoint).name() << '\n'  << std::setprecision(max_digits10) << v << "\n" << to_binary(v, true) << std::endl;
}

// receive a float and print its components
int main(int argc, char** argv)
try {
	using namespace sw::universal;

	if (argc != 2) {
		std::cerr << "fixpnt : components of a fixed-point value\n";
		std::cerr << "Show the sign/scale/fraction components of a fixed-point value.\n";
		std::cerr << "Usage: fixpnt float_value\n";
		std::cerr << "Example: fixpnt 1.0625\n";
		std::cerr << msg << '\n';
		return EXIT_SUCCESS;  // signal successful completion for ctest
	}
	double d = atof(argv[1]);

	attributes(fixpnt< 8, 4>(d));
	attributes(fixpnt<12, 4>(d));
	attributes(fixpnt<16, 8>(d));
	attributes(fixpnt<32,16>(d));
	attributes(fixpnt<64,32>(d));

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
