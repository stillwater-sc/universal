// lindex.cpp: generates encoding tables of level-index configurations (#1349 part 6, #1697)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// One row per encoding of lindex<nbits, lbits>: the pattern, the sign, the level and index
// of |value|, psi = level + index, and the value.  The value column is where level-index
// shows itself: at lindex<8,3> the 16 encodings of level 0 are evenly spaced fractions,
// level 4 starts at phi(4) = 3814279.1 and passes DBL_MAX at psi = 4.632, and levels 5 to 7
// lie beyond any IEEE type, so from there on the value is written as phi(psi).
//
// The configurations are the ones slindex (#1350) will emit, so the two files line up row
// for row, the way takums.cpp and takum_logs.cpp do (#1348).
#include <universal/utility/directives.hpp>
#include <fstream>
#include <iostream>
#include <iomanip>

#include <universal/number/lindex/lindex.hpp>
#include <universal/number/lindex/table.hpp>

#define MANUAL_TESTING 0

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	// Usage: edu_tables_lindex [-csv]
	bool csv = false;
	if (argc == 2) {
		std::string arg(argv[1]);
		if (arg == "-csv") {
			csv = true;
		}
		else {
			std::cerr << "Usage: " << argv[0] << " [-csv]\n";
			return EXIT_FAILURE;
		}
	}
	else if (argc > 2) {
		std::cerr << "Usage: " << argv[0] << " [-csv]\n";
		return EXIT_FAILURE;
	}
	std::cout << "Generate value tables for level-index number system configurations\n";

#if MANUAL_TESTING

	GenerateLindexTable<6, 3>(std::cout, csv);
	GenerateLindexTable<8, 3>(std::cout, csv);

#else // !MANUAL_TESTING

	std::ofstream ostr;
	std::string filename, extension;
	extension = (csv ? ".csv" : ".txt");
	filename = std::string("lindex") + extension;
	ostr.open(filename);
	if (!ostr) {
		std::cerr << "Error: could not open " << filename << " for writing\n";
		return EXIT_FAILURE;
	}

	// The standard level width, lbits = 3: levels 0 through 7, closed under + - * /
	GenerateLindexTable<6, 3>(ostr, csv);
	GenerateLindexTable<7, 3>(ostr, csv);
	GenerateLindexTable<8, 3>(ostr, csv);
	GenerateLindexTable<10, 3>(ostr, csv);
	GenerateLindexTable<12, 3>(ostr, csv);

	// Exploration: varying the level width at 8 bits
	// lbits = 2: levels 0..3, more index bits, but not closed: products past level 3 saturate
	GenerateLindexTable<8, 2>(ostr, csv);
	// lbits = 4: levels 0..15, far past any notation, with only 3 index bits
	GenerateLindexTable<8, 4>(ostr, csv);

	ostr.close();
	if (!ostr) {
		std::cerr << "Error: could not write " << filename << '\n';
		return EXIT_FAILURE;
	}
	std::cout << "Created value tables for lindex<nbits, lbits> in " << filename << '\n';

#endif

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Uncaught universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Uncaught universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::exception& err) {
	std::cerr << "Uncaught exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
