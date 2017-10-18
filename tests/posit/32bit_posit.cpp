// 32bit_posit.cpp: Functionality tests for standard 32-bit posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"
#include "../../posit/posit_manipulators.hpp"

using namespace std;

/*
Standard posits with nbits = 32 have 2 exponent bits.
*/

bool ValidateAddition() {
	const int NR_TEST_CASES = 256;
	bool bValid = true;
	posit<32, 2> pa, pb, psum, pref;

	double input_values[NR_TEST_CASES];
	for (int i = 0; i < NR_TEST_CASES; i++) {
		pref.set_raw_bits(i);
		input_values[i] = pref.to_double();
	}

	float fa, fb;
	for (int i = 0; i < NR_TEST_CASES; i++) {
		fa = input_values[i];
		pa = fa;
		for (int j = 0; j < NR_TEST_CASES; j++) {
			fb = input_values[j];
			pb = fb;
			psum = pa + pb;
			pref = fa + fb;
			if (fabs(psum.to_double() - pref.to_double()) > 0.0001) {
				ReportBinaryArithmeticError("Posit<32,2> addition failed: ", "+", pa, pb, pref, psum);
				bValid = false;
			}
		}
	}
	return bValid;
}

int main(int argc, char** argv)
{
	cout << "Standard posit<32,2> configuration tests" << endl;
	try {
        const size_t nbits = 32;
        const size_t es = 2;
        posit<nbits,es> p;

		cout << spec_to_string(p) << endl;
	}
	catch (char* e) {
		cerr << e << endl;
	}

	return 0;
}
