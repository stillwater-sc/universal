//  extract.cpp : extract a posit from a float
//

#include "stdafx.h"
#include <sstream>
#include "../../posit/posit_scale_factors.hpp"
#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"

using namespace std;

uint64_t regime_lookup[8] = {
	0x0ULL,
	0x1ULL,
	0x2ULL
};


/*
Laid out as bits, floating point numbers look like this:
Single: SEEEEEEE EMMMMMMM MMMMMMMM MMMMMMMM
Double: SEEEEEEE EEEEMMMM MMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM

1.	The sign bit is 0 for positive, 1 for negative.
2.	The exponent base is two.
3.	The exponent field contains 127 plus the true exponent for single-precision, or 1023 plus the true exponent for double precision.
4.	The first bit of the mantissa is typically assumed to be 1.f, where f is the field of fraction bits.

#define FLOAT_SIGN_MASK      0x80000000
#define FLOAT_EXPONENT_MASK  0x7F800000
#define FLOAT_MANTISSA_MASK  0x007FFFFF

#define DOUBLE_SIGN_MASK     0x8000000000000000
#define DOUBLE_EXPONENT_MASK 0x7FF0000000000000
#define DOUBLE_MANTISSA_MASK 0x000FFFFFFFFFFFFF
*/
template<size_t nbits, size_t es>
std::bitset<nbits> extract(float f) {
	int fes = 8;
	int fms = 23;
	uint32_t number_bits = *( (uint32_t*)(&f) );
	std::bitset<nbits> posit_bits;
	posit_bits.reset();

	int exponentBias = two_to_the_power(fes - 1) - 1;
	int16_t biased_exponent = (number_bits >> fms) & ((number_bits << fes) - 1);
	uint32_t mantissa = (number_bits & ((1ULL << fms) - 1));
	int16_t exponent = biased_exponent - exponentBias;
	int sign = 1;
	if (0x80000000L & number_bits) {
		sign = -1;
		posit_bits.set(nbits - 1);
	}
	// Emin = 01H−7FH = −126
	// Emax = FEH−7FH = 127
	exponent = (exponent < -126 ? -126 : exponent);
	exponent = (exponent > 127 ? 127 : exponent);
	//cout << "sign " << sign << " mantissa : " << hex << mantissa << " exponent " << exponent << dec ;

	int16_t regime = exponent >> es; 
	cout << " regime " << regime << endl;
	int16_t shift, msb;
	if (regime >= 0) {
		if (regime > nbits - 2) {
			float maxpos = pow((1 << (1 << es)), nbits - 2);
			char msg[256];
			sprintf(msg, "Value %9.6f is too large to be represented by posit<%d,%d> : maxpos = %9.6f", f, int(nbits), int(es), maxpos);
			throw  msg;
		}
		// these are patterns of s-1110## and at the extreme s-111111, that is, an ending with 1
		msb = nbits - 2;
		posit_bits.set(msb--);
		for (int i = 0; i < regime; i++) {
			posit_bits.set(msb--);
		}
		if (msb >= 0) {
			posit_bits.reset(msb--);
		}
		
		shift = 1 + regime;
		//fraction |= 0x80000000UL;
	}
	else {
		if (regime < 1 - nbits) {
			float minpos = pow(double(uint32_t(1) << (uint32_t(1) << es)), double(1.0 - double(nbits)));
			char msg[256];
			sprintf(msg, "Value %9.6f is too small to be represented by posit<%d,%d> : minpos = %9.6f", f, int(nbits), int(es), minpos);
			throw  msg;
		}
		// these are patterns of s-0001##, and at the extreme s-00000, that is, an ending with 0
		msb = nbits - 2;
		for (int i = 0; i < -regime; i++) {
			posit_bits.reset(msb--);
		}
		if (msb >= 0) {
			posit_bits.set(msb--);
		}
		shift = -regime;
		//fraction |= 0x40000000UL;
	}

	exponent = exponent & ((1 << es) - 1);
	if (es > 0) {
		uint32_t mask = (1 << es-1);
		for (int i = es-1; i >= 0, msb >= 0; --i) {
			posit_bits.set(msb--, mask & exponent);
			mask >>= 1;
		}
	}

	return posit_bits;
}

