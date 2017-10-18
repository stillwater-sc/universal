// add_subtract.cpp: functional tests for addition and subtraction
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include "../../bitset/bitset_helpers.hpp"
#include "../../posit/posit_regime_lookup.hpp"
#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"
#include "../../posit/posit_manipulators.hpp"

using namespace std;

// normalize creates a normalized number with the hidden bit installed: 1.bbbbbbbbb
template<size_t nbits>
void normalize(const std::bitset<nbits>& fraction, std::bitset<nbits>& number) {
	if (nbits == 3) return;
	number.set(nbits - 1); // set hidden bit
	int lb = nbits; lb -= 2;
	for (int i = lb; i >= 0; i--) {
		number.set(i, fraction[i + 1]);
	}
}
/*   h is hidden bit
*   h.bbbb_bbbb_bbbb_b...      fraction
*   0.000h_bbbb_bbbb_bbbb_b... number
*  >-.----<                    shift of 4
*/
template<size_t nbits>
void denormalize(const std::bitset<nbits>& fraction, int shift, std::bitset<nbits>& number) {
	if (nbits == 3) return;
	number.reset();
	if (shift <= nbits - 1) {
		number.set(nbits - 1 - shift); // set hidden bit
		for (int i = nbits - 2 - shift; i >= 0; i--) {
			number.set(i, fraction[i + 1 + shift]);
		}
	}
}

/*
	Testing the reciprocal nature of positive and negative posits
 */

/*
POSIT<4,0>
   #           Binary         Decoded         k-value            sign                        regime        exponent        fraction                         value
   0:             0000            0000               0               1                             1            -            ----                             0
   1:             0001            0001              -2               1                          0.25            -            ----                          0.25
   2:             0010            0010              -1               1                           0.5            -            0---                           0.5
   3:             0011            0011              -1               1                           0.5            -            1---                          0.75
   4:             0100            0100               0               1                             1            -            0---                             1
   5:             0101            0101               0               1                             1            -            1---                           1.5
   6:             0110            0110               1               1                             2            -            ----                             2
   7:             0111            0111               2               1                             4            -            ----                             4
   8:             1000            1000              -3              -1                           inf            -            ----                           inf
   9:             1001            1111               2              -1                             4            -            ----                            -4
  10:             1010            1110               1              -1                             2            -            ----                            -2
  11:             1011            1101               0              -1                             1            -            1---                          -1.5
  12:             1100            1100               0              -1                             1            -            0---                            -1
  13:             1101            1011              -1              -1                           0.5            -            1---                         -0.75
  14:             1110            1010              -1              -1                           0.5            -            0---                          -0.5
  15:             1111            1001              -2              -1                          0.25            -            ----                         -0.25
*/
bool ValidateAdditionPosit_4_0() {
	const int NR_TEST_CASES = 16;
	float input_values[NR_TEST_CASES] = {
		-4.0f, -2.0f, -1.5f, -1.0f, -0.75f, -0.5f, -0.25f, 0.0f, 0.25f, 0.5f, 0.75f, 1.0f, 1.5f, 2.0f, 4.0f, INFINITY
	};

	bool bValid = true;
	posit<4, 0> pa, pb, psum, pref;
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
				ReportBinaryArithmeticError("Posit<4,0> addition failed: ", "+", pa, pb, pref, psum);
				bValid = false;
			}
		}
	}
	return bValid;
}

bool ValidateSubtractionPosit_4_0() {
	const int NR_TEST_CASES = 16;
	float input_values[NR_TEST_CASES] = {
		-4.0f, -2.0f, -1.5f, -1.0f, -0.75f, -0.5f, -0.25f, 0.0f, 0.25f, 0.5f, 0.75f, 1.0f, 1.5f, 2.0f, 4.0f, INFINITY
	};

	bool bValid = true;
	posit<4, 0> pa, pb, pdif, pref;
	float fa, fb;
	for (int i = 0; i < NR_TEST_CASES; i++) {
		fa = input_values[i];
		pa = fa;
		for (int j = 0; j < NR_TEST_CASES; j++) {
			fb = input_values[j];
			pb = fb;
			pdif = pa - pb;
			pref = fa - fb;
			if (fabs(pdif.to_double() - pref.to_double()) > 0.0001) {
				ReportBinaryArithmeticError("Posit<4,0> subtraction failed: ", "-", pa, pb, pref, pdif);
				bValid = false;
			}
		}
	}
	return bValid;
}

