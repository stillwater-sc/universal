// areals.cpp: create detailed component tables that decompose the components that comprise a areal
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// enable/disable special hex format I/O
#define AREAL_ROUNDING_ERROR_FREE_IO_FORMAT 1
#include <universal/areal/areal>
#include <universal/areal/table.hpp>

#define MANUAL_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	bool csv = false;

	cout << "Generate value tables for areal configurations" << endl;

#if MANUAL_TESTING
	areal<5, 2> a;
	a.set_raw_bits(0x08);
	cout << a << endl;

	GenerateArealTable<5, 1>(cout, csv);
	GenerateArealTable<5, 2>(cout, csv);
	GenerateArealTable<6, 1>(cout, csv);
	GenerateArealTable<6, 2>(cout, csv);
	GenerateArealTable<6, 3>(cout, csv);

#else // !MANUAL_TESTING
	GenerateArealTable<4, 1>(cout, csv);

	GenerateArealTable<5, 1>(cout, csv);
	GenerateArealTable<5, 2>(cout, csv);
//	GenerateArealTable<5, 3>(cout, csv);

	GenerateArealTable<6, 1>(cout, csv);
	GenerateArealTable<6, 2>(cout, csv);
	GenerateArealTable<6, 3>(cout, csv);
//	GenerateArealTable<6, 4>(cout, csv);

	GenerateArealTable<7, 1>(cout, csv);
	GenerateArealTable<7, 2>(cout, csv);
	GenerateArealTable<7, 3>(cout, csv);
	GenerateArealTable<7, 4>(cout, csv);
//	GenerateArealTable<7, 5>(cout, csv);

	GenerateArealTable<8, 1>(cout, csv);
	GenerateArealTable<8, 2>(cout, csv);
	GenerateArealTable<8, 3>(cout, csv);
	GenerateArealTable<8, 4>(cout, csv);
	GenerateArealTable<8, 5>(cout, csv);
//	GenerateArealTable<8, 6>(cout, csv);
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
