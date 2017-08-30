// create_small_posit_lookup_table.cpp

#include "stdafx.h"
#include <sstream>
#include "../../posit/posit_regime_lookup.hpp"
#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"

using namespace std;

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
	posit<5, 0> p5_0;
	posit<5, 1> p5_1;
	posit<5, 2> p5_2;

	cout << "posit<5,0> useed = " << p5_0.useed() << ", minpos_scale = " << p5_0.minpos_scale() << ", maxpos_scale = " << p5_0.maxpos_scale() << endl;
	cout << "posit<5,1> useed = " << p5_1.useed() << ", minpos_scale = " << p5_1.minpos_scale() << ", maxpos_scale = " << p5_1.maxpos_scale() << endl;
	cout << "posit<5,2> useed = " << p5_2.useed() << ", minpos_scale = " << p5_2.minpos_scale() << ", maxpos_scale = " << p5_2.maxpos_scale() << endl;

	p5_0 = 9;    // useed =  2, maxpos = useed ^ 3 =    8
	p5_1 = 65;   // useed =  4, maxpos = useed ^ 3 =   64
	p5_2 = 4097; // useed = 64, maxpos = useed ^ 3 = 4096


	cout << "posit<5,0> = " << p5_0 << endl;
	cout << "posit<5,1> = " << p5_1 << endl;
	cout << "posit<5,2> = " << p5_2 << endl;

	return 0;
}
