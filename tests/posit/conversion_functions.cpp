// conversion_functions.cpp : api experiments for conversion algorithms
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"

using namespace std;

template<size_t nbits, size_t es>
void checkSpecialCases(posit<nbits, es> p) {
	cout << "posit is " << (p.isZero() ? "zero " : "non-zero ") << (p.isPositive() ? "positive " : "negative ") << (p.isInfinite() ? "+-infinite" : "not infinite") << endl;
}

void ConversionExamplesPositiveRegime() {
	posit<5, 1> p0, p1, p2, p3, p4, p5, p6;

	cout << "Minpos = " << setprecision(21) << p0.minpos_value() << endl;
	cout << "Maxpos = " << p0.maxpos_value() << setprecision(0) << endl;

	int64_t number = 1;
	for (int i = 0; i < 8; i++) {
		p0 = number;
		cout << p0 << endl;
		number <<= 1;
	}

	return;

	p0 = int(0);  checkSpecialCases(p0);
	p1 = char(1);  cout << "p1 " << p1 << endl;
	p2 = long(2);  cout << "p2 " << p2 << endl;
	p3 =  4;  cout << "p3 " << p3 << endl;
	p4 =  8;  cout << "p4 " << p4 << endl;
	p5 = 16;  cout << "p5 " << p5 << endl;
	p6 = 32;  cout << "p6 " << p6 << endl;
}

void ConversionExamplesNegativeRegime() {
	posit<5, 1> p0, p1, p2, p3, p4, p5, p6;

	cout << "Minpos = " << setprecision(21) << p0.minpos_value() << endl;
	cout << "Maxpos = " << p0.maxpos_value() << setprecision(0) << endl;

	p0 = 0;  checkSpecialCases(p0);
	p1 = -1;  cout << "p1 " << p1 << endl;
	p2 = -2;  cout << "p2 " << p2 << endl;
	p3 = -4;  cout << "p3 " << p3 << endl;
	p4 = -8;  cout << "p4 " << p4 << endl;
	p5 = -16; cout << "p5 " << p5 << endl;
	p6 = -32; cout << "p6 " << p6 << endl;
}

// what are you trying to capture with this method? TODO
// return the position of the msb of the largest binary number representable by this posit?
// this would be the scale of maxpos
template<size_t nbits, size_t es>
unsigned int maxpos_scale_f()  {
	return (nbits-2) * (1 << es);
}

unsigned int scale(unsigned int max_k, unsigned int es) {
	return max_k * (1 << es);
}

unsigned int base_regime(int64_t value, unsigned int es) {
	return (findMostSignificantBit(value) - 1) >> es;
}

unsigned int base_exponent(unsigned int msb, unsigned int es) {
	unsigned int value = 0;
	if (es > 0) {
		value = msb % (1 << es);
	}
	return value;
}

uint64_t base_fraction(int64_t value) {
	unsigned int hidden_bit_at = findMostSignificantBit(value) - 1;
	uint64_t mask = ~(uint64_t(1) << hidden_bit_at);
	return value & mask;
}

void EnumerationTests() {
	// set the max es we want to evaluate. useed grows very quickly as a function of es
	int max_es = 4;

	// cycle through the k values to test the scale calculation
	// since useed^k grows so quickly, we can't print the value, 
	// so instead we just print the scale of the number as measured in the binary exponent of useed^k = k*2^es
	cout << setw(10) << "posit size" << setw(6) << "max_k" << "   scale of max regime" << endl;
	cout << setw(16) << "           ";
	for (int i = 0; i < max_es; i++) {
		cout << setw(5) << "es@" << i;
	}
	cout << endl;
	for (int max_k = 1; max_k < 14; max_k++) {
		cout << setw(10) << max_k + 2 << setw(6) << max_k;
		for (int es = 0; es < max_es; es++) {
			cout << setw(6) << scale(max_k, es);
		}
		cout << endl;
	}

	// cycle through scales to test the regime determination
	cout << setw(10) << "Value";
	for (int i = 0; i < max_es; i++) {
		cout << setw(7) << "k";
	}
	cout << endl;
	unsigned int value = 1;
	for (int i = 0; i < 16; i++) {
		cout << setw(10) << value;
		for (int es = 0; es < max_es; es++) {
			cout << setw(7) << base_regime(value, es);
		}
		cout << endl;
		value <<= 1;
	}

	// cycle through a range to test the exponent extraction
	for (int i = 0; i < 32; i++) {
		cout << setw(10) << i;
		for (int es = 0; es < max_es; es++) {
			cout << setw(5) << base_exponent(i, es);
		}
		cout << endl;
	}

	// cycle through a range to test the faction extraction
	for (int i = 0; i < 32; i++) {
		cout << setw(10) << hex << i << setw(5) << base_fraction(i) << endl;
	}
}

