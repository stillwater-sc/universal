// create_small_posit_lookup_table.cpp

#include "stdafx.h"
#include <sstream>
#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"

using namespace std;

/*
  Posit values are a combination of a scaling factor, useed, an exponent, e, and a fraction, f.
  For small posits, it is cleaner to have a lookup mechanism to obtain the value.
  This is vaulable for conversion operators from posit to int.
*/

int main(int argc, char** argv)
{
    // 8 bit posits cover 256 combinations with two special cases 0 == '0000-0000' and +-inf == '1000-0000'
	const size_t nbits = 5;
	const size_t es = 1;
	cout << "Generate Posit Lookup table for a POSIT<" << nbits << "," << es << ">" << endl;

	const size_t size = (1 << nbits);
	double lookup[size];
    posit<nbits,es>	myPosit;

    for ( int i = 0; i < size; i++ ) {
        myPosit.set_raw_bits(i);
        lookup[i] = myPosit.to_double();
    }

	const size_t index_column    =  5;
	const size_t bin_column      = 16;
	const size_t k_column        = 16;
	const size_t sign_column     = 16;
	const size_t regime_column   = 16;
	const size_t exponent_column = 16;
	const size_t fraction_column = 16;
	const size_t value_column    = 16;

	cout << setw(index_column) << " # " 
		 << setw(bin_column) << " Binary" 
		 << setw(k_column) << " k-value" 
		 << setw(sign_column) << "sign"
		 << setw(regime_column) << " regime" 
		 << setw(exponent_column) << " exponent"
		 << setw(fraction_column) << " fraction"
		 << setw(value_column) << " value" << endl;
    for ( int i = 0; i < size; i++ ) {
		myPosit.set_raw_bits(i);
		string binary = to_binary(myPosit.get_raw_bits());
		string fraction = "-";
		if (nbits - 3 - es > 0) {
			fraction = to_binary(myPosit.fraction_bits());
		}
		cout << setw(4) << i << ": " 
			 << setw(bin_column) << binary
			 << setw(k_column) << myPosit.run_length()
			 << setw(sign_column) << myPosit.sign() 
			 << setw(regime_column) << setprecision(7) << myPosit.regime() << setprecision(0)
			 << setw(exponent_column) << myPosit.exponent()
			 << setw(fraction_column) << fraction
		     << setw(value_column) << hex << int(lookup[i]) << dec
			 << endl;
    }

    return 0;
}
