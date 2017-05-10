// unum.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

using namespace std;

/*
LOOKUP table for different useed^k values
useed = 2^2^es
The table is just positive k's as columns, and es as rows
         k = 0  k = 1  k = 2 ...
es = 0   
es = 1          useed^k
es = 2
...

*/
#define MAX_ES 5
#define MAX_K 10
const uint64_t SCALE_FACTORS[MAX_ES][MAX_K] = {
	{1,     1,         1,             1,          1,             1,            1,         1,          1,           1},
	{1,     4,        16,            64,        256,          1024,         4096,     16384,      65536,        262144},
	{1,    16,       256,          4096,      65536,       1048576,     16777216, 268435546, 4294967296,    68719476736},
	{1,   256,     65536,      16777216, 4294967296, 1099511627776,           -1,   -1,   -1,   -1},
	{1, 16384, 268435546, 4398047985664,         -1,            -1,           -1, -1, -1, -1}
};

/*
Laid out as bits, floating point numbers look like this:
Single: SEEEEEEE EMMMMMMM MMMMMMMM MMMMMMMM
Double: SEEEEEEE EEEEMMMM MMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM

1.	The sign bit is 0 for positive, 1 for negative.
2.	The exponent base is two.
3.	The exponent field contains 127 plus the true exponent for single-precision, or 1023 plus the true exponent for double precision.
4.	The first bit of the mantissa is typically assumed to be 1.f, where f is the field of fraction bits.

*/
template<size_t nbits, size_t es>
void extract(uint32_t f, int fes, int fms) {
	cout << "value : " << dec << f << " bits : " << hex << f << " mantissa mask : " << (1ULL << fms)-1 << dec << endl;

	int exponentBias = POW2(fes - 1) - 1;
	int16_t exponent = (f >> fms) & ((1 << fes) - 1);
	uint32_t mantissa = (f & ((1ULL << fms) - 1));

	cout << " mantissa : " << hex << mantissa << " exponent : " << exponent << " bias " << exponentBias << dec << endl;

	// clip exponent
	long rmin = POW2(es) * (2 - nbits);
	long rmax = POW2(es) * (nbits - 2);
	long rf = MIN(MAX(exponent - exponentBias, rmin), rmax);

	cout << "rmin " << rmin << " rmax " << rmax << " rf " << rf << endl;

	uint32_t positSignBit = f >> (fes + fms);
	int positRegionSize = rf >> es;
	int positExponentSize = rf - POW2(es) * positRegionSize;

	cout << "positSignBit " << positSignBit << " positRegionSize " << positRegionSize << " exponent " << positExponentSize << endl;

	uint32_t positFraction;
	if (fms <= nbits) {
		positFraction = mantissa << (nbits - fms);
	}
	else {
		positFraction = mantissa >> (fms - nbits);
	}
	cout << "posit Fraction " << positFraction << endl;
}

void testBasicOperators() {
	posit<16, 1> p1, p2, p3, p4, p5, p6;

	cin >> p1 >> p2;

	cout << "p1 is " << (p1.isZero() ? "zero " : "non-zero ") << (p1.isPositive() ? "positive " : "negative ") << (p1.isInfinite() ? "+-infinite" : "not infinite") << endl;
	cout << "p2 is " << (p2.isZero() ? "zero " : "non-zero ") << (p2.isPositive() ? "positive " : "negative ") << (p2.isInfinite() ? "+-infinite" : "not infinite") << endl;

	long long aLong = -65536;
	p2 = aLong;
	p3 = p1 + p2;
	p4 = p2 - p1;
	p5 = p2 * p3;
	p6 = p5 / p3;

	p1.Info();
	cout << "p1: " << p1 << "\n";
	cout << "p2: " << p2 << "\n";
	cout << "p3: " << p3 << "\n";
	cout << "p4: " << p4 << "\n";
	cout << "p5: " << p5 << "\n";
	cout << "p6: " << p6 << "\n";
	cout << "p1++ " << p1++ << " " << p1 << "\n";
	cout << "++p1 " << ++p1 << "\n";
	cout << "p1-- " << p1-- << " " << p1 << "\n";
	cout << "--p1 " << --p1 << "\n";
}

void extractTest()
{
	union {
		float f1 = 2.0e9f;
		uint32_t bits;
	};
	cout << "Value : " << f1 << endl;
	extract<16, 1>(bits, 8, 23);
}

void printScaleFactors(int es_max, int k_max) {
	cout << "      ";
	for (int j = 0; j < k_max; j++) {
		cout << "     k = " << j << "   ";
	}
	cout << endl;
	for (int i = 0; i < es_max; i++) {
		cout << "es = " << i << " ";
		for (int j = 0; j < k_max; j++) {
			cout << setw(12) << SCALE_FACTORS[i][j] << " ";
		}
		cout << endl;
	}
	cout << endl;
}
int main()
{
	printScaleFactors(MAX_ES, MAX_K);
	testBasicOperators();
    return 0;
}



