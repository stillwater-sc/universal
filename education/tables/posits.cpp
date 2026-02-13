// posits.cpp: create detailed component tables that decompose the components that comprise a posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <fstream>
#include <iostream>
#include <iomanip>
// enable/disable special posit format I/O
#define POSIT_ERROR_FREE_IO_FORMAT 1
#include <universal/number/posit1/posit1.hpp>
#include <universal/number/posit1/table.hpp>

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	// Usage: posits [-csv]
	bool csv = false;
	if (argc == 2) {
		if (std::string(argv[1]) == std::string("-csv")) csv = true;
	}
	std::cout << "Generate value tables for posit configurations\n";

#if MANUAL_TESTING

	GeneratePositTable<4, 1>(std::cout, csv);

	GeneratePositTable<5, 1>(std::cout, csv);
	GeneratePositTable<5, 2>(std::cout, csv);

	GeneratePositTable<6, 0>(std::cout, csv);
	GeneratePositTable<6, 1>(std::cout, csv);
	GeneratePositTable<6, 2>(std::cout, csv);

#else // !MANUAL_TESTING

	std::ofstream ostr;
	std::string filename, extension;
	extension = (csv ? ".csv" : ".txt");
	filename = std::string("posit") + extension;
	ostr.open(filename);

	GeneratePositTable<2, 0>(ostr, csv);

	GeneratePositTable<3, 0>(ostr, csv);
	GeneratePositTable<3, 1>(ostr, csv);
	GeneratePositTable<3, 2>(ostr, csv);
	GeneratePositTable<3, 3>(ostr, csv);
	GeneratePositTable<3, 4>(ostr, csv);
	GeneratePositTable<3, 5>(ostr, csv);

	GeneratePositTable<4, 0>(ostr, csv);
	GeneratePositTable<4, 1>(ostr, csv);
	GeneratePositTable<4, 2>(ostr, csv);
	GeneratePositTable<4, 3>(ostr, csv);
	GeneratePositTable<4, 4>(ostr, csv);
	GeneratePositTable<4, 5>(ostr, csv);
	GeneratePositTable<4, 6>(ostr, csv);

	GeneratePositTable<5, 0>(ostr, csv);
	GeneratePositTable<5, 1>(ostr, csv);
	GeneratePositTable<5, 2>(ostr, csv);
	GeneratePositTable<5, 3>(ostr, csv);
	GeneratePositTable<5, 4>(ostr, csv);
	GeneratePositTable<5, 5>(ostr, csv);

	GeneratePositTable<6, 0>(ostr, csv);
	GeneratePositTable<6, 1>(ostr, csv);
	GeneratePositTable<6, 2>(ostr, csv);
	GeneratePositTable<6, 3>(ostr, csv);
	GeneratePositTable<6, 4>(ostr, csv);
	GeneratePositTable<6, 5>(ostr, csv);
	GeneratePositTable<6, 6>(ostr, csv);

	GeneratePositTable<7, 0>(ostr, csv);
	GeneratePositTable<7, 1>(ostr, csv);
	GeneratePositTable<7, 2>(ostr, csv);
	GeneratePositTable<7, 3>(ostr, csv);
	GeneratePositTable<7, 4>(ostr, csv);

	GeneratePositTable<8, 0>(ostr, csv);
	GeneratePositTable<8, 1>(ostr, csv);
	GeneratePositTable<8, 2>(ostr, csv);
	GeneratePositTable<8, 3>(ostr, csv);
	GeneratePositTable<8, 4>(ostr, csv);
	GeneratePositTable<8, 5>(ostr, csv);

	ostr.close();
	std::cout << "Created value tables for posit<nbits, es> in " << filename << '\n';

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
