// l1_fused_dot.cpp: example program showing a fused-dot product for error free linear algebra
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// enable the mathematical constants in cmath: old-style preprocessor magic which isn't best practice anymore
#define _USE_MATH_DEFINES
#include "stdafx.h"

#include <vector>

#define POSIT_TRACE_DEBUG
#define POSIT_TRACE_MUL
#include <posit>

#include "blas.hpp"

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
	typename std::vector<Ty>::const_iterator it = min_element(samples.begin(), samples.end());
	return *it;
}
template<typename Ty>
Ty maxValue(const std::vector<Ty>& samples) {
	typename std::vector<Ty>::const_iterator it = max_element(samples.begin(), samples.end());
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
	for (typename std::vector<Ty>::const_iterator it = samples.begin(); it != samples.end(); it++) {
		ostr << setw(3) << cnt++ << " " << setw(6) << float(*it) << " ";
		DisplaySample(ostr, *it, min, max);
	}
	ostr << setprecision(17);
}

int main(int argc, char** argv)
try {
	static constexpr size_t nbits = 16;
	static constexpr size_t es = 1;
	static constexpr size_t capacity = 6;   // 2^3 accumulations of maxpos^2
	static constexpr size_t vecSizePwr = 5;
	static constexpr size_t vecSize = (size_t(1) << vecSizePwr);

	int nrOfFailedTestCases = 0;

	float f;
	vector< float > funiform(vecSize), fminpos(vecSize), fmaxpos(vecSize);
	posit<nbits, es> p;
	vector< posit<nbits,es> > puniform(vecSize), pminpos(vecSize), pmaxpos(vecSize);

	for (int i = 0; i < vecSize; i++) {
		puniform[i] = funiform[i] = 1.0f;
		pminpos[i] = sw::unum::minpos_value<nbits, es>();
		pmaxpos[i] = sw::unum::maxpos_value<nbits, es>();

		fminpos[i] = (float)pminpos[i];
		fmaxpos[i] = (float)pmaxpos[i];
	}

	int width = std::numeric_limits<double>::max_digits10;
	int minp_scale = sw::unum::minpos_scale<nbits, es>();
	int maxp_scale = sw::unum::maxpos_scale<nbits, es>();
	int minpos_dot_product_scale = vecSizePwr + minp_scale;
	int maxpos_dot_product_scale = vecSizePwr + maxp_scale;
	//cout << fixed << setprecision(width);
	cout << "posit<" << nbits << ", " << es << ">  quire<" << nbits << ", " << es << ", " << capacity << ">" << endl;
	cout << "Vector size                      " << setw(width + 2) << vecSize << endl;
	cout << "Reference uniform dot uniform    " << setw(width + 2) << vecSize << endl;
	cout << "Reference minpos   scale         " << setw(width + 2) << minp_scale << endl;
	cout << "Reference minpos^2 scale of dot  " << setw(width + 2) << minpos_dot_product_scale << endl;
	cout << "Reference maxpos   scale         " << setw(width + 2) << maxp_scale << endl;
	cout << "Reference maxpos^2 scale of dot  " << setw(width + 2) << maxpos_dot_product_scale << endl;

	quire<nbits, es, capacity> q;
	cout << "Fused dot products" << endl;
	sw::blas::fused_dot(q, vecSize, puniform, 1, puniform, 1);
	cout << "uniform * uniform   " << setw(23) << q.to_value() << endl;
	q.clear();
	sw::blas::fused_dot(q, vecSize, pminpos, 1, pminpos, 1);
	cout << "minpos dot minpos   " << setw(23) << q.to_value() << endl;
	q.clear();
	sw::blas::fused_dot(q, vecSize, pmaxpos, 1, pmaxpos, 1);
	cout << "maxpos dot maxpos   " << setw(23) << q.to_value() << endl;
	q.clear();
	sw::blas::fused_dot(q, vecSize, pminpos, 1, pmaxpos, 1);  // each product equals 1
	cout << "minpos dot maxpos   " << setw(23) << q.to_value() << endl;

	f = sw::blas::dot(vecSize, funiform, 1, funiform, 1);
	cout << "Regular Dot product is " << setw(23) << f << endl;

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
