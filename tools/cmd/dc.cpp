// dc.cpp: show double components: show the sign/scale/fraction components of a double
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include <posit>

using namespace std;
using namespace sw::unum;

// receive a float and print its components
int main(int argc, char** argv)
try {
	if (argc != 2) {
		cerr << "Show the sign/scale/fraction components of a double." << endl;
		cerr << "Usage: dc double_value" << endl;
		cerr << "Example: dc 0.03124999" << endl;
		cerr << "double: 0.03124998999999999832466457405644177924842 Sign: 0 Scale: -6 Fraction: b1111111111111111111101010100001100111000100011101110" << endl;
		return EXIT_SUCCESS;   // signal successful completion for ctest
	}
	double d = atof(argv[1]);
	value<52> v(d);
	cout << "double: " << setprecision(40) << d << components(v) << endl;
	return EXIT_SUCCESS;
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
