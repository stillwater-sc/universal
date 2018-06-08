// extract.cpp : extracting IEEE floating point components and relate them to posit components
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "common.hpp"
#include <posit>

using namespace std;
using namespace sw::unum;

/*
Laid out as bits, floating point numbers look like this:
Single: SEEEEEEE EMMMMMMM MMMMMMMM MMMMMMMM
Double: SEEEEEEE EEEEMMMM MMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM

1.	The sign bit is 0 for positive, 1 for negative.
2.	The exponent base is two.
3.	The exponent field contains 127 plus the true exponent for single-precision, or 1023 plus the true exponent for double precision.
4.	The first bit of the mantissa is typically assumed to be 1.f, where f is the field of fraction bits.

*/

#define FLOAT_SIGN_MASK      0x80000000
#define FLOAT_EXPONENT_MASK  0x7F800000
#define FLOAT_MANTISSA_MASK  0x007FFFFF

#define FLOAT_ALTERNATING_BITS_SIGNIFICANT_5 0x00555555
#define FLOAT_ALTERNATING_BITS_SIGNIFICANT_A 0x002AAAAA

#define DOUBLE_SIGN_MASK     0x8000000000000000
#define DOUBLE_EXPONENT_MASK 0x7FF0000000000000
#define DOUBLE_MANTISSA_MASK 0x000FFFFFFFFFFFFF

#define DOUBLE_ALTERNATING_BITS_SIGNIFICANT_5 0x0005555555555555
#define DOUBLE_ALTERNATING_BITS_SIGNIFICANT_A 0x000AAAAAAAAAAAAA

/*
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
	constexpr size_t fbits = posit<nbits, es>::fbits;
	posit<nbits, es> p;
	bool		 _sign;
	int			 _scale;
	float		 _fr;
	uint32_t	 _23b_fraction_without_hidden_bit;

	extract_fp_components(f, _sign, _scale, _fr, _23b_fraction_without_hidden_bit);
	bitblock<fbits> _fraction = extract_23b_fraction<fbits>(_23b_fraction_without_hidden_bit);

	p.convert(_sign, _scale, _fraction);
	return p;
}

template<size_t nbits, size_t es>
posit<nbits, es> extract(double d) {
	constexpr size_t fbits = posit<nbits, es>::fbits;
	posit<nbits, es> p;
	bool				_sign;
	int					_scale;
	double				_fr;
	unsigned long long	_52b_fraction_without_hidden_bit;

	extract_fp_components(d, _sign, _scale, _fr, _52b_fraction_without_hidden_bit);
	bitblock<fbits> _fraction = extract_52b_fraction<fbits>(_52b_fraction_without_hidden_bit);

	p.convert(_sign, _scale, _fraction);
	return p;
}

int main()
try {
	const size_t nbits = 32;
	const size_t es = 2;

	posit<nbits,es>		p;
	bool 				sign;
	int					exponent;
	float				ffr;
	unsigned int		ulfraction;
	double				dfr;
	unsigned long long	ullfraction;
	bitblock<nbits>     _fraction;

	cout << "Conversion tests" << endl;

	union {
		float f;
		unsigned int i;
	} uf;


	uf.i = FLOAT_ALTERNATING_BITS_SIGNIFICANT_5 | !FLOAT_SIGN_MASK;
	cout << "Positive Regime: float value: " << uf.f << endl;
	extract_fp_components(uf.f, sign, exponent, ffr, ulfraction);
	_fraction = extract_23b_fraction<nbits>(ulfraction);
	cout << "f " << uf.f << " sign " << (sign ? -1 : 1) << " exponent " << exponent << " fraction " << ulfraction << endl;

	p = extract<nbits, es>(uf.f);
	cout << "posit<" << nbits << "," << es << "> = " << p << endl;
	cout << "posit<" << nbits << "," << es << "> = " << components_to_string(p) << endl;


	uf.i = FLOAT_ALTERNATING_BITS_SIGNIFICANT_5 | FLOAT_SIGN_MASK;
	cout << "Negative Regime: float value: " << uf.f << endl;
	extract_fp_components(uf.f, sign, exponent, ffr, ulfraction);
	_fraction = extract_23b_fraction<nbits>(ulfraction);
	cout << "f " << uf.f << " sign " << (sign ? -1 : 1) << " exponent " << exponent << " fraction " << ulfraction << endl;

	p = extract<nbits, es>(uf.f);
	cout << "posit<" << nbits << "," << es << "> = " << p << endl;
	cout << "posit<" << nbits << "," << es << "> = " << components_to_string(p) << endl;

	union {
		double d;
		unsigned long long i;
	} ud;


	ud.i = DOUBLE_ALTERNATING_BITS_SIGNIFICANT_5 | !DOUBLE_SIGN_MASK;
	cout << "Positive Regime: float value: " << ud.d << endl;
	extract_fp_components((double)ud.d, sign, exponent, dfr, ullfraction);
	_fraction = extract_52b_fraction<nbits>(ullfraction);
	cout << "d " << ud.d << " sign " << (sign ? -1 : 1) << " exponent " << exponent << " fraction " << ullfraction << endl;

	p = extract<nbits, es>(ud.d);
	cout << "posit<" << nbits << "," << es << "> = " << p << endl;
	cout << "posit<" << nbits << "," << es << "> = " << components_to_string(p) << endl;


	ud.i = DOUBLE_ALTERNATING_BITS_SIGNIFICANT_5 | DOUBLE_SIGN_MASK;
	cout << "Negative Regime: float value: " << ud.d << endl;
	extract_fp_components((double)ud.d, sign, exponent, dfr, ullfraction);
	_fraction = extract_52b_fraction<nbits>(ullfraction);
	cout << "d " << ud.d << " sign " << (sign ? -1 : 1) << " exponent " << exponent << " fraction " << ullfraction << endl;

	p = extract<nbits, es>(uf.f);
	cout << "posit<" << nbits << "," << es << "> = " << p << endl;
	cout << "posit<" << nbits << "," << es << "> = " << components_to_string(p) << endl;

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
catch (...) {
	cerr << "Caught unknown exception" << endl;
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

