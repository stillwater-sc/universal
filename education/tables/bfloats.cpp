// bfloats.cpp: create detailed component tables that decompose the components that comprise a bfloat
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
#define BFLOAT_ROUNDING_ERROR_FREE_IO_FORMAT 1
// if you want to trace conversion: NOTE: tracing will destroy the constexpr-ness of operator=()
#define TRACE_CONVERSION 0
#include <universal/number/bfloat/bfloat>
#include <universal/number/bfloat/table.hpp>

#define MANUAL_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	// Usage: tables_bfloats [-csv]
	bool csv = false;
	if (argc == 2) {
		if (std::string(argv[1]) == std::string("-csv")) csv = true;
	}
	cout << "Generate value tables for bfloat configurations" << endl;

#if MANUAL_TESTING

	GenerateTable< bfloat<8, 3, uint8_t> >(cout, csv);

	/*
	* #include <fstream>
	ofstream ostr;
	ostr.open("bfloat_8_1.csv");
	GenerateTable< bfloat<8, 1, uint8_t> >(cout, true);
	ostr.close();
	*/

#else // !MANUAL_TESTING
	GenerateTable< bfloat<3, 1, uint8_t> >(cout, csv);

	GenerateTable< bfloat<4, 1, uint8_t> >(cout, csv);
	GenerateTable< bfloat<4, 2, uint8_t> >(cout, csv);

	GenerateTable< bfloat<5, 1, uint8_t> >(cout, csv);
	GenerateTable< bfloat<5, 2, uint8_t> >(cout, csv);
	GenerateTable< bfloat<5, 3, uint8_t> >(cout, csv);

	GenerateTable< bfloat<6, 1, uint8_t> >(cout, csv);
	GenerateTable< bfloat<6, 2, uint8_t> >(cout, csv);
	GenerateTable< bfloat<6, 3, uint8_t> >(cout, csv);
	GenerateTable< bfloat<6, 4, uint8_t> >(cout, csv);

	GenerateTable< bfloat<7, 1, uint8_t> >(cout, csv);
	GenerateTable< bfloat<7, 2, uint8_t> >(cout, csv);
	GenerateTable< bfloat<7, 3, uint8_t> >(cout, csv);
	GenerateTable< bfloat<7, 4, uint8_t> >(cout, csv);
	GenerateTable< bfloat<7, 5, uint8_t> >(cout, csv);

	GenerateTable< bfloat<8, 1, uint8_t> >(cout, csv);
	GenerateTable< bfloat<8, 2, uint8_t> >(cout, csv);
	GenerateTable< bfloat<8, 3, uint8_t> >(cout, csv);
	GenerateTable< bfloat<8, 4, uint8_t> >(cout, csv);
	GenerateTable< bfloat<8, 5, uint8_t> >(cout, csv);
	GenerateTable< bfloat<8, 6, uint8_t> >(cout, csv);
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
