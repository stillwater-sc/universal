// enumeration.cpp: examples of enumerating the posit state space
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//#include <universal/number/posit1/posit1.hpp>
#include <universal/number/posit/posit.hpp>

// example of enumerating the state space of a posit configuration
int main(int argc, char** argv)
try {
	using namespace sw::universal;

	std::cout << "State space enumeration\n";

	constexpr int nbits = 5;
	constexpr int es = 0;	

	// forward enumeration
	std::cout << "Increment-based ascention from 0 to NaR and back to 0\n";
	posit<nbits, es> p(0);
	for (unsigned i = 0; i <= (unsigned(1) << nbits); ++i) {
		std::cout << components(p++) << '\n';
	}
	// reverse enumeration from NaR to 0
	std::cout << "Decrement-based descention from NaR to 0 and back to NaR\n";
	p.setnar();
	for (long i = (unsigned(1) << nbits); i >= 0; --i) {
		std::cout << components(p--) << '\n';
	}
	std::cout << '\n';

	// same enumeration but with different reporting
	// forward enumeration
	std::cout << "Increment-based ascention from 0 to NaR and back to 0\n";
	p = 0;
	for (unsigned i = 0; i <= (unsigned(1) << nbits); ++i) {
		std::cout << pretty_print(p++) << '\n';
	}
	// reverse enumeration from NaR to 0
	std::cout << "Decrement-based descention from NaR to 0 and back to NaR\n";
	p.setnar();
	for (long i = (unsigned(1) << nbits); i >= 0; --i) {
		std::cout << pretty_print(p--) << '\n';
	}
	std::cout << '\n';

	// same enumeration but with different reporting
	// forward enumeration
	std::cout << "Increment-based ascention from 0 to NaR and back to 0\n";
	p = 0;
	for (unsigned i = 0; i <= (unsigned(1) << nbits); ++i) {
		std::cout << info_print(p++, nbits) << '\n';
	}
	// reverse enumeration from NaR to 0
	std::cout << "Decrement-based descention from NaR to 0 and back to NaR\n";
	p.setnar();
	for (long i = (unsigned(1) << nbits); i >= 0; --i) {
		std::cout << info_print(p--, nbits) << '\n';
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
