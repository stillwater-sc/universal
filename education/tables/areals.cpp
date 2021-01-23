// areals.cpp: create detailed component tables that decompose the components that comprise a areal
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#if defined(_MSC_VER)
#pragma warning(disable : 4514)  // unreferenced function is removed
#pragma warning(disable : 4710)  // function is not inlined
#endif 
#include <iostream>
#include <iomanip>

// enable/disable special hex format I/O
#define AREAL_ROUNDING_ERROR_FREE_IO_FORMAT 1
#include <universal/areal/areal>
#include <universal/areal/table.hpp>

#define MANUAL_TESTING 1

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	if (argc > 1) {
		for (int i = 0; i < argc; ++i) {
			std::cout << argv[i] << ' ';
		}
		std::cout << std::endl;
	}
	cout << "Generate value tables for areal configurations" << endl;

#if MANUAL_TESTING
	areal<5, 2> a;
	a.set_raw_bits(0x08);
	cout << a << endl;

	constexpr bool csv = false;
	constexpr bool uncertainty = false;

	GenerateArealTable<5, 1>(cout, uncertainty, csv);
	GenerateArealTable<5, 2>(cout, uncertainty, csv);
	GenerateArealTable<6, 1>(cout, uncertainty, csv);
	GenerateArealTable<6, 2>(cout, uncertainty, csv);
	GenerateArealTable<6, 3>(cout, uncertainty, csv);

	GenerateArealTable<8, 3>(cout, uncertainty, csv);

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
	GenerateArealTable<4, 1>(cout);

	GenerateArealTable<5, 1>(cout);
	GenerateArealTable<5, 2>(cout);
//	GenerateArealTable<5, 3>(cout);

	GenerateArealTable<6, 1>(cout);
	GenerateArealTable<6, 2>(cout);
	GenerateArealTable<6, 3>(cout);
//	GenerateArealTable<6, 4>(cout);

	GenerateArealTable<7, 1>(cout);
	GenerateArealTable<7, 2>(cout);
	GenerateArealTable<7, 3>(cout);
	GenerateArealTable<7, 4>(cout);
//	GenerateArealTable<7, 5>(cout);

	GenerateArealTable<8, 1>(cout);
	GenerateArealTable<8, 2>(cout);
	GenerateArealTable<8, 3>(cout);
	GenerateArealTable<8, 4>(cout);
	GenerateArealTable<8, 5>(cout);
//	GenerateArealTable<8, 6>(cout);
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