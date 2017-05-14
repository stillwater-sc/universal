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
/*               k = 0         k = 1            k = 2         k = 3          k = 4            k = 5              k = 6         k = 7         k = 8          k = 9  */
	/* es = 0 */ {   1,            4,               8,           16,            32,              64,               128,          256,          512,          1024 },
	/* es = 1 */ {   1,           16,              64,          256,          1024,            4096,             16384,        65536,       262144,       1048576 },
	/* es = 2 */ {   1,          256,            4096,        65536,       1048576,        16777216,         268435456,   4294967296,  68719476736, 1099511627776 },
	/* es = 3 */ {   1,        65536,        16777216,   4294967296, 1099511627776, 281474976710656, 72057594037927936,            0,            0,             0 },
	/* es = 4 */ {   1,   4294967296, 281474976710656,            0,             0,               0,                 0,            0,            0,             0 },
};

uint64_t GENERATED_SCALE_FACTORS[MAX_ES][MAX_K];

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

void generateScaleFactorLookupTable() {
	uint64_t useed, useed_power_k;
	for (int es = 0; es < MAX_ES; es++) {
		useed = POW2(POW2(es));
		useed_power_k = useed; 
		GENERATED_SCALE_FACTORS[es][0] = 1; // for k = 0
		for (int k = 1; k < MAX_K; k++) {
			useed_power_k *= useed;
			GENERATED_SCALE_FACTORS[es][k] = useed_power_k;
		}
	}

}


template<size_t nbits, size_t es>
void checkSpecialCases(posit<nbits, es> p) {
	cout << "posit is " << (p.isZero() ? "zero " : "non-zero ") << (p.isPositive() ? "positive " : "negative ") << (p.isInfinite() ? "+-infinite" : "not infinite") << endl;
}

void testBasicOperators() {
	posit<16, 1> p1, p2, p3, p4, p5, p6;

	p1.Range();

	p1 = 0;  checkSpecialCases(p1);
	p1 = 1;  checkSpecialCases(p1);
	p2 = 2;  checkSpecialCases(p2);

	p3 = p1 + p2;
	p4 = p2 - p1;
	p5 = p2 * p3;
	p6 = p5 / p3;

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

	// negative regime
	p1 = -1; checkSpecialCases(p1);
}

void testConversionOperatorsPositiveRegime() {
	posit<16, 1> p0, p1, p2, p3, p4, p5, p6;

	p0.Range();

	p0 = 0;  checkSpecialCases(p0);
	p1 = 1;  checkSpecialCases(p1);
	p2 = 2;  checkSpecialCases(p2);
	p3 = 4;  checkSpecialCases(p3);
	p4 = 8;  checkSpecialCases(p4);
	p5 = 16;  checkSpecialCases(p5);
	p6 = 32;  checkSpecialCases(p6);
}

void testConversionOperatorsNegativeRegime() {
	posit<16, 1> p0, p1, p2, p3, p4, p5, p6;

	p0.Range();

	p0 = 0;  checkSpecialCases(p0);
	p1 = -1;  checkSpecialCases(p1);
	p2 = -2;  checkSpecialCases(p2);
	p3 = -4;  checkSpecialCases(p3);
	p4 = -8;  checkSpecialCases(p4);
	p5 = -16;  checkSpecialCases(p5);
	p6 = -32;  checkSpecialCases(p6);
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


void printScaleFactors(uint64_t scale_factors[MAX_ES][MAX_K]) {
	cout << "      ";
	for (int k = 0; k < MAX_K; k++) {
		cout << "     k = " << k << "   ";
	}
	cout << endl;
	for (int es = 0; es < MAX_ES; es++) {
		cout << "es = " << es << " ";
		for (int k = 0; k < MAX_K; k++) {
			cout << setw(12) << scale_factors[es][k] << " ";
		}
		cout << endl;
	}
	cout << endl;
}

int main()
{
//	generateScaleFactorLookupTable();
//	printScaleFactors(GENERATED_SCALE_FACTORS);
	testConversionOperatorsPositiveRegime();
//	testConversionOperatorsNegativeRegime();
//	testBasicOperators();
    return 0;
}



