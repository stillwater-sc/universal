// components.cpp : tests for regime/exponent/fraction components of a posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include "../../posit/posit.hpp"

using namespace std;
using namespace sw::unum;

int main()
try {
	const size_t nbits = 8;
	const size_t es = 2;
	const bool _sign = false; // positive regime
	int nrOfFailedTestCases = 0;

	posit<nbits, es> p;

	int k_max = nbits - 1;
	int bound = (k_max << es);
	float upper_range = float(useed<nbits, es>());

	// regime component of the posit
	regime<nbits, es> test_regime;
	for (int scale = -bound; scale < bound; scale++) {
		int k = scale >> es;
		test_regime.assign_regime_pattern(k);
		cout << "scale " << setw(4) << scale << " k " << setw(2) << k << " " << test_regime.get() << " scale " << test_regime.scale() <<  endl;
	}
    
	// exponent component of the posit
	exponent<nbits, es> test_exponent;
	for (int scale = -bound; scale < bound; scale++) {
		int k = calculate_k<nbits, es>(scale);
		unsigned int nrOfRegimeBits = test_regime.assign_regime_pattern(k);
		unsigned int nrOfExponentBits = test_exponent.assign_exponent_bits(scale, k, nrOfRegimeBits);
		cout << "scale " << setw(4) << scale << " k " << setw(2) << k << " " << test_regime << " " << test_exponent << endl;
	}

	// fraction component of the posit
	std::bitset<nbits-2> _fraction;
	_fraction.set(nbits - 3, true);
	_fraction.set(nbits - 4, false);
	_fraction.set(nbits - 5, true);
	fraction<nbits-2> test_fraction;
	unsigned int nrOfFractionBits = 3;
	test_fraction.set(_fraction, nrOfFractionBits);	
	for (int scale = -bound; scale < bound; scale++) {
		int k = calculate_k<nbits, es>(scale);;
		unsigned int nrOfRegimeBits = test_regime.assign_regime_pattern(k);
		unsigned int nrOfExponentBits = test_exponent.assign_exponent_bits(scale, k, nrOfRegimeBits);

		cout << "scale " << setw(4) << scale << " k " << setw(2) << k << " " << test_regime << " " << test_exponent << " " << test_fraction << endl;
	}
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}



