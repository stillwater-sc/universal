// create_small_posit_lookup_table.cpp

#include "stdafx.h"

#include "../../posit/posit.hpp"

using namespace std;

/*
  Posit values are a combination of a scaling factor, useed, an exponent, e, and a fraction, f.
  For small posits, it is cleaner to have a lookup mechanism to obtain the value.
  This is vaulable for conversion operators from posit to int.
*/

int main(int argc, char** argv)
{
    cout << "Generate Posit Lookup table" << endl;

    // 8 bit posits cover 256 combinations with two special cases '0000-0000' and '1000-0000'
	const size_t nbits = 5;
	const size_t size = (1 << nbits);
    uint8_t	lookup[size];
    posit<nbits,0>	posit_number;

    for ( int i = 0; i < size; i++ ) {
        posit_number.set_raw_bits(i);
        lookup[i] = uint8_t(posit_number.to_ulong());
    }

    for ( int i = 0; i < size; i++ ) {
		posit_number.set_raw_bits(i);
        cout << setw(3) << i << ": " << setw(3) << hex << int(lookup[i]) << " " << dec << setw(3) << posit_number.identifyRegime() << endl;
    }

    return 0;
}
