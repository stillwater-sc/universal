// create_small_posit_lookup_table.cpp

#include "stdafx.h"
#include <sstream>
#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"

using namespace std;

/*
  Posit values are a combination of
  1) a scaling factor, useed, 
  2) an exponent, e, and 
  3) a fraction, f.
  For small posits, it is cleaner to have a lookup mechanism to obtain the value.
  This is valuable for conversion operators from posit to int.
*/

int main(int argc, char** argv)
{
	const size_t nbits = 8;
	const size_t es = 1;
	posit<nbits, es> myPosit;

	for (int i = 0; i < 256; i++) {
		myPosit.set_raw_bits(uint32_t(i));
		cout << myPosit << endl;
	}

	return 0;
}
