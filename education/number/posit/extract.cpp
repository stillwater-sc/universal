// extract.cpp : extracting IEEE floating point components and relate them to posit components
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/posit/posit.hpp>

/*
Laid out as bits, floating point numbers look like this:
Single: SEEEEEEE EMMMMMMM MMMMMMMM MMMMMMMM
Double: SEEEEEEE EEEEMMMM MMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM MMMMMMMM

1.	The sign bit is 0 for positive, 1 for negative.
2.	The exponent base is two.
3.	The exponent field contains 127 plus the true exponent for single-precision, or 1023 plus the true exponent for double precision.
4.	The first bit of the mantissa is typically assumed to be 1.f, where f is the field of fraction bits.

*/

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
and stores an integer value in *exp such that x*2(*exp)=arg
If the value to be stored in *exp is outside the range of int, the behavior is unspecified.
If arg is not a floating-point number, the behavior is unspecified.

float frexp(float in, int* exponent)
double frexp(double in, int* exponent)
long double frexp(long double in, int* exponent)

*/

int main()
try {
	using namespace sw::universal;

	const unsigned nbits = 32;
	const unsigned es = 2;

	posit<nbits,es>		p;

	std::cout << "Extraction examples\n";
	std::cout << "Using blocktriple to display IEEE-754 decomposition (sign, scale, significand)\n";
	std::cout << "and showing the resulting posit encoding\n\n";

	// blocktriple<N, REP> decomposes IEEE-754 values into (sign, scale, significand)
	// Use N > source mantissa bits to avoid a known round() edge case
	blocktriple<24, BlockTripleOperator::REP, uint8_t> fbt;  // for float (23 mantissa bits)
	blocktriple<53, BlockTripleOperator::REP, uint8_t> dbt;  // for double (52 mantissa bits)

	// Float extraction examples
	std::cout << "--- Float to posit<" << nbits << "," << es << "> ---\n";
	float test_floats[] = { 1.5f, -1.5f, 3.14159f, -0.125f, 1024.0f, 0.001f };
	for (int i = 0; i < 6; ++i) {
		float f = test_floats[i];
		fbt = f;
		p = f;  // posit uses direct frexp-based conversion
		std::cout << "float " << std::setw(12) << f
		          << " -> " << to_triple(fbt)
		          << " -> posit " << p
		          << '\n';
	}

	std::cout << '\n';

	// Double extraction examples
	std::cout << "--- Double to posit<" << nbits << "," << es << "> ---\n";
	double test_doubles[] = { 1.5, -1.5, 3.141592653589793, -0.125, 1024.0, 0.001 };
	for (int i = 0; i < 6; ++i) {
		double d = test_doubles[i];
		dbt = d;
		p = d;  // posit uses direct frexp-based conversion
		std::cout << "double " << std::setw(20) << std::setprecision(15) << d
		          << " -> " << to_triple(dbt)
		          << " -> posit " << p
		          << '\n';
	}

	std::cout << '\n';

	// Show detailed posit encoding for a few values
	std::cout << "--- Detailed posit encoding ---\n";
	double detailed[] = { 1.0, 0.5, -1.0, 3.14159265358979, 256.0, 0.001 };
	for (int i = 0; i < 6; ++i) {
		p = detailed[i];
		dbt = detailed[i];
		std::cout << std::setprecision(15) << detailed[i] << '\n';
		std::cout << "  blocktriple: " << to_triple(dbt) << '\n';
		std::cout << "  posit:       " << components(p) << '\n';
	}

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
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
