// components.cpp : tests for regime/exponent/fraction components of a posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"

using namespace std;

int main()
{
	const size_t nbits = 8;
	const size_t es = 2;


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
		int k = scale >> es;
		unsigned int nrOfRegimeBits = test_regime.assign_regime_pattern(k);
		unsigned int nrOfExponentBits = test_exponent.assign_exponent_bits(scale, nrOfRegimeBits);
		cout << "scale " << setw(4) << scale << " k " << setw(2) << k << " " << test_regime << " " << test_exponent << endl;
	}

	// fraction component of the posit
	std::bitset<nbits> _fraction;
	_fraction.set(nbits - 1, true);
	_fraction.set(nbits - 2, false);
	_fraction.set(nbits - 3, true);
	fraction<nbits, es> test_fraction;
	unsigned int nrOfFractionBits = 3;
	test_fraction.set(_fraction, nrOfFractionBits);	
	for (int scale = -bound; scale < bound; scale++) {
		int k = scale >> es;
		unsigned int nrOfRegimeBits = test_regime.assign_regime_pattern(k);
		unsigned int nrOfExponentBits = test_exponent.assign_exponent_bits(scale, nrOfRegimeBits);

		cout << "scale " << setw(4) << scale << " k " << setw(2) << k << " " << test_regime << " " << test_exponent << " " << test_fraction << endl;
	}
	return 0;
}



