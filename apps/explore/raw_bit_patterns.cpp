// raw_bit_patterns.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "../../posit/posit_scale_factors.hpp"
#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"

using namespace std;

uint64_t GENERATED_SCALE_FACTORS[MAX_ES][MAX_K];

int main() {
	posit<16, 2> p;
	bitset<16> raw;

	raw.reset();
	// positive regime infinity - 1
	raw[15] = 1;
	p.set(raw); 	cout << p << endl;
	raw.set();
	raw[15] = false;						// 1152921504606846976			
	p.set(raw); 	cout << p << endl;
	raw[0] = false;							// 72057594037927936			
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
	raw[9] = true;			// 1/4096
	p.set(raw); 	cout << p << endl;
	raw.reset();
	raw[8] = true;			// 1/4096
	p.set(raw); 	cout << p << endl;
	raw.reset();
	raw[7] = true;			// 1/4096
	p.set(raw); 	cout << p << endl;
	raw.reset();
	raw[6] = true;			// 1/4096
	p.set(raw); 	cout << p << endl;
	raw.reset();
	raw[5] = true;			// 1/4096
	p.set(raw); 	cout << p << endl;
	raw.reset();
	raw[4] = true;			// 1/4096
	p.set(raw); 	cout << p << endl;
	raw.reset();
	raw[3] = true;			// 1/4096
	p.set(raw); 	cout << p << endl;
	raw.reset();
	raw[2] = true;			// 1/4096
	p.set(raw); 	cout << p << endl;
	raw.reset();
	raw[1] = true;			// 1/4096
	p.set(raw); 	cout << p << endl;
	raw[1] = false;			// 0
	p.set(raw); 	cout << p << endl;

    	return 0;
}
