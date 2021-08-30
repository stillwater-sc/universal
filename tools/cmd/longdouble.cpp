// longdouble.cpp: components of a long double: cli to show the sign/scale/fraction components of a long double native IEEE float
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <limits>
#include <universal/internal/value/value>

// receive a float and print the components of a long double representation
int main(int argc, char** argv)
try {
	using namespace sw::universal::internal;

	// long double attributes
	constexpr int max_digits10 = std::numeric_limits<long double>::max_digits10;
	constexpr int fbits = std::numeric_limits<long double>::digits - 1;

	if (argc != 2) {
		std::cerr << "longdouble: components of an IEEE long-double (compiler dependent, 80-bit extended precision on x86 and ARM, 128-bit on RISC-V\n";
		std::cerr << "Show the sign/scale/fraction components of an IEEE long double.\n";
		std::cerr << "Usage: longdouble long_double_value\n";
		std::cerr << "Example: compld 0.03124999\n";
		std::cerr << "long double: 0.0312499899999999983247 (+,-6,000000000000000000000000000000000011111111111110000000000000000)\n";
		return EXIT_SUCCESS;   // signal successful completion for ctest
	}
	long double q = atof(argv[1]);
	value<fbits> v(q);

	std::cout << "long double: " << std::setprecision(max_digits10) << q << " " << to_triple(v) << '\n';
	return EXIT_SUCCESS;
}
catch (const char* const msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
