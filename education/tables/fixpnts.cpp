// fixpnts.cpp: generates encoding tables of fixed-point configurations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <fstream>
#include <iostream>
#include <iomanip>
// Configure the fixpnt template environment
// first: enable general or specialized fixed-point configurations
#define FIXPNT_FAST_SPECIALIZATION
// second: enable/disable fixpnt arithmetic exceptions
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/fixpnt/table.hpp>

#define MANUAL_TESTING 0

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	// Usage: fixpnts [-csv]
	bool csv = false;
	if (argc == 2) {
		if (std::string(argv[1]) == std::string("-csv")) csv = true;
	}
	std::cout << "Generate value tables for fixpnt configurations\n";

#if MANUAL_TESTING
	GenerateFixedPointTable<4, 2>(std::cout, csv);
	GenerateFixedPointTable<5, 3>(std::cout, csv);
	GenerateFixedPointTable<6, 3>(std::cout, csv);
	GenerateFixedPointTable<8, 4>(std::cout, csv);

#else // !MANUAL_TESTING

	std::ofstream ostr;
	std::string filename, extension;
	extension = (csv ? ".csv" : ".txt");
	filename = std::string("fixpnt") + extension;
	ostr.open(filename);

	GenerateFixedPointTable<4, 0>(ostr, csv);
	GenerateFixedPointTable<4, 1>(ostr, csv);
	GenerateFixedPointTable<4, 2>(ostr, csv);
	GenerateFixedPointTable<4, 3>(ostr, csv);
	GenerateFixedPointTable<4, 4>(ostr, csv);

	GenerateFixedPointTable<5, 0>(ostr, csv);
	GenerateFixedPointTable<5, 1>(ostr, csv);
	GenerateFixedPointTable<5, 2>(ostr, csv);
	GenerateFixedPointTable<5, 3>(ostr, csv);
	GenerateFixedPointTable<5, 4>(ostr, csv);
	GenerateFixedPointTable<5, 5>(ostr, csv);

	GenerateFixedPointTable<6, 0>(ostr, csv);
	GenerateFixedPointTable<6, 1>(ostr, csv);
	GenerateFixedPointTable<6, 2>(ostr, csv);
	GenerateFixedPointTable<6, 3>(ostr, csv);
	GenerateFixedPointTable<6, 4>(ostr, csv);
	GenerateFixedPointTable<6, 5>(ostr, csv);
	GenerateFixedPointTable<6, 6>(ostr, csv);

	GenerateFixedPointTable<8, 0>(ostr, csv);
	GenerateFixedPointTable<8, 1>(ostr, csv);
	GenerateFixedPointTable<8, 2>(ostr, csv);
	GenerateFixedPointTable<8, 3>(ostr, csv);
	GenerateFixedPointTable<8, 4>(ostr, csv);
	GenerateFixedPointTable<8, 5>(ostr, csv);
	GenerateFixedPointTable<8, 6>(ostr, csv);
	GenerateFixedPointTable<8, 7>(ostr, csv);
	GenerateFixedPointTable<8, 8>(ostr, csv);

	ostr.close();
	std::cout << "Created value tables for fixpnt<nbits, rbits> in " << filename << '\n';

#endif

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_arithmetic_exception& err) {
	std::cerr << "Uncaught fixpnt arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_internal_exception& err) {
	std::cerr << "Uncaught fixpnt internal exception: " << err.what() << std::endl;
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
