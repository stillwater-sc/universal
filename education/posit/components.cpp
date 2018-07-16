// components.cpp : examples working with regime/exponent/fraction components of a posit
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "common.hpp"
#include <posit>

// Examples of working with the core components that make up a posit.
// These examples show the dynamic behavior of the different segments.
// These examples show the internal workings of the posit class and 
// thus are intended for library developers and posit enthusiasts.
int main()
try {
	using namespace std;
	using namespace sw::unum;

	const size_t nbits = 8;
	const size_t es = 2;
	//const bool _sign = false; // positive regime

	posit<nbits, es> p;

	int k_max = nbits - 1;
	int bound = (k_max << es);
	//float upper_range = float(useed<nbits, es>());

	// regime component of the posit
	cout << "REGIME\n";
	regime<nbits, es> test_regime;
	for (int scale = -bound; scale < bound; scale++) {
		int k = scale >> es;
		test_regime.assign_regime_pattern(k);
		cout << "scale " << setw(4) << scale << " k " << setw(2) << k << " " << test_regime.get() << " scale " << test_regime.scale() << '\n';
	}
	cout << endl;
    
	// exponent component of the posit
	cout << "EXPONENT\n";
	exponent<nbits, es> test_exponent;
	for (int scale = -bound; scale < bound; scale++) {
		int k = calculate_k<nbits, es>(scale);
		size_t nrOfRegimeBits = test_regime.assign_regime_pattern(k);
		// assign_exponent() returns the rounding mode: not used anymore: TODO
		test_exponent.assign_exponent_bits(scale, k, nrOfRegimeBits);
		cout << "scale " << setw(4) << scale << " k " << setw(2) << k << " " << test_regime << " " << test_exponent << '\n';
	}
	cout << endl;

	// fraction component of the posit
	cout << "FRACTION\n";
	bitblock<nbits-2> _fraction;
	_fraction.set(nbits - 3, true);
	_fraction.set(nbits - 4, false);
	_fraction.set(nbits - 5, true);
	fraction<nbits-2> test_fraction;
	size_t nrOfFractionBits = 3;
	test_fraction.set(_fraction, nrOfFractionBits);	
	for (int scale = -bound; scale < bound; scale++) {
		int k = calculate_k<nbits, es>(scale);;
		size_t nrOfRegimeBits = test_regime.assign_regime_pattern(k);
		test_exponent.assign_exponent_bits(scale, k, nrOfRegimeBits);
		cout << "scale " << setw(4) << scale << " k " << setw(2) << k << " " << test_regime << " " << test_exponent << " " << test_fraction << '\n';
	}
	cout << endl;

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}



