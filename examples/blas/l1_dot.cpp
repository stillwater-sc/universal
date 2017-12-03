// l1_dot.cpp: example program contrasting a BLAS L1 ?dot routine between FLOAT and POSIT
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include <vector>
#include <posit>

#include "blas.hpp"
#include "blas_utils.hpp"

using namespace std;
using namespace sw::blas;

int main(int argc, char** argv)
try {
	const size_t nbits = 32;
	const size_t es = 2;
	const size_t vecSize = 32;

	int nrOfFailedTestCases = 0;

	cout << "DOT product examples" << endl;
	vector<float> x(vecSize), y(vecSize);
	float fresult;

	randomVectorFillAroundOneEPS(vecSize, x);  //	sampleVector("x", x);
	randomVectorFillAroundOneEPS(vecSize, y);  // 	sampleVector("y", y);
	fresult = sw::blas::dot<float,float>(vecSize, x, 1, y, 1);
	cout << "DOT product is " << setprecision(20) << fresult << endl;

	using Posit = sw::unum::posit<nbits, es>;
	vector<Posit> px(vecSize), py(vecSize);
	Posit presult;
	randomVectorFillAroundOneEPS(vecSize, px);  //	sampleVector("px", px);
	randomVectorFillAroundOneEPS(vecSize, py);  // 	sampleVector("py", py);
	presult = sw::blas::dot<Posit, Posit>(vecSize, px, 1, py, 1);
	cout << "DOT product is " << setprecision(20) << presult << endl;
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