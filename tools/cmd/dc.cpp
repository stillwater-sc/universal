// dc.cpp: show double components: show the sign/scale/fraction components of a double
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
		cerr << "Show the sign/scale/fraction components of a double.    Usage: dc double_value" << endl;
		return 1;
	}
	double d = atof(argv[1]);
	value<52> v(d);
	cout << components(v) << endl;
	return 0;

}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
