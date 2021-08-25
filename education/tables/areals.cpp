// areals.cpp: create detailed component tables that decompose the components that comprise a areal
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#if defined(_MSC_VER)
#pragma warning(disable : 4514)  // unreferenced function is removed
#pragma warning(disable : 4710)  // function is not inlined
#pragma warning(disable : 5045)  // compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
#endif 
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

	/*
	* #include <fstream>
	ofstream ostr;
	ostr.open("areal_8_1.csv");
	GenerateArealTable<8, 1>(ostr, false, true);
	ostr.close();
	ostr.open("areal_8_2.csv");
	GenerateArealTable<8, 2>(ostr, false, true);
	ostr.close();
	ostr.open("areal_8_3.csv");
	GenerateArealTable<8, 3>(ostr, false, true);
	ostr.close();
	*/

#else // !MANUAL_TESTING
	GenerateArealTable<4, 1>(std::cout, ubit, csv);

	GenerateArealTable<5, 1>(std::cout, ubit, csv);
	GenerateArealTable<5, 2>(std::cout, ubit, csv);

	GenerateArealTable<6, 1>(std::cout, ubit, csv);
	GenerateArealTable<6, 2>(std::cout, ubit, csv);
	GenerateArealTable<6, 3>(std::cout, ubit, csv);

	GenerateArealTable<7, 1>(std::cout, ubit, csv);
	GenerateArealTable<7, 2>(std::cout, ubit, csv);
	GenerateArealTable<7, 3>(std::cout, ubit, csv);
	GenerateArealTable<7, 4>(std::cout, ubit, csv);

	GenerateArealTable<8, 1>(std::cout, ubit, csv);
	GenerateArealTable<8, 2>(std::cout, ubit, csv);
	GenerateArealTable<8, 3>(std::cout, ubit, csv);
	GenerateArealTable<8, 4>(std::cout, ubit, csv);
	GenerateArealTable<8, 5>(std::cout, ubit, csv);

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
