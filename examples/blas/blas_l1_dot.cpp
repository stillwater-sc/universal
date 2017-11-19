// blas_l1_dot.cpp example program contrasting a BLAS L1 ?dot routine between FLOAT and POSIT
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
void randomFill(size_t n, std::vector<element_T>& vec) {
	for (size_t i = 0; i < n; i++) {
		int rnd1 = rand();
		int rnd2 = rand();
		double rnd = rnd1 / (double)rnd2;
		vec[i] = (element_T)rnd;
	}
}

int main(int argc, char** argv)
try {
	const size_t nbits = 16;
	const size_t es = 1;
	const size_t vecSize = 32;

	int nrOfFailedTestCases = 0;

	cout << "DOT product examples" << endl;
	vector<float> x(vecSize), y(vecSize);
	double result;

	randomFill(vecSize, x);
	randomFill(vecSize, y);
	result = blas::dot<double,vector<float>,vector<float>>(vecSize, x, 1, y, 1);
	cout << "DOT product is " << result << endl;

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
