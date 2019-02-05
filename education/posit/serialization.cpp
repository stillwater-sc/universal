//  serialization.cpp : examples how to serialize posit values
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "common.hpp"

#define POSIT_ROUNDING_ERROR_FREE_IO_FORMAT 1
#include <posit>

int main()
try {
	using namespace std;
	using namespace sw::unum;

	int nrOfFailedTestCases = 0;

	cout << "Serialization of posit values\n";

	constexpr size_t nbits = 40;
	constexpr size_t es    =  3;
	posit<nbits, es> a, b, c;

	a =  1.23456789012345;
	b = -1.23456789012345;
	c = NAR;

	cout << "a : " << a << endl;
	cout << "b : " << b << endl;
	cout << "c : " << c << endl;

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
