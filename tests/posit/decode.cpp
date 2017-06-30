// create_small_posit_lookup_table.cpp

#include "stdafx.h"
#include <sstream>
#include "../../posit/posit.hpp"

using namespace std;

template<size_t nbits>
std::string BinaryRepresentation(std::bitset<nbits> bits) {
	std::stringstream ss;
	for (int i = nbits - 1; i >= 0; --i) {
		if (bits[i]) {
			ss << "1";
		}
		else {
			ss << "0";
		}
	}
	return ss.str();
}

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
	posit<5, 2> p5_2;
	p5_2.set_raw_bits(uint32_t(9));
	bitset<4> regime = p5_2.regime_bits();
	bitset<2> exp = p5_2.exponent_bits();
	cout << BinaryRepresentation(p5_2.get_raw_bits()) << " "
		<< BinaryRepresentation(regime) << " "
		<< BinaryRepresentation(exp)
		<< endl;

	for (int i = 0; i < 32; i++) {
		p5_2.set_raw_bits(uint32_t(i));
		bitset<4> regime = p5_2.regime_bits();
		bitset<2> exp = p5_2.exponent_bits();
		cout << BinaryRepresentation(p5_2.get_raw_bits()) << " "
			 << BinaryRepresentation(regime) << " "
			 << BinaryRepresentation(exp)
			 << endl;
	}

	return 0;
}
