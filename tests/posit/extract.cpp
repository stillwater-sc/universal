//  extract.cpp : extract a posit from a float
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include "../../posit/posit.hpp"
#include "../../posit/posit_manipulators.hpp"

using namespace std;
using namespace sw::unum;

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

In the Standard C++ library there are several functions that manipulate these components:
in the header <cmath>
frexp returns the exponent in exponent and the fraction in the return value
Parameters
arg	-	floating point value
exp	-	pointer to integer value to store the exponent to
Return value
If arg is zero, returns zero and stores zero in *exp.
Otherwise (if arg is not zero), if no errors occur, returns the value x in the range (-1;-0.5], [0.5; 1) 
and stores an integer value in *exp such that x×2(*exp)=arg
If the value to be stored in *exp is outside the range of int, the behavior is unspecified.
If arg is not a floating-point number, the behavior is unspecified.

float frexp(float in, int* exponent)
double frexp(double in, int* exponent)
long double frexp(long double in, int* exponent)

*/
template<size_t nbits, size_t es>
posit<nbits, es> extract(float f) {
	posit<nbits, es> p;
	bool _sign = extract_sign(f);
	int _scale = extract_exponent(f) - 1;		// exponent is for an unnormalized number 0.1234*2^exp
	uint32_t _23b_fraction_without_hidden_bit = extract_fraction(f);
	constexpr size_t fbits = p.fbits;
	std::bitset<fbits> _fraction = extract_float_fraction<fbits>(_23b_fraction_without_hidden_bit);

	p.convert(_sign, _scale, _fraction, fbits);
	return p;
}

int main()
try {
	const size_t nbits = 4;
	const size_t es = 0;
	const size_t size = 128;
	int nrOfFailedTestCases = 0;
	posit<nbits,es> myPosit;
	float f;
	bool sign;
	int exponent;
	uint32_t fraction;
	std::bitset<nbits> _fraction;

	cout << "Conversion tests" << endl;


	cout << "Positive regime" << endl;
	f = 4.0f;
	sign = extract_sign(f);
	exponent = extract_exponent(f);
	fraction = extract_fraction(f);
	_fraction = extract_float_fraction<nbits>(fraction);
	cout << "f " << f << "sign " << (sign ? -1 : 1) << " exponent " << exponent << " fraction " << fraction << endl;

	myPosit = extract<nbits, es>(f);
	cout << "posit<" << nbits << "," << es << "> = " << myPosit << endl;
	cout << "posit<" << nbits << "," << es << "> = " << components_to_string(myPosit) << endl;


	cout << "Negative Regime" << endl;
	f = -4.0f;
	sign = extract_sign(f);
	exponent = extract_exponent(f);
	fraction = extract_fraction(f);
	_fraction = extract_float_fraction<nbits>(fraction);
	cout << "f " << f << "sign " << (sign ? -1 : 1) << " exponent " << exponent << " fraction " << fraction << endl;

	myPosit = extract<nbits, es>(f);
	cout << "posit<" << nbits << "," << es << "> = " << myPosit << endl;
	cout << "posit<" << nbits << "," << es << "> = " << components_to_string(myPosit) << endl;


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

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
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