bool ValidateNegationPosit_4_0() {
	const int NR_TEST_CASES = 16;
	float target_values[NR_TEST_CASES] = {
		-4.0, -2.0, -1.5, -1.0, -0.75, -0.5, -0.25, 0.0, 0.25, 0.5, 0.75, 1.0, 1.5, 2.0, 4.0, INFINITY
	};

	float golden_value[NR_TEST_CASES] = {
		+4.0, +2.0, +1.5, +1.0, +0.75, +0.5, +0.25, 0.0, -0.25, -0.5, -0.75, -1.0, -1.5, -2.0, -4.0, INFINITY
	};

	bool bValid = true;
	posit<4, 0> pa, pb, pref;
	for (int i = 0; i < NR_TEST_CASES; i++) {
		pa = target_values[i];
		pb = -pa;
		pref = golden_value[i];
		if (pb != pref) {
			ReportUnaryArithmeticError("Posit<4,0> negation failed: ", "-", pa, pref, pb);
			bValid = false;
		}
	}

	return bValid;
}

bool ValidateNegAdditionPosit_4_0() {
	float target_values[15] = {
		-4.0f, -2.0f, -1.5f, -1.0f, -0.75f, -0.5f, -0.25f, 0.0f, 0.25f, 0.5f, 0.75f, 1.0f, 1.5f, 2.0f, 4.0f
	};

	bool bValid = true;
	posit<4, 0> pa, pb, psum;
	for (int i = 0; i < 15; i++) {
		pa =  target_values[i];
		pb = -pa;
		psum = pa + pb;
		if (fabs(psum.to_double()) > 0.0001) {
			cerr << "Posit<4,0> negated add failed: " << pa << " + " << pb << " != 0 instead it yielded " << psum << " " << components_to_string(psum) << endl;
			bValid = false;
		}
	}

	pa = INFINITY;
	pb = -pa;
	psum = pa + pb;
	if (!psum.isInfinite()) {
		cerr << "Posit<4,0> negated add failed: " << pa << " + " << pb << " != inf instead it yielded " << psum << " " << components_to_string(psum) << endl;
		bValid = false;
	}
	return bValid;
}

int main(int argc, char** argv)
{
	posit<5, 0> pa, pb, pref, psum;
	float fa = -0.625f;
	float fb =  4.0f;
	pa = fa; 
	pb = fb;
	pref = fa + fb;
	psum = pa + pb;
	cout << pref << " " << psum << endl;
	ReportTestResult(ValidateAddition<5, 0>("Posit<5,0> addition failed: "), "posit<5,0>", "addition");

	return 0;

	ReportTestResult(ValidateAdditionPosit_4_0(), "posit<4,0>", "addition");
	ReportTestResult(ValidateNegationPosit_4_0(), "posit<4,0>", "negation");
	ReportTestResult(ValidateNegAdditionPosit_4_0(), "posit<4,0>", "neg addition");
	ReportTestResult(ValidateSubtractionPosit_4_0(), "posit<4,0>", "subtraction");

	ReportTestResult(ValidateAddition<5, 0>("Posit<5,0> addition failed: "), "posit<5,0>", "addition");
	ReportTestResult(ValidateAddition<5, 1>("Posit<5,1> addition failed: "), "posit<5,1>", "addition");
	ReportTestResult(ValidateAddition<5, 2>("Posit<5,2> addition failed: "), "posit<5,2>", "addition");

	ReportTestResult(ValidateAddition<6, 0>("Posit<6,0> addition failed: "), "posit<6,0>", "addition");
	ReportTestResult(ValidateAddition<6, 1>("Posit<6,1> addition failed: "), "posit<6,1>", "addition");
	ReportTestResult(ValidateAddition<6, 2>("Posit<6,2> addition failed: "), "posit<6,2>", "addition");
	ReportTestResult(ValidateAddition<6, 2>("Posit<6,3> addition failed: "), "posit<6,3>", "addition");

	return 0;
}
