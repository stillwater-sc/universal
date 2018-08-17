// tables.cpp: create detailed component tables that spell out all the components that make up a posit
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "common.hpp"
#include <posit>

/*
  Posit values are a combination of 
  - a scaling factor: useed, 
  - an exponent: e, and 
  - a fraction: f.
  For small posits, it is faster to have a lookup mechanism to obtain the value.
  This is most valuable for conversion operators from posit to int.

  TODO: create a single lookup table for any posit configuration with 16 or fewer bits.
*/


int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	//int nrOfFailedTestCases = 0;
	bool csv = false;

	cout << "Generate posit configurations" << endl;

	// TODO: need to re-enable nbits = 2
	//GeneratePositTable<2, 0>(cout, csv);

	GeneratePositTable<3, 0>(cout, csv);
	//GeneratePositTable<3, 1>(cout, csv);

	GeneratePositTable<4, 0>(cout, csv);
	GeneratePositTable<4, 1>(cout, csv);

	GeneratePositTable<5, 0>(cout, csv);
	GeneratePositTable<5, 1>(cout, csv);
	GeneratePositTable<5, 2>(cout, csv);

	GeneratePositTable<6, 0>(cout, csv);
	GeneratePositTable<6, 1>(cout, csv);
	GeneratePositTable<6, 2>(cout, csv);
	GeneratePositTable<6, 3>(cout, csv);

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
