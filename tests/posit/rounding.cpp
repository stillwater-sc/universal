// Round-to-nearest tests for posits

#include "stdafx.h"

#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"

using namespace std;

template<size_t nbits>
void transform_into_sign_scale_fraction(int64_t value) {
	bool _sign = (0x8000000000000000 & value);
	unsigned int _scale = findMostSignificantBit(value) - 1;
	uint64_t _fraction_without_hidden_bit = (value << (64 - _scale));
	std::bitset<nbits - 3> _fraction = copy_integer_fraction<nbits>(_fraction_without_hidden_bit);
	cout << "Value    " << value << endl << "Binary   " << to_binary(value) << endl;
	cout << "Sign     " << _sign << endl << "Scale    " << _scale << endl << "Fraction " << _fraction << endl;
}

/*
  Posits are a tapered floating point system, and this creates additional requirements for rounding.

  Rounding to nearest is more involved as we need to find geometric or arithmetic means between two posits
  to find the proper projection for a real value.

  Posit Lookup table for a POSIT<5,1>
   #           Binary         k-value            sign                        regime        exponent        fraction                         value
   0:            00000              -4               1                    0.00390625               0              00                             0
   1:            00001              -3               1                      0.015625               0              00                      0.015625
   2:            00010              -2               1                        0.0625               0              00                        0.0625
   3:            00011              -2               1                        0.0625               1              00                         0.125
   4:            00100              -1               1                          0.25               0              00                          0.25
   5:            00101              -1               1                          0.25               0              10                         0.375
   6:            00110              -1               1                          0.25               1              00                           0.5
   7:            00111              -1               1                          0.25               1              10                          0.75
   8:            01000               0               1                             1               0              00                             1
   9:            01001               0               1                             1               0              10                           1.5
  10:            01010               0               1                             1               1              00                             2
  11:            01011               0               1                             1               1              10                             3
  12:            01100               1               1                             4               0              00                             4
  13:            01101               1               1                             4               1              00                             8
  14:            01110               2               1                            16               0              00                            16
  15:            01111               3               1                            64               0              00                            64
  16:            10000               4              -1                           256               0              00                           inf
  17:            10001               3              -1                            64               0              00                           -64
  18:            10010               2              -1                            16               0              00                           -16
  19:            10011               1              -1                             4               1              00                            -8
  20:            10100               1              -1                             4               0              00                            -4
  21:            10101               0              -1                             1               1              10                            -3
  22:            10110               0              -1                             1               1              00                            -2
  23:            10111               0              -1                             1               0              10                          -1.5
  24:            11000               0              -1                             1               0              00                            -1
  25:            11001              -1              -1                          0.25               1              10                         -0.75
  26:            11010              -1              -1                          0.25               1              00                          -0.5
  27:            11011              -1              -1                          0.25               0              10                        -0.375
  28:            11100              -1              -1                          0.25               0              00                         -0.25
  29:            11101              -2              -1                        0.0625               1              00                        -0.125
  30:            11110              -2              -1                        0.0625               0              00                       -0.0625
  31:            11111              -3              -1                      0.015625               0              00                     -0.015625
*/
int main(int argc, char** argv)
{
	try {
        const size_t nbits = 5;
        const size_t es = 1;
        posit<nbits,es> p;
		uint64_t value;

		cout << spec_to_string(p) << endl;

		value = 4;
		transform_into_sign_scale_fraction<nbits>(value);
		p = value; // is 4
		cout << p << endl;

		value = 5;
		transform_into_sign_scale_fraction<nbits>(value);
		p = value; // projects to 4
		cout << p << endl;

		value = 6;
		transform_into_sign_scale_fraction<nbits>(value);
		p = value; // projects to 8
		cout << p << endl;

		value = 7; 
		transform_into_sign_scale_fraction<nbits>(value);
		p = value; // projects to 8
		cout << p << endl;

		value = 8;
		transform_into_sign_scale_fraction<nbits>(value);
		p = value; // is 8
		cout << p << endl;

	}
	catch (char* e) {
		cerr << e << endl;
	}

	return 0;
}
