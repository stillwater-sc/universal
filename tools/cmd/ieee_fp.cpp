// ieee_fp.cpp: show the sign/scale/fraction components of a 32b/64/128b floats
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include <value>

using namespace std;
using namespace sw::unum;

// receive a float and print the components of a IEEE float representations
int main(int argc, char** argv)
try {
	// long double attributes
	constexpr int f_prec = std::numeric_limits<float>::max_digits10;
	constexpr int d_prec = std::numeric_limits<double>::max_digits10;
	constexpr int q_prec = std::numeric_limits<long double>::max_digits10;

	constexpr int f_fbits = std::numeric_limits<float>::digits - 1;
	constexpr int d_fbits = std::numeric_limits<double>::digits - 1;
	constexpr int q_fbits = std::numeric_limits<long double>::digits - 1;

	if (argc != 2) {
		cerr << "Show the sign/scale/fraction components of a double." << endl;
		cerr << "Usage: ieee_fp float_value" << endl;
		cerr << "Example: ieee_fp 0.03124999" << endl;
                cerr << "input value:                0.03124999" << endl;
                cerr << "      float:              0.0312499907 (+,-6,11111111111111111111011)" << endl;
                cerr << "     double:      0.031249989999999998 (+,-6,1111111111111111111101010100001100111000100011101110)" << endl;
                cerr << "long double:  0.0312499899999999983247 (+,-6,111111111111111111101001011110100011111111111110001111111001111)" << endl;

		return EXIT_SUCCESS;   // signal successful completion for ctest
	}
	double d      = atof(argv[1]);
	float f       = d;
	long double q = d;
	value<f_fbits> vf(f);
	value<d_fbits> vd(d);
	value<q_fbits> vq(q);

        int width = q_prec + 4;

	cout << "input value: " << setprecision(f_prec) << setw(width) << argv[1] << endl;
	cout << "      float: " << setprecision(f_prec) << setw(width) << f << " " << components(vf) << endl;
	cout << "     double: " << setprecision(d_prec) << setw(width) << d << " " << components(vd) << endl;
	cout << "long double: " << setprecision(q_prec) << setw(width) << q << " " << components(vq) << endl;
	return EXIT_SUCCESS;
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
