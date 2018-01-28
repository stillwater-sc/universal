// fc.cpp: float components: show the sign/scale/fraction components of a float 
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include <value>

using namespace std;
using namespace sw::unum;

// receive a float and print its components
int main(int argc, char** argv)
try {
	// float attributes
	constexpr int max_digits10 = std::numeric_limits<double>::max_digits10;
	constexpr int fbits = std::numeric_limits<float>::digits - 1;

	if (argc != 2) {
		cerr << "Show the sign/scale/fraction components of a float." << endl;
	    cerr << "Usage: fc float_value" << endl;
		cerr << "Example: fc 0.03124999" << endl;
		cerr << "float: 0.031249990686774254 (+,-6,11111111111111111111011)" << endl;
		return EXIT_SUCCESS;  // signal successful completion for ctest
	}
	float f = float(atof(argv[1]));
	value<fbits> v(f);

	cout << "float: " << setprecision(max_digits10) << f << " " << components(v) << endl;
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
