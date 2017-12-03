// fir_filter.cpp example program showing a FIR filter using error-free custom posit configurations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// enable the mathematical constants in cmath
#define _USE_MATH_DEFINES
#include "stdafx.h"

#include <vector>
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

const double pi = 3.14159265358979323846;  // best practice for C++

int main(int argc, char** argv)
try {
	const size_t nbits = 16;
	const size_t es = 1;
	const size_t vecSize = 32;
	int nrOfFailedTestCases = 0;

	posit<nbits, es> p;
	vector< posit<nbits, es> > sinusoid(vecSize), weights(vecSize);

	for (int i = 0; i < vecSize; i++) {
		sinusoid[i] = sin((float(i) / float(vecSize)) *2.0 * pi);

		weights[i] = 0.5f;
	}

	// dot product
	posit<nbits, es> fir;
	fir = 0.0f;
	for (int i = 0; i < vecSize; i++) {
		fir += sinusoid[i] * weights[i];
	}
	cout << "Value is " << fir << endl;

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
