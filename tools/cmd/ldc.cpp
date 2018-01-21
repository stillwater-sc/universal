// ldc.cpp: long double components: show the sign/scale/fraction components of a long double (128b quad precision float)
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include <value>

using namespace std;
using namespace sw::unum;

// receive a float and print the components of a long double representation
int main(int argc, char** argv)
try {
	// long double attributes
	constexpr int max_digits10 = std::numeric_limits<long double>::max_digits10;
	constexpr int fbits = std::numeric_limits<long double>::digits - 1;

	if (argc != 2) {
		cerr << "Show the sign/scale/fraction components of a double." << endl;
		cerr << "Usage: dc double_value" << endl;
		cerr << "Example: ldc 0.03124999" << endl;
		cerr << "long double: 0.031249989999999998 (+,-6,1111111111111111111101010100001100111000100011101110)" << endl;
		return EXIT_SUCCESS;   // signal successful completion for ctest
	}
	long double q = atof(argv[1]);
	value<fbits> v(q);

	cout << "long double: " << setprecision(max_digits10) << q << " " << components(v) << endl;
	return EXIT_SUCCESS;
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
