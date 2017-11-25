// fc.cpp: show float components: show the sign/scale/fraction components of a float 
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include <posit>

using namespace std;

// receive a float and print its components
int main(int argc, char** argv)
try {
	if (argc != 2) {
		cerr << "Show the sign/scale/fraction components of a float.    Usage: fc float_value" << endl;
		return 1;
	}
	float f = atof(argv[1]);
	value<23> v(f);
	cout << components(v) << endl;
	return 0;

}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
