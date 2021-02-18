// bfloats.cpp: create detailed component tables that decompose the components that comprise a bfloat
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
#include <universal/number/bfloat/bfloat>
#include <universal/number/bfloat/table.hpp>

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
	cout << "Generate value tables for bfloat configurations" << endl;

#if MANUAL_TESTING

	constexpr bool csv = false;

	GenerateTable< bfloat<3, 1, uint8_t> >(cout, csv);
	GenerateTable< bfloat<4, 1, uint8_t> >(cout, csv);
	GenerateTable< bfloat<5, 1, uint8_t> >(cout, csv);
	GenerateTable< bfloat<5, 2, uint8_t> >(cout, csv);
//	GenerateTable< bfloat<6, 1, uint8_t> >(cout, csv);
//	GenerateTable< bfloat<6, 2, uint8_t> >(cout, csv);
//	GenerateTable< bfloat<6, 3, uint8_t> >(cout, csv);

//	GenerateTable< bfloat<8, 3, uint8_t> >(cout, csv);

	/*
	* #include <fstream>
	ofstream ostr;
	ostr.open("bfloat_8_1.csv");
	GenerateTable< bfloat<8, 1> >(ostr, true);
	ostr.close();
	ostr.open("bfloat_8_2.csv");
	GenerateTable< bfloat<8, 2> >(ostr, true);
	ostr.close();
	ostr.open("bfloat_8_3.csv");
	GenerateTable< bfloat<8, 3> >(ostr, true);
	ostr.close();
	*/

#else // !MANUAL_TESTING
	GenerateTable< bfloat<4, 1, uint8_t> >(cout);

	GenerateTable< bfloat<5, 1, uint8_t> >(cout);
	GenerateTable< bfloat<5, 2, uint8_t> >(cout);
//	GenerateTable< bfloat<5, 3, uint8_t> >(cout);

	GenerateTable< bfloat<6, 1, uint8_t> >(cout);
	GenerateTable< bfloat<6, 2, uint8_t> >(cout);
	GenerateTable< bfloat<6, 3, uint8_t> >(cout);
//	GenerateTable< bfloat<6, 4, uint8_t> >(cout);

	GenerateTable< bfloat<7, 1, uint8_t> >(cout);
	GenerateTable< bfloat<7, 2, uint8_t> >(cout);
	GenerateTable< bfloat<7, 3, uint8_t> >(cout);
	GenerateTable< bfloat<7, 4, uint8_t> >(cout);
//	GenerateTable< bfloat<7, 5, uint8_t> >(cout);

	GenerateTable< bfloat<8, 1, uint8_t> >(cout);
	GenerateTable< bfloat<8, 2, uint8_t> >(cout);
	GenerateTable< bfloat<8, 3, uint8_t> >(cout);
	GenerateTable< bfloat<8, 4, uint8_t> >(cout);
	GenerateTable< bfloat<8, 5, uint8_t> >(cout);
//	GenerateTable< bfloat<8, 6, uint8_t> >(cout);
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