void basic_algorithm_for_conversion() {
	const size_t nbits = 5;
	const size_t es = 1;

	int64_t value;
	unsigned int msb;
	unsigned int maxpos_scale = maxpos_scale_f<nbits, es>();

	value = 8;
	msb = findMostSignificantBit(value) - 1;
	if (msb > maxpos_scale) {
		cerr << "msb = " << msb << " and maxpos_scale() = " << maxpos_scale << endl;
		cerr << "Can't represent " << value << " with posit<" << nbits << "," << es << ">: maxpos = " << (1 << maxpos_scale) << endl;
	}
	// we need to find the regime for this rhs
	// regime represents a scale factor of useed ^ k, where k ranges from [1-nbits, nbits-2]
	// regime @ k = 0 -> 1
	// regime @ k = 1 -> (1 << (1 << es) ^ 1 = 2
	// regime @ k = 2 -> (1 << (1 << es) ^ 2 = 4
	// regime @ k = 3 -> (1 << (1 << es) ^ 3 = 8
	// the left shift of the regime is simply k * 2^es
	// which means that the msb of the regime is simply k*2^es

	// a posit has the form: useed^k * 2^exp * 1.fraction
	// useed^k is the regime and is encoded by the run length m of:
	//   - a string of 0's for numbers [0,1), and 
	//   - a string of 1's for numbers [1,inf)

	// The value k ranges from [1-nbits,nbits-2]
	//  m  s-regime   k  
	//  ...
	//  4  0-00001   -4
	//  3  0-0001    -3
	//  2  0-001     -2
	//  1  0-01      -1
	//  1  0-10       0
	//  2  0-110      1
	//  3  0-1110     2
	//  4  0-11110    3
	//  ...
	//
	// We'll be using m to convert from posit to integer/float.
	// We'll be using k to convert from integer/float to posit

	// The first step to convert an integer to a posit is to find the base regime scale
	// The base is defined as the biggest k where useed^k < integer
	// => k*2^es < msb && (k+1)*2^es > msb
	// => k < msb/2^es && k > msb/2^es - 1
	// => k = (msb >> es)

	// algorithm: convert int64 to posit<nbits,es>
	// step 1: find base regime
	//         if int64 is positive
	//            base regime = useed ^ k, where k = msb_of_int64 >> es
	//         else
	//            negate int64
	//            base regime = useed ^ k, where k = msb_of_negated_int64 >> es
	// step 2: find exponent
	//         exp = msb % 2^es
	// step 3: extract remaining fraction
	//         remove hidden bit
	// step 4: if int64 is negative, take 2's complement the posit of positive int64 calculated above
	//
	value = 33;
	cout << hex << "0x" << value << dec << setw(12) << value << endl;
	msb = findMostSignificantBit(value) - 1;
	cout << "MSB      = " << msb << endl;
	cout << "Regime   = " << base_regime(value, es) << endl;
	cout << "Exponent = " << base_exponent(msb, es) << endl;
	cout << "Fraction = 0x" << hex << base_fraction(value) << dec << endl;
}

