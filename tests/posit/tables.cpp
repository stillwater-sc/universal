// tables.cpp: create detailed component tables that spell out all the components that make up a posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"
#include <sstream>

#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"
#include "../../posit/posit_manipulators.hpp"

using namespace std;

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
	cout << "Valid posit configurations" << endl;

	posit<6, 2> p;
	p.set_raw_bits(0x25);
	exponent<6, 2> exponent = p.get_exponent();
	cout << p << " " << components_to_string(p) << " " << exponent << endl;

	GeneratePositTable<3, 0>(cout);

	GeneratePositTable<4, 0>(cout);		
	GeneratePositTable<4, 1>(cout);

	GeneratePositTable<5, 0>(cout);
	GeneratePositTable<5, 1>(cout);
	GeneratePositTable<5, 2>(cout);

	GeneratePositTable<6, 0>(cout);
	GeneratePositTable<6, 1>(cout);
	GeneratePositTable<6, 2>(cout);

	GeneratePositTable<7, 0>(cout);
	GeneratePositTable<7, 1>(cout);
	GeneratePositTable<7, 2>(cout);
	GeneratePositTable<7, 3>(cout);

	GeneratePositTable<8, 0>(cout);
	GeneratePositTable<8, 1>(cout);
	GeneratePositTable<8, 2>(cout);
	GeneratePositTable<8, 3>(cout);
	GeneratePositTable<8, 4>(cout);

	return 0;
}
catch (char* e) {
	cerr << e << endl;
	return 1;
}
