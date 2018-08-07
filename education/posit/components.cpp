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

template<size_t nbits, size_t es>
void EnumeratePositComponentsAcrossTheirScale() {
	using namespace std;
	using namespace sw::unum;

	static_assert(nbits > es + 2, "posit configuration must adhere to nbits > es + 2");
	if (nbits < es + 3) return;

	cout << "Enumerating the posit components across the dynamic range of the posit configuration\n";

	posit<nbits, es> p;

	// calculate the dynamic range of this posit configuration
	int k_max = nbits - 1;
	int bound = (k_max << es);

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
		test_exponent.assign_exponent_bits(scale, k, nrOfRegimeBits);
		cout << "scale " << setw(4) << scale << " k " << setw(2) << k << " " << test_regime << " " << test_exponent << '\n';
	}
	cout << endl;

	// fraction component of the posit
	cout << "FRACTION\n";
	constexpr size_t fbits = nbits - 3 - es;
	bitblock<fbits> _fraction;
	fraction<fbits> test_fraction;
	test_fraction.set(_fraction, fbits);
	for (int scale = -bound; scale < bound; scale++) {
		int k = calculate_k<nbits, es>(scale);;
		size_t nrOfRegimeBits = test_regime.assign_regime_pattern(k);
		test_exponent.assign_exponent_bits(scale, k, nrOfRegimeBits);
		cout << "scale " << setw(4) << scale << " k " << setw(2) << k << " " << test_regime << " " << test_exponent << " " << test_fraction << '\n';
	}
	cout << endl;
}

int main()
try {
	using namespace std;
	using namespace sw::unum;

	EnumeratePositComponentsAcrossTheirScale<4, 0>();
	EnumeratePositComponentsAcrossTheirScale<4, 1>();
	//EnumeratePositComponentsAcrossTheirScale<4, 2>();  TODO: to have an independent es, we need a different fraction size calculation

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}