template<size_t nbits, size_t es>
void extract(double d) {
	int fes = 11;
	int fms = 52;
	uint64_t bits = *( (uint64_t*)(&d) );
	cout << "value : " << dec << d << " bits : " << hex << bits << " mantissa mask : " << (1ULL << fms) - 1 << dec << endl;

	int exponentBias = two_to_the_power(fes - 1) - 1;
	int16_t biased_exponent = (bits >> fms) & ((1 << fes) - 1);
	uint32_t mantissa = (bits & ((1ULL << fms) - 1));
	int16_t exponent = biased_exponent - exponentBias;
	int sign = 1;
	if (0x8000000L & bits) {
		sign = -1;
	}
	cout << "sign " << sign << " mantissa : " << hex << mantissa << " exponent " << exponent << dec << endl;

	uint32_t fraction = (number_bits << (7 - es) & (0x3FFFFFFFL >> es));
	fraction |= ((uint32_t)exponent) << (30 - es);

	//cout << "regime " << regime << " exponent: " << hex << exponent << " fraction: " << fraction;

	// perform an *arithmetic* shift; convert back to unsigned.
	fraction = (uint32_t)(((int32_t)fraction) >> shift);
	cout << " after shift of " << dec << shift << " " << hex << fraction;

	// mask out the top bit of the fraction, which is going to be the basis for the result.
	fraction = fraction & 0x7fffffffL;
	cout << " " << fraction;

	bool guard = (fraction & 0x00800000L) != 0;
	bool summ = (fraction & 0x007fffffL) != 0;
	bool inner = (fraction & 0x01000000L) != 0;

	// round the fraction variable in the event it needs be augmented.
	fraction += ((guard && inner) || (guard && summ)) ? 0x01000000UL : 0x00000000UL;

	// shift further, as necessary, to match sizes
	fraction = fraction >> 24;

	//check to recast zeros to the smallest value
	fraction = (fraction == 0) ? 0x00000001UL : fraction;

	uint8_t sfrac = (uint8_t)fraction;
	//(signbit ? -sfrac : sfrac);

	cout << " regime " << regime << " exponent " << exponent << " fraction " << fraction << endl;

}
int main()
try
{
	const size_t nbits = 5;
	const size_t es = 1;
	const size_t size = (1 << nbits);
	posit<nbits,es> myPosit;

	cout << "Conversion tests" << endl;

	union {
		float f;
		uint32_t v;
	};
	v = 0x55555555UL;

	cout << "Positive regime" << endl;
	try {
		f = 1.0f;
		for (int i; i < size; i++) {
			cout << "Value " << setw(16) << setprecision(6) << f << " ";
			std::bitset<nbits> raw_bits = extract<nbits, es>(f);
			myPosit.set(raw_bits);
			cout << "Posit = " << myPosit << endl;
			f *= 2.0f;
		}
	}
	catch (char* msg) {
		cerr << endl << msg << endl;
	}

	cout << "Negative Regime" << endl;
	try {
		f = 1.0f;
		for (int i; i < size; i++) {
			cout << "Value " << setw(16) << setprecision(6) << f << " ";
			std::bitset<nbits> raw_bits = extract<nbits, es>(f);
			myPosit.set(raw_bits);
			cout << "Posit = " << myPosit << endl;
			f /= 2.0f;
		}
	}
	catch (char* msg) {
		cerr << endl << msg << endl;
	}

	// regime
	// posit<3,#>
	// -2 s-00
	// -1 s-01
	//  0 s-10
	//  1 s-11

	// posit<4,#>
	// -3 s-000
	// -2 s-001
	// -1 s-01#
	//  0 s-10#
	//  1 s-110
	//  2 s-111

	// posit<5,#>
	// -4 s-0000
	// -3 s-0001
	// -2 s-001#
	// -1 s-01##
	//  0 s-10##
	//  1 s-110#
	//  2 s-1110
	//  3 s-1111

	// posit<6,#>
	// -5 s-00000
	// -4 s-00001
	// -3 s-0001#
	// -2 s-001##
	// -1 s-01###
	//  0 s-10###
	//  1 s-110##
	//  2 s-1110#
	//  3 s-11110
	//  4 s-11111

	// posit<7,#>
	// -6 s-000000
	// -5 s-000001
	// -4 s-00001#
	// -3 s-0001##
	// -2 s-001###
	// -1 s-01####
	//  0 s-10####
	//  1 s-110###
	//  2 s-1110##
	//  3 s-11110#
	//  4 s-111110
	//  5 s-111111

	// posit<8,#>
	// -7 s-0000000
	// -6 s-0000001
	// -5 s-000001#
	// -4 s-00001##
	// -3 s-0001###
	// -2 s-001####
	// -1 s-01#####
	//  0 s-10#####
	//  1 s-110####
	//  2 s-1110###
	//  3 s-11110##
	//  4 s-111110#
	//  5 s-1111110
	//  6 s-1111111

	return 0;
}
catch (char* msg) {
	cerr << msg << endl;
	return 1;
}

// REGIME BITS
//		posit<3,#>	posit<4,#>	posit<5,#>	posit<6,#>	posit<7,#>	posit<8,#>
// -7																s-0000000
// -6													s-000000	s-0000001
// -5										s-00000		s-000001	s-000001#
// -4							s-0000		s-00001		s-00001#	s-00001##
// -3				s-000		s-0001		s-0001#		s-0001##	s-0001###
// -2	s-00		s-001		s-001#		s-001##		s-001###	s-001####
// -1	s-01		s-01#		s-01##		s-01###		s-01####	s-01#####
//  0	s-10		s-10#		s-10##		s-10###		s-10####	s-10#####
//  1	s-11		s-110		s-110#		s-110##		s-110###	s-110####
//  2				s-111		s-1110		s-1110#		s-1110##	s-1110###
//  3							s-1111		s-11110		s-11110#	s-11110##
//  4										s-11111		s-111110	s-111110#
//  5													s-111111	s-1111110
//  6																s-1111111

