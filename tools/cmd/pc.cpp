// pc.cpp: show standard posit components: show the sign/scale/regime/exponent/fraction components of a posit 
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include <posit>

using namespace std;
using namespace sw::unum;

typedef std::numeric_limits< double > dbl;

// receive a float and print its components
int main(int argc, char** argv)
try {
	if (argc != 2) {
		cerr << "Show the sign/scale/regime/exponent/fraction components of a posit." << endl;
	    cerr << "Usage: pc float_value" << endl;
		cerr << "Example: pc -1.123456789e17" << endl;
		cerr << "posit< 8,0>: " << endl;
		cerr << "posit<16,1>: " << endl;
		cerr << "posit<32,2>: " << endl;
		cerr << "posit<64,3>: " << endl;
		return EXIT_SUCCESS;  // signal successful completion for ctest
	}
	double d = atof(argv[1]);
	posit<8, 0>  p8(d);
	posit<16, 1> p16(d);
	posit<32, 2> p32(d);
	posit<48, 2> p48(d);
	//posit<64, 3> p64(d);

	int precision = dbl::max_digits10;
	cout << "posit< 8,0> = " << pretty_print(p8, precision) << endl;
	cout << "posit<16,1> = " << pretty_print(p16, precision) << endl;
	cout << "posit<32,2> = " << pretty_print(p32, precision) << endl;
	cout << "posit<48,2> = " << pretty_print(p48, precision) << endl;
	//cout << "posit<64,3> = " << pretty_print(p64, precision) << endl;

	return EXIT_SUCCESS;
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
