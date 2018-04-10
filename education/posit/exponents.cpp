//  exponents.cpp : examples of working with posit exponents
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "common.hpp"
#include <posit>

// examples of how regime and exponent are related to the scale of a posit
int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	// generate individual testcases to hand trace/debug
	cout << "Manual Exponent tests" << endl;
	constexpr size_t nbits = 6;
	constexpr size_t es = 2;
	posit<nbits, es> p; // for calculate_k method
	regime<nbits, es> r;
	exponent<nbits, es> e;
	for (int scale = -16; scale < 17; scale++) {
		int k = calculate_k<nbits, es>(scale);
		size_t regime_size = r.assign_regime_pattern(scale >> es);
		size_t exp_size = e.assign_exponent_bits(scale, k, regime_size);
		if (scale < 0) {
			cout << "in value = " << setw(12) << 1.0/(unsigned(1) << -scale) << " scale = " << setw(3) << scale << " r(" << r << ")  e(" << e << ")     projected value " << r.value() * e.value() << endl;
		}
		else {
			cout << "in value = " << setw(12) << (unsigned(1) << scale)      << " scale = " << setw(3) << scale << " r(" << r << ")  e(" << e << ")     projected value " << r.value() * e.value() << endl;
		}
	}

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
