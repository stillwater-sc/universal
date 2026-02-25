// reciprocal_tables.cpp: create lookup tables for reciprocal and division of small posits up to 16 bits.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/posit1/posit1.hpp>

/*
  Generator of a single lookup table for reciprocals of any posit configuration with 16 or fewer bits.
*/
template<size_t nbits, size_t es>
void GeneratePositReciprocalLookupTable(std::ostream& os) {
	const size_t NR_OF_ENTRIES = size_t(1) << nbits;

	sw::universal::posit<nbits, es> p, r;
	double v,rv;
	for (size_t i = 0; i < NR_OF_ENTRIES; i++) {
		p.setbits(i);
		v = double(p);
		rv = 1.0 / v;
		r = rv;
		// os << p << " reciprocal of " << v << " is " << rv << " : " << r << std::endl;
		os << i << " " << p << " " << r << std::endl;
	}
}

int main()
try {
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;

	std::cout << "Generate posit reciprocal lookup table\n";

	GeneratePositReciprocalLookupTable<3, 0>(std::cout);
	//GeneratePositReciprocalLookupTable<4, 0>(std::cout);
	//GeneratePositReciprocalLookupTable<4, 1>(std::cout);

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
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
