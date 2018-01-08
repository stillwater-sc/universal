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
	posit<8, 0>  p8_0(d);
	posit<8, 1>  p8_1(d);
	posit<8, 2>  p8_2(d);
	posit<8, 3>  p8_3(d);
	posit<16, 1> p16_1(d);
	posit<16, 2> p16_2(d);
	posit<16, 3> p16_3(d);
	posit<32, 1> p32_1(d);
	posit<32, 2> p32_2(d);
	posit<32, 3> p32_3(d);
	posit<48, 1> p48_1(d);
	posit<48, 2> p48_2(d);
	posit<48, 3> p48_3(d);
	posit<64, 1> p64_1(d);
	posit<64, 2> p64_2(d);
	posit<64, 3> p64_3(d);

	int precision = dbl::max_digits10;
	cout << "posit< 8,0> = " << pretty_print(p8_0, precision) << endl;
	cout << "posit< 8,1> = " << pretty_print(p8_1, precision) << endl;
	cout << "posit< 8,2> = " << pretty_print(p8_2, precision) << endl;
	cout << "posit< 8,3> = " << pretty_print(p8_3, precision) << endl;
	cout << "posit<16,1> = " << pretty_print(p16_1, precision) << endl;
	cout << "posit<16,2> = " << pretty_print(p16_2, precision) << endl;
	cout << "posit<16,3> = " << pretty_print(p16_3, precision) << endl;
	cout << "posit<32,1> = " << pretty_print(p32_1, precision) << endl;
	cout << "posit<32,2> = " << pretty_print(p32_2, precision) << endl;
	cout << "posit<32,3> = " << pretty_print(p32_3, precision) << endl;
	cout << "posit<48,1> = " << pretty_print(p48_1, precision) << endl;
	cout << "posit<48,2> = " << pretty_print(p48_2, precision) << endl;
	cout << "posit<48,3> = " << pretty_print(p48_3, precision) << endl;
	cout << "posit<64,1> = " << pretty_print(p64_1, precision) << endl;
	cout << "posit<64,2> = " << pretty_print(p64_2, precision) << endl;
	cout << "posit<64,3> = " << pretty_print(p64_3, precision) << endl;

	return EXIT_SUCCESS;
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
