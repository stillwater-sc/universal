// l1_dot.cpp: example program contrasting a BLAS L1 ?dot routine between FLOAT and POSIT
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include <vector>
#include <posit>

#include "blas.hpp"

using namespace std;

template<typename element_T>
void randomVectorFill(size_t n, std::vector<element_T>& vec) {
	for (size_t i = 0; i < n; i++) {
		int rnd1 = rand();
		int rnd2 = rand();
		double rnd = rnd1 / (double)rnd2;
		vec[i] = (element_T)rnd;
	}
}

// generate a vector of random permutations around 1.0
// contraction is a right shift of the random variable causing smaller fluctuations
// RAND_MAX is typically a 16bit number so can't contract more than 15 bits
template<typename element_T>
void randomVectorFillAroundOneEPS(size_t n, std::vector<element_T>& vec, size_t contraction = 6) {
	for (size_t i = 0; i < n; i++) {
		int rnd1 = (rand() - (RAND_MAX >> 1)) >> contraction;
		double eps = rnd1 / (double)RAND_MAX;
		double v = 1.0 + eps;
		vec[i] = (element_T)v;
	}
}

// if samples is 0, all elements are printed
template<typename element_T>
void sampleVector(std::string vec_name, std::vector<element_T>& vec, uint32_t start = 0, uint32_t incr = 1, uint32_t nrSamples = 0) {
	cout << "Vector sample is: " << endl;
	if (nrSamples) {
		uint32_t printed = 0;
		for (uint32_t i = start; i < vec.size(); i += incr) {
			if (printed < nrSamples) {
				printed++;
				cout << vec_name << "[" << setw(3) << i << "] = " << setprecision(15) << vec[i] << endl;
			}
		}
	}
	else {
		for (uint32_t i = start; i < vec.size(); i += incr) {
			cout << vec_name << "[" << setw(3) << i << "] = " << setprecision(15) << vec[i] << endl;
		}
	}
}

int main(int argc, char** argv)
try {
	const size_t nbits = 8;
	const size_t es = 0;
	const size_t vecSize = 32;

	int nrOfFailedTestCases = 0;

	cout << "DOT product examples" << endl;
	vector<float> x(vecSize), y(vecSize);
	double result;

	randomVectorFillAroundOneEPS(vecSize, x);  //	sampleVector("x", x);
	randomVectorFillAroundOneEPS(vecSize, y);  // 	sampleVector("y", y);
	result = blas::dot<double,float>(vecSize, x, 1, y, 1);
	cout << "DOT product is " << setprecision(20) << result << endl;

	using Posit = posit<nbits, es>;
	vector<Posit> px(vecSize), py(vecSize);
	randomVectorFillAroundOneEPS(vecSize, px);  //	sampleVector("px", px);
	randomVectorFillAroundOneEPS(vecSize, py);  // 	sampleVector("py", py);
	result = blas::dot<double, Posit>(vecSize, px, 1, py, 1);
	cout << "DOT product is " << setprecision(20) << result << endl;
	sampleVector("px", px);  // <-- currently shows bad conversions....

	Posit p;
	p = 1.001; cout << p << endl;
	p = 0.999; cout << p << endl;

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
