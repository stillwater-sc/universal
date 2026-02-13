//  exponents.cpp : examples of working with posit exponents
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/posit1/posit1.hpp>

// examples of how regime and exponent are related to the scale of a posit
int main(int argc, char** argv)
try {
	using namespace sw::universal;

	// generate individual testcases to hand trace/debug
	std::cout << "Manual Exponent tests\n";
	constexpr unsigned nbits = 6;
	constexpr unsigned es = 2;
	posit<nbits, es> p; // for calculate_k method
	positRegime<nbits, es> r;
	positExponent<nbits, es> e;

	// create a two column table of regime and exponent bits
	std::cout << "regime  exponent\n";
	for (int scale = minpos_scale<nbits,es>()-1; scale < maxpos_scale<nbits,es>()+1; ++scale) {
		p = std::pow(2.0, scale);
		unsigned nrRegimeBits = r.assign_regime_pattern(decode_regime(p.get()));
		e.extract_exponent_bits(p.get(), nrRegimeBits);
		std::cout << r << ' ' << e << '\n';
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
