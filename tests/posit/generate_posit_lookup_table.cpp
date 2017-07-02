// create_small_posit_lookup_table.cpp

#include "stdafx.h"
#include <sstream>
#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"

using namespace std;

/*
  Posit values are a combination of a scaling factor, useed, an exponent, e, and a fraction, f.
  For small posits, it is simpler to have a lookup mechanism to obtain the value.
  This is most valuable for conversion operators from posit to int.
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
	const size_t regime_column = 16;
	const size_t exponent_column = 16;
	const size_t fraction_column = 16;
	const size_t value_column = 16;

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
		string binary = to_binary(myPosit.get_raw_bits());
		string exponent = "-";
		if (nbits - 3 > 0 && es > 0) {
			exponent = to_binary(myPosit.exponent_bits());
		}
		string fraction = "-";
		if (int(nbits) - 3 - int(es) > 0) {
			fraction = to_binary(myPosit.fraction_bits());
		}
		ostr << setw(4) << i << ": "
			<< setw(bin_column) << binary
			<< setw(k_column) << myPosit.run_length()
			<< setw(sign_column) << myPosit.sign()
			<< setw(regime_column) << setprecision(7) << myPosit.regime() << setprecision(0)
			<< setw(exponent_column) << exponent
			<< setw(fraction_column) << fraction
			<< setw(value_column) << setprecision(7) << lookup[i] << setprecision(0)
			<< endl;
	}
}

int main(int argc, char** argv)
{

	GeneratePositTable<3, 0>(cout);
	GeneratePositTable<3, 1>(cout);
	GeneratePositTable<3, 2>(cout);

	GeneratePositTable<4, 0>(cout);
	GeneratePositTable<4, 1>(cout);
	GeneratePositTable<4, 2>(cout);
	GeneratePositTable<4, 3>(cout);

	GeneratePositTable<5, 0>(cout);
	GeneratePositTable<5, 1>(cout);
	GeneratePositTable<5, 2>(cout);
	GeneratePositTable<5, 3>(cout);
	GeneratePositTable<5, 4>(cout);	

	return 0;
}
