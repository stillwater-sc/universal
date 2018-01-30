// l1_fused_dot.cpp: example program showing a fused-dot product for error free linear algebra
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// enable the mathematical constants in cmath: old-style preprocessor magic which isn't best practice anymore
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

template<typename Ty>
Ty minValue(const std::vector<Ty>& samples) {
	std::vector<Ty>::const_iterator it = min_element(samples.begin(), samples.end());
	return *it;
}
template<typename Ty>
Ty maxValue(const std::vector<Ty>& samples) {
	std::vector<Ty>::const_iterator it = max_element(samples.begin(), samples.end());
	return *it;
}
template<typename Ty>
ostream& DisplaySample(ostream& ostr, const Ty& value, const Ty& min, const Ty& max) {
	// min is 0 stars
	// 0   is 40 stars
	// max is 80 stars
	int maxStars = 80;
	float sample = float(value);
	float range = float(max - min);
	float midPoint = range/2.0f;
	float portion = (sample + midPoint)/range;
	int ub = int(maxStars * portion);
	for (int i = 0; i < ub; i++) {
		ostr << "*";
	}
	return ostr << std::endl;
}

template<typename Ty>
void DisplaySignal(ostream& ostr, const std::vector<Ty>& samples) {
	Ty min = minValue(samples);
	Ty max = maxValue(samples);
	// create a horizontal display
	int cnt = 0;
	ostr << fixed << setprecision(3);
	for (std::vector<Ty>::const_iterator it = samples.begin(); it != samples.end(); it++) {
		ostr << setw(3) << cnt++ << " " << setw(6) << float(*it) << " ";
		DisplaySample(ostr, *it, min, max);
	}
	ostr << setprecision(17);
}

int main(int argc, char** argv)
try {
	const size_t nbits = 16;
	const size_t es = 1;
	const size_t vecSize = 32;

	int nrOfFailedTestCases = 0;

	posit<nbits, es> p;
	vector< posit<nbits,es> > sinusoid(vecSize), cosinusoid(vecSize);

	for (int i = 0; i < vecSize; i++) {
		p = sin( (float(i) / float(vecSize)) *2.0 * pi);
		sinusoid[i] = p;
		p = cos((float(i) / float(vecSize)) *2.0 * pi);
		cosinusoid[i] = p;
	}

	DisplaySignal(std::cout, sinusoid);

	minpos_value<nbits, es>();
	// dot product
	quire<nbits, es, 2> dot_product;
	dot_product = 0.0f;
	for (int i = 0; i < vecSize; i++) {
		dot_product += quire_mul(sinusoid[i], cosinusoid[i]);
	}

	cout << "Dot product is " << dot_product << endl;

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
catch (...) {
	cerr << "Caught unknown exception" << endl;
	return EXIT_FAILURE;
}
