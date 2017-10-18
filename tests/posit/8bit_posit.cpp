// 8bit_posit.cpp: Functionality tests for standard 8-bit posits
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
Standard posits with nbits = 8 have no exponent bits.
*/

bool ValidateAddition() {
	const int NR_TEST_CASES = 256;
	bool bValid = true;
	posit<8, 0> pa, pb, psum, pref;	
	
	float input_values[NR_TEST_CASES];
	for (int i = 0; i < NR_TEST_CASES; i++) {
		pref.set_raw_bits(i);
		input_values[i] = float(pref.to_double());
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
				ReportBinaryArithmeticError("Posit<8,0> addition failed: ", "+", pa, pb, pref, psum);
				bValid = false;
			}
		}
	}
	return bValid;
}

void TestPositArithmeticOperators(bool bValid, string posit_cfg, string op)
{
	if (!bValid) {
		cout << posit_cfg << " " << op << " FAIL" << endl;
	}
	else {
		cout << posit_cfg << " " << op << " PASS" << endl;
	}
}

int main(int argc, char** argv)
{
	cout << "Standard posit<8,0> configuration tests" << endl;
	try {
        const size_t nbits = 8;
        const size_t es = 0;
        posit<nbits,es> p;

		cout << spec_to_string(p) << endl;

		TestPositArithmeticOperators (ValidateAddition(), "posit<8,0>", "addition") ;
	}
	catch (char* e) {
		cerr << e << endl;
	}

	return 0;
}
