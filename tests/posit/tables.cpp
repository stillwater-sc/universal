// tables.cpp: create detailed component tables that spell out all the components that make up a posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"
#include <sstream>
#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"

using namespace std;

/*
  Posit values are a combination of 
  - a scaling factor: useed, 
  - an exponent: e, and 
  - a fraction: f.
  For small posits, it is faster to have a lookup mechanism to obtain the value.
  This is most valuable for conversion operators from posit to int.

  Goal is to use a single lookup table for any posit configuration with 16 or fewer bits.
*/

template<size_t nbits, size_t es>
void GeneratePositTable(ostream& ostr) 
{
	ostr << "Generate Posit Lookup table for a POSIT<" << nbits << "," << es << ">" << endl;

	const size_t size = (1 << nbits);
	double lookup[size];
	posit<nbits, es>	myPosit;

	for (int i = 0; i < size; i++) {
		myPosit.set_raw_bits(i);
		lookup[i] = myPosit.to_double();
	}

	const size_t index_column = 5;
	const size_t bin_column = 16;
	const size_t k_column = 16;
	const size_t sign_column = 16;
	const size_t regime_column = 30;
	const size_t exponent_column = 16;
	const size_t fraction_column = 16;
	const size_t value_column = 30;

	ostr << setw(index_column) << " # "
		<< setw(bin_column) << " Binary"
		<< setw(k_column) << " k-value"
		<< setw(sign_column) << "sign"
		<< setw(regime_column) << " regime"
		<< setw(exponent_column) << " exponent"
		<< setw(fraction_column) << " fraction"
		<< setw(value_column) << " value" << endl;
	for (int i = 0; i < size; i++) {
		myPosit.set_raw_bits(i);
		regime<nbits,es>   r = myPosit.get_regime();
		exponent<nbits,es> e = myPosit.get_exponent();
		fraction<nbits,es> f = myPosit.get_fraction();
		ostr << setw(4) << i << ": "
			<< setw(bin_column) << myPosit.get()
			<< setw(k_column) << myPosit.regime_k()
			<< setw(sign_column) << myPosit.sign_value()
			<< setw(regime_column) << setprecision(22) << r.value() << setprecision(0)
			<< setw(exponent_column) << right << e 
			<< setw(fraction_column) << right << f
			<< setw(value_column) << setprecision(22) << lookup[i] << setprecision(0)
			<< endl;
	}
}

int main(int argc, char** argv)
try {
	cerr << "Valid posit configurations" << endl;

	posit<4, 0> p;
	p.set_raw_bits(11);
	cout << p << " " << components_to_string(p) << endl;
	//return 0;

	/*
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
	*/

	GeneratePositTable<7, 2>(cout);
	return 0;
	GeneratePositTable<7, 3>(cout);
	return 0;

	GeneratePositTable<8, 0>(cout);
	GeneratePositTable<8, 1>(cout);
	GeneratePositTable<8, 2>(cout);
	GeneratePositTable<8, 3>(cout);
	GeneratePositTable<8, 4>(cout);

}
catch (char* e) {
	cerr << e << endl;
}
