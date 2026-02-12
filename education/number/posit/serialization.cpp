//  serialization.cpp : examples how to serialize posit values
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// configure the posit environment to print error-free posit format
#define POSIT_ERROR_FREE_IO_FORMAT 1
//#include <universal/number/posit1/posit1.hpp>
#include <universal/number/posit/posit.hpp>

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;

	std::cout << "Lossless serialization of posit values\n";

	constexpr unsigned nbits = 40;
	constexpr unsigned es    =  3;
	posit<nbits, es> a, b, c;

	a =  1.23456789012345;
	b = -1.23456789012345;
	c = NAR;

	// without the POSIT_ROUNDING_ERROR_FREE_IO_FORMAT flag set
	// the following statements would simply print the value of the posits.
	// Default ostream behavior of a posit
	//	a :  1.23457
	//	b : -1.23457
	//	c : -nan(ind)
	// Now they will print in native posit format.
	// Lossless serialization of posit values
	//	a : 40.3x40f03290a3p
	//	b : 40.3xbf0fcd6f5dp
	//	c : 40.3x8000000000p
	std::cout << "a : " << a << '\n';
	std::cout << "b : " << b << '\n';
	std::cout << "c : " << c << '\n';

	// in addition to using a system-wide flag to modify ostream behavior
	// you can also print native posit format using an ostream helper
	std::cout << "Using an ostream helper\n";
	std::cout << "a : " << hex_format(a) << " a value : " << double(a) << '\n';
	std::cout << "b : " << hex_format(b) << " b value : " << double(b) << '\n';
	std::cout << "c : " << hex_format(c) << " c value : " << double(c) << '\n';

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
