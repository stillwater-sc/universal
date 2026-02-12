// components.cpp : examples working with regime/exponent/fraction components of a posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/posit1/posit1.hpp>

// Examples of working with the core components that make up a posit.
// These examples show the dynamic behavior of the different segments.
// These examples show the internal workings of the posit class and 
// thus are intended for library developers and posit enthusiasts.

template<unsigned nbits, unsigned es>
void EnumeratePositComponentsAcrossTheirScale() {
	using namespace sw::universal;

	std::cout << "Enumerating posit components across the dynamic range of the posit<" << nbits << "," << es << ">\n";

	// calculate the dynamic range of this posit configuration
	int k_max = nbits - 2;
	int bound = (k_max << es);

	// regime component of the posit
	std::cout << "REGIME\n";
	positRegime<nbits, es> test_regime;
	for (int scale = -bound; scale < bound; scale++) {
		int k = scale >> es;
		test_regime.assign_regime_pattern(k);
		std::cout << "scale of input number: " << std::setw(4) << scale << " regime attributes: k " << std::setw(2) << k << " " << test_regime.get() << " scale " << test_regime.scale() << '\n';
	}
	std::cout << '\n';

	// exponent component of the posit
	std::cout << "EXPONENT\n";
	positExponent<nbits, es> test_exponent;
	for (int scale = -bound; scale < bound; scale++) {
		int k = calculate_k<nbits, es>(scale);
		unsigned nrOfRegimeBits = test_regime.assign_regime_pattern(k);
		test_exponent.assign_exponent_bits(scale, k, nrOfRegimeBits);
		std::cout << "scale of input number: " << std::setw(4) << scale << " exponent bits: " << test_exponent << '\n';
	}
	std::cout << '\n';

	// fraction component of the posit
	std::cout << "FRACTION\n";
	constexpr unsigned fbits = (es + 2 >= nbits ? 0 : nbits - 3 - es);
	bitblock<fbits> _fraction;
	positFraction<fbits> test_fraction;
	test_fraction.set(_fraction, fbits);
	for (int scale = -bound; scale < bound; scale++) {
		int k = calculate_k<nbits, es>(scale);
		unsigned nrOfRegimeBits = test_regime.assign_regime_pattern(k);
		test_exponent.assign_exponent_bits(scale, k, nrOfRegimeBits);
		std::cout << "scale of input number: " << std::setw(4) << scale << " fraction bits: " << test_fraction << '\n';
	}
	std::cout << '\n';
}

int main()
try {
	using namespace sw::universal;

	EnumeratePositComponentsAcrossTheirScale<4, 0>();
	EnumeratePositComponentsAcrossTheirScale<4, 1>();
	EnumeratePositComponentsAcrossTheirScale<4, 2>();
	EnumeratePositComponentsAcrossTheirScale<4, 3>();
	EnumeratePositComponentsAcrossTheirScale<4, 4>();
	EnumeratePositComponentsAcrossTheirScale<4, 5>();

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}



