// reciprocal_tables.cpp: create lookup tables for reciprocal and division of small posits up to 16 bits.
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "common.hpp"

#include "../../posit/posit.hpp"
#include "../../posit/posit_manipulators.hpp"

/*
  Generator of a single lookup table for reciprocals of any posit configuration with 16 or fewer bits.
*/
template<size_t nbits, size_t es>
void GeneratePositReciprocalLookupTable(std::ostream& os) {
	const size_t NR_OF_ENTRIES = size_t(1) << nbits;

	sw::unum::posit<nbits, es> p, r;
	double v,rv;
	for (size_t i = 0; i < NR_OF_ENTRIES; i++) {
		p.set_raw_bits(i);
		v = double(p);
		rv = 1.0 / v;
		r = rv;
		// os << p << " reciprocal of " << v << " is " << rv << " : " << r << std::endl;
		os << i << " " << p << " " << r << std::endl;
	}
}

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	int nrOfFailedTestCases = 0;

	cout << "Generate posit reciprocal lookup table" << endl;

	GeneratePositReciprocalLookupTable<3, 0>(cout);
	//GeneratePositReciprocalLookupTable<4, 0>(cout);
	//GeneratePositReciprocalLookupTable<4, 1>(cout);

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
