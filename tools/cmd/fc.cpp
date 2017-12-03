// fc.cpp: show float components: show the sign/scale/fraction components of a float 
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
		cerr << "Show the sign/scale/fraction components of a float." << endl;
	    cerr << "Usage: fc float_value" << endl;
		cerr << "Example: fc 0.03124999" << endl;
		cerr << "float: 0.03124999068677425384521484375 Sign: 0 Scale: -6 Fraction: b11111111111111111111011" << endl;
		return 1;
	}
	float f = float(atof(argv[1]));
	value<23> v(f);
	cout << "float: " << setprecision(40) << f << components(v) << endl;
	return 0;
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
