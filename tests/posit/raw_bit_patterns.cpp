// raw_bit_patterns.cpp : Defines the entry point for the console application.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"
#include <sstream>

#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"

using namespace std;

/* OUTPUT
72057594037927936
4503599627370496
281474976710656
17592186044416
1099511627776
68719476736
4294967296
268435456
16777216
1048576
65536
4096
256
16
1 1
0.0625
0.00390625
0.000244140625
1.52587890625e-05
9.5367431640625e-07
5.9604644775390625e-08
3.7252902984619140625e-09
2.3283064365386962890625e-10
1.4551915228366851806640625e-11
9.094947017729282379150390625e-13
5.684341886080801486968994140625e-14
3.552713678800500929355621337890625e-15
2.220446049250313080847263336181641e-16
0 0
*/

int main() 
try {
	posit<16, 2> p;
	bitset<16> raw;
	int nrOfFailedTestCases = 0;

	// 1152921504606846976
	raw.reset();
	// positive regime infinity - 1
	cout << setprecision(34);
	raw[15] = 1;							// inf
	p.set(raw); 	cout << p << endl;
	raw.set();
	raw[15] = false;						// 72057594037927936			
	p.set(raw); 	cout << p << endl;
	raw[0] = false;							// 4503599627370496			
	p.set(raw); 	cout << p << endl;
	raw[1] = false;							// 281474976710656			
	p.set(raw); 	cout << p << endl;
	raw[2] = false;							// 17592186044416			
	p.set(raw); 	cout << p << endl;
	raw[3] = false;							// 1099511627776			
	p.set(raw); 	cout << p << endl;
	raw[4] = false;							// 68719476736			
	p.set(raw); 	cout << p << endl;
	raw[5] = false;							// 4294967296			
	p.set(raw); 	cout << p << endl;
	raw[6] = false;							// 268435456			
	p.set(raw); 	cout << p << endl;
	raw[7] = false;							// 16777216			
	p.set(raw); 	cout << p << endl;
	raw[8] = false;							// 1048576			
	p.set(raw); 	cout << p << endl;
	raw[9] = false;							// 65536			
	p.set(raw); 	cout << p << endl;
	raw[10] = false;						// 4096			
	p.set(raw); 	cout << p << endl;
	raw[11] = false;						// 256
	p.set(raw); 	cout << p << endl;
	raw[12] = false;						// 16
	p.set(raw); 	cout << p << endl;
	raw[13] = false;						// 1
	p.set(raw); 	cout << p << " 1 " << endl;

	raw.reset();
	// positive fractional regime 1 - 0
	raw[13] = true;			// 1
	p.set(raw); 	cout << p << endl;
	raw.reset();
	raw[12] = true;			// 1/16
	p.set(raw); 	cout << p << endl;
	raw.reset();
	raw[11] = true;			// 1/256
	p.set(raw); 	cout << p << endl;
	raw.reset();
	raw[10] = true;			// 1/4096
	p.set(raw); 	cout << p << endl;
	raw.reset();
	raw[9] = true;			// 1/65536
	p.set(raw); 	cout << p << endl;
	raw.reset();
	raw[8] = true;			// 1/1048576
	p.set(raw); 	cout << p << endl;
	raw.reset();
	raw[7] = true;			// 1/16777216
	p.set(raw); 	cout << p << endl;
	raw.reset();
	raw[6] = true;			// 1/268435456
	p.set(raw); 	cout << p << endl;
	raw.reset();
	raw[5] = true;			// 1/4294967296
	p.set(raw); 	cout << p << endl;
	raw.reset();
	raw[4] = true;			// 1/68719476736
	p.set(raw); 	cout << p << endl;
	raw.reset();
	raw[3] = true;			// 1/1099511627776
	p.set(raw); 	cout << p << endl;
	raw.reset();
	raw[2] = true;			// 1/17592186044416
	p.set(raw); 	cout << p << endl;
	raw.reset();
	raw[1] = true;			// 1/281474976710656
	p.set(raw); 	cout << p << endl;
	raw[1] = false;			// 0
	p.set(raw); 	cout << p << " 0" << endl;

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}