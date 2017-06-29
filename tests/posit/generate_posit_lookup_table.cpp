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

    uint8_t	lookup[256];
    posit<8,0>	p8_0;

    for ( int i = 0; i < 256; i++ ) {
        p8_0.SetBits(i);
        lookup[i] = uint8_t(p8_0.to_ulong());
    }

    for ( int i = 0; i < 256; i++ ) {
        cout << setw(3) << i << lookup[i] << endl;
    }

    return 0;
}
