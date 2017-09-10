// create_small_posit_lookup_table.cpp

#include "stdafx.h"
#include <sstream>
#include "../../posit/posit_regime_lookup.hpp"
#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"

using namespace std;

template<size_t nbits>
void normalize(const std::bitset<nbits - 3>& fraction, std::bitset<nbits - 3>& number) {
	number.set(nbits - 4); // set hidden bit
	for (int i = nbits - 5; i >= 0; i--) {
		number.set(i, fraction[i + 2]);
	}
}
/*   h is hidden bit
*   h.bbbb_bbbb_bbbb_b...      fraction
*   0.000h_bbbb_bbbb_bbbb_b... number
*  >-.----<                    shift of 4
*/
template<size_t nbits>
void denormalize(const std::bitset<nbits - 3>& fraction, int shift, std::bitset<nbits - 3>& number) {
	for (int i = nbits - 4; i > nbits - 4 - shift; i--) {
		number.reset(i);
	}
	number.set(nbits - 4 - shift); // set hidden bit
	for (int i = nbits - 5 - shift; i >= 0; i--) {
		number.set(i, fraction[i + 2 + shift]);
	}
}

/*
	Testing the reciprocal nature of positive and negative posits

	posit<5,0>
	  #           Binary         k-value            sign          regime        exponent        fraction           value
	 8:            01000               0               1               1               -           00000               1
	 9:            01001               0               1               1               -           01000            1.25
	10:            01010               0               1               1               -           10000             1.5
	11:            01011               0               1               1               -           11000            1.75
	12:            01100               1               1               2               -           00000               2
	13:            01101               1               1               2               -           10000               3
	14:            01110               2               1               4               -           00000               4
	15:            01111               3               1               8               -           00000               8
	16:            10000               4              -1              16               -           00000             inf
	17:            10001               3              -1               8               -           00000              -8
	18:            10010               2              -1               4               -           00000              -4
	19:            10011               1              -1               2               -           10000              -3
	20:            10100               1              -1               2               -           00000              -2
	21:            10101               0              -1               1               -           11000           -1.75
	22:            10110               0              -1               1               -           10000            -1.5
	23:            10111               0              -1               1               -           01000           -1.25
	24:            11000               0              -1               1               -           00000              -1
	posit<5,1>
	  #           Binary         k-value            sign          regime        exponent        fraction           value
	 8:            01000               0               1               1               0           00000               1
	 9:            01001               0               1               1               0           10000             1.5
	10:            01010               0               1               1               1           00000               2
	11:            01011               0               1               1               1           10000               3
	12:            01100               1               1               4               0           00000               4
	13:            01101               1               1               4               1           00000               8
	14:            01110               2               1              16               0           00000              16
	15:            01111               3               1              64               0           00000              64
	16:            10000               4              -1             256               0           00000             inf
	17:            10001               3              -1              64               0           00000             -64
	18:            10010               2              -1              16               0           00000             -16
	19:            10011               1              -1               4               1           00000              -8
	20:            10100               1              -1               4               0           00000              -4
	21:            10101               0              -1               1               1           10000              -3
	22:            10110               0              -1               1               1           00000              -2
	23:            10111               0              -1               1               0           10000            -1.5
	24:            11000               0              -1               1               0           00000              -1
	posit<5,2>
	  #           Binary         k-value            sign          regime        exponent        fraction           value
	 8:            01000               0               1               1              00               -               1
	 9:            01001               0               1               1              01               -               2
	10:            01010               0               1               1              10               -               4
	11:            01011               0               1               1              11               -               8
	12:            01100               1               1              16              00               -              16
	13:            01101               1               1              16              01               -              32
	14:            01110               2               1             256              00               -             256
	15:            01111               3               1            4096              00               -            4096
	16:            10000               4              -1           65536              00               -             inf
	17:            10001               3              -1            4096              00               -           -4096
	18:            10010               2              -1             256              00               -            -256
	19:            10011               1              -1              16              01               -             -32
	20:            10100               1              -1              16              00               -             -16
	21:            10101               0              -1               1              11               -              -8
	22:            10110               0              -1               1              10               -              -4
	23:            10111               0              -1               1              01               -              -2
	24:            11000               0              -1               1              00               -              -1
*/
int main(int argc, char** argv)
{
	const size_t nbits = 16;

	goto debug_test;
	//goto float_posit_comparison;

	posit<nbits, 0> p0;
	posit<nbits, 1> p1;
	posit<nbits, 2> p2;

	cout << spec_to_string(p0) << endl;
	cout << spec_to_string(p1) << endl;
	cout << spec_to_string(p2) << endl;

	p0 = 9;    // useed =  2, maxpos = useed ^ 3 =    8
	p1 = 65;   // useed =  4, maxpos = useed ^ 3 =   64
	p2 = 4097; // useed = 64, maxpos = useed ^ 3 = 4096

	cout << "posit<" << nbits << ",0> = " << components_to_string(p0) << endl;
	cout << "posit<" << nbits << ",1> = " << components_to_string(p1) << endl;
	cout << "posit<" << nbits << ",2> = " << components_to_string(p2) << endl;

	cout << "posit<" << nbits << ",2> = " << component_values_to_string(p2) << endl;

	{
		posit<8, 0> a, b, sum, diff;
		a.set_raw_bits(0x5F);
		b = 1;
		sum = a + b;
		cout << "Sum  : " << sum << endl;
	}

	{
		posit<16,0> a = 0;
		for (uint32_t i = 0; i < 10; i++) {
			cout << "a = " << a++ << " a++ = " << a << endl;
		}
	}

float_posit_comparison:
	{
		float fa, fb, fsum;
		posit<16, 1> pa, pb, psum;
		fa = 16;  pa = 16; 
		for (int i = 5; i < 16; i++) {
			fb = i; fsum = fa + fb;
			//cout << fa << " + " << fb << " = " << fsum << endl;
			pb = i; psum = pa + pb;
			cout << pa << " + " << pb << " = " << psum << endl;
		}
		return 0;
	}

debug_test:
	{
		cout << to_binary(6) << endl;
		cout << to_binary(16) << endl;
		cout << to_binary(24) << endl;
		const size_t nbits = 8;
		const size_t es = 1;
		posit<nbits, es> pa, pb, psum;
		int32_t va, vb;
		va = 16; vb = 5;
		pa = va; pb = vb;

		int lhs_scale = pa.scale();
		int rhs_scale = pb.scale();
		cout << "LHS " << va << " scale " << lhs_scale << endl;
		cout << "RHS " << vb << " scale " << rhs_scale << endl;
		int diff = lhs_scale - rhs_scale;
		if (diff < 0) {
			cerr << "Wrong" << endl;
		}
		else {
			bitset<nbits-3> r1, r2;
			normalize<nbits>(pa.fraction_bits(), r1);
			cout << "frac1 " << pa.fraction_bits() << endl;
			cout << "r1    " << r1 << endl;

			denormalize<nbits>(pb.fraction_bits(), diff, r2);
			cout << "frac2 " << pb.fraction_bits() << endl;
			cout << "r2    " << r2 << endl;
		}
		cout.flush();
		psum = pa + pb;
		cout << pa << " + " << pb << " = " << psum << endl;
		cout << psum.get() << endl;
		psum = 21;
		cout << psum.get() << endl;
	}


	//diff = a - b;
	//cout << "Diff : " << diff << endl;

	//cout << "a " << a++ << " a+1 " << a << endl;

	return 0;
}