void PositIntegerConversion() {
	cout << "Conversion examples: (notice the rounding errors)" << endl;
	posit<5, 1> p1;
	int64_t value;
	cout << "Rounding mode : " << p1.RoundingMode() << endl;
	value =  1;  p1 = value;	cout << "value: " << setw(2) << value << " -> posit: " << p1 << endl;
	value =  2;  p1 = value;	cout << "value: " << setw(2) << value << " -> posit: " << p1 << endl;
	value =  3;  p1 = value;	cout << "value: " << setw(2) << value << " -> posit: " << p1 << endl;
	value =  4;  p1 = value;	cout << "value: " << setw(2) << value << " -> posit: " << p1 << endl;
	value =  5;  p1 = value;	cout << "value: " << setw(2) << value << " -> posit: " << p1 << endl;
	value =  7;  p1 = value;	cout << "value: " << setw(2) << value << " -> posit: " << p1 << endl;
	value =  8;  p1 = value;	cout << "value: " << setw(2) << value << " -> posit: " << p1 << endl;
	value = 15;  p1 = value;	cout << "value: " << setw(2) << value << " -> posit: " << p1 << endl;
	cout << endl;
}

void ReportPositScales() {
	// print scales of different posit configurations
	// useed = 2^(2^es) and thus is just a function of the exponent configuration
	// maxpos = useed^(nbits-2)
	// minpos = useed^(2-nbits)
	posit<4, 0> p4_0;
	posit<8, 0> p8_0;
	posit<16, 1> p16_1;
	posit<32, 2> p32_2;
	posit<64, 3> p64_3;
	cout << "Posit specificiation examples and their ranges:" << endl;
	cout << spec_to_string(p4_0) << endl;
	cout << spec_to_string(p8_0) << endl;
	cout << spec_to_string(p16_1) << endl;
	cout << spec_to_string(p32_2) << endl;
	cout << spec_to_string(p64_3) << endl;
	cout << endl;
}

bool TestPositInitialization() {
	// posit initialization and assignment
	bool bValid = false;
	cout << "Posit copy constructor, assignment, and test" << endl;
	posit<16, 1> p16_1_1;
	p16_1_1 = (1 << 16);
	posit<16, 1> p16_1_2(p16_1_1);
	if (p16_1_1 != p16_1_2) {
		cerr << "Copy constructor failed" << endl;
		cerr << "value: " << (1 << 16) << " posits " << p16_1_1 << " == " << p16_1_2 << endl;
	}
	else {
		cout << "PASS" << endl;
		bValid = true;
	}
	cout << endl;
	return bValid;
}

void PositFloatConversion()
// posit float conversion
{
	cout << "Posit float conversion" << endl;
	float value;
	posit<4, 0> p1;
	cout << "Rounding mode : " << p1.RoundingMode() << endl;
	value = 0.0;  p1 = value;	cout << "value: " << setw(2) << value << " -> posit: " << p1 << endl;
	value = 0.25;  p1 = value;	cout << "value: " << setw(2) << value << " -> posit: " << p1 << endl;
	value = 0.5;  p1 = value;	cout << "value: " << setw(2) << value << " -> posit: " << p1 << endl;
	value = 0.75;  p1 = value;	cout << "value: " << setw(2) << value << " -> posit: " << p1 << endl;
	value = 1.0;  p1 = value;	cout << "value: " << setw(2) << value << " -> posit: " << p1 << endl;
	value = 1.5;  p1 = value;	cout << "value: " << setw(2) << value << " -> posit: " << p1 << endl;
	value = 2.0;  p1 = value;	cout << "value: " << setw(2) << value << " -> posit: " << p1 << endl;
	value = 4.0;  p1 = value;	cout << "value: " << setw(2) << value << " -> posit: " << p1 << endl;
	value = INFINITY;  p1 = value;	cout << "value: " << setw(2) << value << " -> posit: " << p1 << endl;
	
	cout << endl;
}

int main()
{
	ReportPositScales();
	PositIntegerConversion();
	PositFloatConversion();
	if (!TestPositInitialization()) {
		cerr << "initialization failed" << endl;
	}
	ConversionExamplesPositiveRegime();
	ConversionExamplesNegativeRegime();
	return 0;
}


