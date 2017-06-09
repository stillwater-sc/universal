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


int main()
{
	posit<16,2> myPosit;

	union {
	    float myFloat;
	    uint32_t value;
	};
	myFloat = 1.0f;
	extract<16,2>(myFloat, 8, 23);

	return 0;
}



