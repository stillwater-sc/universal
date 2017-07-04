//  extract.cpp : extract a posit from a float
//

#include "stdafx.h"

#include "../../posit/posit_scale_factors.hpp"
#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"

using namespace std;

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
void extract(float f) {
	int fes = 8;
	int fms = 23;
	uint32_t bits = *( (uint32_t*)(&f) );

	int exponentBias = two_to_the_power(fes - 1) - 1;
	int16_t biased_exponent = (bits >> fms) & ((bits << fes) - 1);
	uint32_t mantissa = (bits & ((1ULL << fms) - 1));
	int16_t exponent = biased_exponent - exponentBias;
	int sign = 1;
	if (0x80000000L & bits) {
		sign = -1;
	}
	// Emin = 01H−7FH = −126
	// Emax = FEH−7FH = 127
	exponent = (exponent < -126 ? -126 : exponent);
	exponent = (exponent > 127 ? 127 : exponent);
	cout << " bits : " << setw(8) << hex << bits << " sign " << sign << " mantissa : " << hex << mantissa << " exponent " << exponent << dec << endl;

	int16_t regime = exponent >> es;
	exponent = exponent & ((1 << es) - 1);

	uint32_t fraction = (bits << (7 - es) & (0x3FFFFFFFL >> es));
	fraction |= ((uint32_t)exponent) << (30 - es);

	int16_t shift;
	if (regime >= 0) {
		shift = 1 + regime;
		fraction |= 0x80000000UL;
	}
	else {
		shift = -regime;
		fraction |= 0x40000000UL;
	}
	// perform an *arithmetic* shift; convert back to unsigned.
	fraction = (uint32_t)(((int32_t)fraction) >> shift);

	// mask out the top bit of the fraction, which is going to be the basis for the result.
	fraction = fraction & 0x7fffffffL;

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

}
int main()
{
	const size_t nbits = 5;
	const size_t es = 1;
	const size_t size = (1 << nbits);
	posit<nbits,es> myPosit;
	float myFloat = 1.0f;
	for (int i; i < size; i++) {
		cout << "Value " << setw(16) << setprecision(6) << myFloat << " ";
		extract<16, 2>(myFloat);
		myFloat *= 2.0f;
	}



	return 0;
}



