// components.cpp : examples working with regime/exponent/fraction components of a posit
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/posit/posit>

// Examples of working with the core components that make up a posit.
// These examples show the dynamic behavior of the different segments.
// These examples show the internal workings of the posit class and 
// thus are intended for library developers and posit enthusiasts.

template<size_t nbits, size_t es>
void EnumeratePositComponentsAcrossTheirScale() {
	using namespace std;
	using namespace sw::universal;

	cout << "Enumerating posit components across the dynamic range of the posit<" << nbits << "," << es << ">\n";

	// calculate the dynamic range of this posit configuration
	int k_max = nbits - 2;
	int bound = (k_max << es);

	// regime component of the posit
	cout << "REGIME\n";
	regime<nbits, es> test_regime;
	for (int scale = -bound; scale < bound; scale++) {
		int k = scale >> es;
		test_regime.assign_regime_pattern(k);
		cout << "scale of input number: " << setw(4) << scale << " regime attributes: k " << setw(2) << k << " " << test_regime.get() << " scale " << test_regime.scale() << '\n';
	}
	cout << endl;

	// exponent component of the posit
	cout << "EXPONENT\n";
	exponent<nbits, es> test_exponent;
	for (int scale = -bound; scale < bound; scale++) {
		int k = calculate_k<nbits, es>(scale);
		size_t nrOfRegimeBits = test_regime.assign_regime_pattern(k);
		test_exponent.assign_exponent_bits(scale, k, nrOfRegimeBits);
		cout << "scale of input number: " << setw(4) << scale << " exponent bits: " << test_exponent << '\n';
	}
	cout << endl;

	// fraction component of the posit
	cout << "FRACTION\n";
	constexpr size_t fbits = (es + 2 >= nbits ? 0 : nbits - 3 - es);
	bitblock<fbits> _fraction;
	fraction<fbits> test_fraction;
	test_fraction.set(_fraction, fbits);
	for (int scale = -bound; scale < bound; scale++) {
		int k = calculate_k<nbits, es>(scale);;
		size_t nrOfRegimeBits = test_regime.assign_regime_pattern(k);
		test_exponent.assign_exponent_bits(scale, k, nrOfRegimeBits);
		cout << "scale of input number: " << setw(4) << scale << " fraction bits: " << test_fraction << '\n';
	}
	cout << endl;
}

int main()
try {
	using namespace std;
	using namespace sw::universal;

	EnumeratePositComponentsAcrossTheirScale<4, 5>();
	return 0;

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



