// unum.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "posit_scale_factors.hpp"

using namespace std;

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
	long long rmin = POW2(es) * (2 - nbits);
	long long rmax = POW2(es) * (nbits - 2);
	long long rf = MIN(MAX(exponent - exponentBias, rmin), rmax);

	cout << "rmin " << rmin << " rmax " << rmax << " rf " << rf << endl;

	uint32_t positSignBit = f >> (fes + fms);
	int64_t positRegionSize = rf >> es;
	int64_t positExponentSize = rf - POW2(es) * positRegionSize;

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
		useed = POW2(POW2(uint64_t(es)));
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

	p1 = 0;  //checkSpecialCases(p1);
	return;

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

//	p0 = int(0);  checkSpecialCases(p0);
	p1 = char(1);  cout << "P1 " << p1 << endl;
	p2 = long(2);  cout << "P2 " << p2 << endl;
	p3 = 4;  cout << "P3 " << p3 << endl;
	p4 = 8;  cout << "P4 " << p4 << endl;
	p5 = 16;  cout << "P5 " << p5 << endl;
	p6 = (long long)(32);  cout << "P6 " << p6 << endl;
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

void testRawBitPatterns() {
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
	testRawBitPatterns();
//	testConversionOperatorsPositiveRegime();
//	testConversionOperatorsNegativeRegime();
//	testBasicOperators();
    return 0;
}



