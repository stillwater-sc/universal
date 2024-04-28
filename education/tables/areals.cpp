// areals.cpp: create detailed component tables that decompose the components that comprise a areal
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <fstream>
#include <iostream>
#include <iomanip>
// enable/disable special hex format I/O
#define AREAL_ROUNDING_ERROR_FREE_IO_FORMAT 1
#include <universal/number/areal/areal.hpp>
#include <universal/number/areal/table.hpp>

#define MANUAL_TESTING 0

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	// Usage: areals [-ubit] [-csv]
	bool csv = false;
	bool ubit = false;
	if (argc == 2) {
		if (std::string(argv[1]) == std::string("-ubit")) ubit = true;
		if (std::string(argv[1]) == std::string("-csv")) csv = true;
	}
	else if (argc == 3) {
		if (std::string(argv[1]) == std::string("-ubit")) ubit = true;
		if (std::string(argv[1]) == std::string("-csv")) csv = true;
		if (std::string(argv[2]) == std::string("-ubit")) ubit = true;
		if (std::string(argv[2]) == std::string("-csv")) csv = true;
	}
	std::cout << "Generate value tables for areal configurations\n";

#if MANUAL_TESTING

	GenerateArealTable<5, 1>(cout, ubit, csv);
	GenerateArealTable<5, 2>(cout, ubit, csv);
	GenerateArealTable<6, 1>(cout, ubit, csv);
	GenerateArealTable<6, 2>(cout, ubit, csv);
	GenerateArealTable<6, 3>(cout, ubit, csv);

	GenerateArealTable<8, 3>(cout, ubit, csv);

#else // !MANUAL_TESTING

	std::ofstream ostr;
	std::string filename, extension;
	extension = (csv ? ".csv" : ".txt");
	filename = std::string("areal") + extension;
	ostr.open(filename);

	GenerateArealTable<4, 1>(ostr, ubit, csv);

	GenerateArealTable<5, 1>(ostr, ubit, csv);
	GenerateArealTable<5, 2>(ostr, ubit, csv);

	GenerateArealTable<6, 1>(ostr, ubit, csv);
	GenerateArealTable<6, 2>(ostr, ubit, csv);
	GenerateArealTable<6, 3>(ostr, ubit, csv);

	GenerateArealTable<7, 1>(ostr, ubit, csv);
	GenerateArealTable<7, 2>(ostr, ubit, csv);
	GenerateArealTable<7, 3>(ostr, ubit, csv);
	GenerateArealTable<7, 4>(ostr, ubit, csv);

	GenerateArealTable<8, 1>(ostr, ubit, csv);
	GenerateArealTable<8, 2>(ostr, ubit, csv);
	GenerateArealTable<8, 3>(ostr, ubit, csv);
	GenerateArealTable<8, 4>(ostr, ubit, csv);
	GenerateArealTable<8, 5>(ostr, ubit, csv);

	ostr.close();
	std::cout << "Created value tables for areal<> in " << filename << '\n';

#endif

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (std::runtime_error& e) {
	std::cerr << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
