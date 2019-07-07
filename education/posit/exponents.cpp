//  exponents.cpp : examples of working with posit exponents
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/posit/posit>

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

	// create a two column table of regime and exponent bits
	cout << "regime  exponent\n";
	for (int scale = minpos_scale<nbits,es>()-1; scale < maxpos_scale<nbits,es>()+1; ++scale) {
		p = std::pow(2.0, scale);
		size_t nrRegimeBits = r.assign_regime_pattern(decode_regime(p.get()));
		e.extract_exponent_bits(p.get(), nrRegimeBits);
		cout << r << ' ' << e << endl;
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
