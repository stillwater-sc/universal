// skeleton.cpp example showing the basic program structure to use custom posit configurations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// enable the mathematical constants in cmath
#define _USE_MATH_DEFINES
#include "stdafx.h"

#include <posit>

using namespace std;
using namespace sw::unum;

/*

Mathematical 	C++ Symbol	Decimal Representation
Expression
pi				M_PI		3.14159265358979323846
pi/2			M_PI_2		1.57079632679489661923
pi/4			M_PI_4		0.785398163397448309616
1/pi			M_1_PI		0.318309886183790671538
2/pi			M_2_PI		0.636619772367581343076
2/sqrt(pi)		M_2_SQRTPI	1.12837916709551257390
sqrt(2)			M_SQRT2		1.41421356237309504880
1/sqrt(2)		M_SQRT1_2	0.707106781186547524401
e				M_E			2.71828182845904523536
log_2(e)		M_LOG2E		1.44269504088896340736
log_10(e)		M_LOG10E	0.434294481903251827651
log_e(2)		M_LN2		0.693147180559945309417
log_e(10)		M_LN10		2.30258509299404568402

*/

constexpr double pi = 3.14159265358979323846;  // best practice for C++11

int main(int argc, char** argv)
try {
	bool bSuccess = true;

	{
		constexpr size_t nbits = 8;
		constexpr size_t es = 1;

		posit<nbits, es> p;

		// assign PI to posit<8,1>
		p = M_PI;
		cout << "posit<8,1> value of PI    = " << p << " " << pretty_print(p) << endl;

		// convert posit back to float
		float f = float(p);
		cout << "float value               = " << f << endl;

		// calculate PI/2
		p = p / posit<nbits, es>(2.0);  // explicit conversions of literals
		cout << "posit<8,1> value of PI/2  = " << p << " " << pretty_print(p) << endl;
	}
	cout << endl;
	{
		constexpr size_t nbits = 16;
		constexpr size_t es = 1;

		posit<nbits, es> p;

		// assign PI to posit<16,1>
		p = M_PI;
		cout << "posit<16,1> value of PI   = " << p << " " << pretty_print(p) << endl;

		// convert posit back to float
		float f = float(p);
		cout << "float value               = " << f << endl;

		// calculate PI/2
		p = p / posit<nbits, es>(2.0);  // explicit conversions of literals
		cout << "posit<16,1> value of PI/2 = " << p << " " << pretty_print(p) << endl;
	}

	return (bSuccess ? EXIT_SUCCESS : EXIT_FAILURE);
}
catch (char const* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
catch (...) {
	cerr << "Caught unknown exception" << endl;
	return EXIT_FAILURE;
}
