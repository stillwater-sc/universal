// posits.cpp: create detailed component tables that decompose the components that comprise a posit
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// enable/disable special posit format I/O
#define POSIT_ROUNDING_ERROR_FREE_IO_FORMAT 1
#include <universal/number/posit/posit>
#include <universal/number/posit/table.hpp>

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	bool csv = false;

	cout << "Generate posit configurations" << endl;

	// TODO: need to re-enable nbits = 2
	GeneratePositTable<2, 0>(cout, csv);

	GeneratePositTable<3, 0>(cout, csv);
	GeneratePositTable<3, 1>(cout, csv);
	GeneratePositTable<3, 2>(cout, csv);
	GeneratePositTable<3, 3>(cout, csv);
	GeneratePositTable<3, 4>(cout, csv);
	GeneratePositTable<3, 5>(cout, csv);

	GeneratePositTable<4, 0>(cout, csv);
	GeneratePositTable<4, 1>(cout, csv);
	GeneratePositTable<4, 2>(cout, csv);
	GeneratePositTable<4, 3>(cout, csv);
	GeneratePositTable<4, 4>(cout, csv);
	GeneratePositTable<4, 5>(cout, csv);
	GeneratePositTable<4, 6>(cout, csv);

	GeneratePositTable<5, 0>(cout, csv);
	GeneratePositTable<5, 1>(cout, csv);
	GeneratePositTable<5, 2>(cout, csv);
	GeneratePositTable<5, 3>(cout, csv);
	GeneratePositTable<5, 4>(cout, csv);
	GeneratePositTable<5, 5>(cout, csv);

	GeneratePositTable<6, 0>(cout, csv);
	GeneratePositTable<6, 1>(cout, csv);
	GeneratePositTable<6, 2>(cout, csv);
	GeneratePositTable<6, 3>(cout, csv);
	GeneratePositTable<6, 4>(cout, csv);
	GeneratePositTable<6, 5>(cout, csv);
	GeneratePositTable<6, 6>(cout, csv);

	GeneratePositTable<7, 0>(cout, csv);
	GeneratePositTable<7, 1>(cout, csv);
	GeneratePositTable<7, 2>(cout, csv);
	GeneratePositTable<7, 3>(cout, csv);
	GeneratePositTable<7, 4>(cout, csv);

	GeneratePositTable<8, 0>(cout, csv);
	GeneratePositTable<8, 1>(cout, csv);
	GeneratePositTable<8, 2>(cout, csv);
	GeneratePositTable<8, 3>(cout, csv);
	GeneratePositTable<8, 4>(cout, csv);
	GeneratePositTable<8, 5>(cout, csv);

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
