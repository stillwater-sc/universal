// scales.cpp : tests to characterize scales of posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include "../../posit/posit.hpp"
#include "../../posit/posit_manipulators.hpp"

using namespace std;

constexpr unsigned int MAX_ES = 5;
constexpr unsigned int MAX_K = 10;
uint64_t GENERATED_SCALE_FACTORS[MAX_ES][MAX_K];

void generateScaleFactorLookupTable() {
	uint64_t useed, useed_power_k;
	for (int es = 0; es < MAX_ES; es++) {
		useed = two_to_the_power(two_to_the_power(es));
		useed_power_k = useed; 
		GENERATED_SCALE_FACTORS[es][0] = 1; // for k = 0
		for (int k = 1; k < MAX_K; k++) {
			useed_power_k *= useed;
			GENERATED_SCALE_FACTORS[es][k] = useed_power_k;
		}
	}
}

void printScaleFactors(uint64_t scale_factors[MAX_ES][MAX_K]) {
	cout << "      ";
	for (int k = 0; k < MAX_K; k++) {
		cout << "     k = " << k << "   ";
	}
	cout << endl;
	for (int es = 0; es < MAX_ES; es++) {
		cout << "es = " << es << " ";
		for (int k = 0; k < MAX_K; k++) {
			cout << setw(12) << scale_factors[es][k] << " ";
		}
		cout << endl;
	}
	cout << endl;
}

void ReportPositScales() {
	// print scales of different posit configurations
	// useed = 2^(2^es) and thus is just a function of the exponent configuration
	// maxpos = useed^(nbits-2)
	// minpos = useed^(2-nbits)
	posit<3, 0> p3_0;
	posit<4, 0> p4_0;
	posit<4, 1> p4_1;
	posit<5, 0> p5_0;
	posit<5, 1> p5_1;
	posit<5, 2> p5_2;
	posit<6, 0> p6_0;
	posit<6, 1> p6_1;
	posit<6, 2> p6_2;
	posit<6, 3> p6_3;
	posit<7, 0> p7_0;
	posit<7, 1> p7_1;
	posit<7, 2> p7_2;
	posit<7, 3> p7_3;
	posit<7, 4> p7_4;
	posit<8, 0> p8_0;
	posit<16, 1> p16_1;
	posit<32, 2> p32_2;
	posit<64, 3> p64_3;
	cout << "Posit specificiation examples and their ranges:" << endl;
	cout << "Small, specialized posit configurations" << endl;
	cout << "nbits = 3" << endl;
	cout << spec_to_string(p3_0) << endl;
	cout << "nbits = 4" << endl;
	cout << spec_to_string(p4_0) << endl;
	cout << spec_to_string(p4_1) << endl;
	cout << "nbits = 5" << endl;
	cout << spec_to_string(p5_0) << endl;
	cout << spec_to_string(p5_1) << endl;
	cout << spec_to_string(p5_2) << endl;
	cout << "nbits = 6" << endl;
	cout << spec_to_string(p6_0) << endl;
	cout << spec_to_string(p6_1) << endl;
	cout << spec_to_string(p6_2) << endl;
	cout << spec_to_string(p6_3) << endl;
	cout << "nbits = 7" << endl;
	cout << spec_to_string(p7_0) << endl;
	cout << spec_to_string(p7_1) << endl;
	cout << spec_to_string(p7_2) << endl;
	cout << spec_to_string(p7_3) << endl;
	cout << spec_to_string(p7_4) << endl;
	cout << "Standard posit configurations" << endl;
	cout << spec_to_string(p8_0) << endl;
	cout << spec_to_string(p16_1) << endl;
	cout << spec_to_string(p32_2) << endl;
	cout << spec_to_string(p64_3) << endl;
	cout << endl;
}

int main()
{
	ReportPositScales();

	/*
	   it is easier to work with scales than with absolute values
	generateScaleFactorLookupTable();
	printScaleFactors(GENERATED_SCALE_FACTORS);
	*/
    
	return 0;
}



