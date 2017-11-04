//  exponents.cpp : tests on posit exponents
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"
#include "../../posit/posit_manipulators.hpp"

using namespace std;

template<size_t es>
int ValidateExponentOperations() {
	int nrOfFailedTestCases = 0;

	return nrOfFailedTestCases;
}

int main()
try {
	const size_t nbits = 5;
	const size_t es = 0;

	int nrOfFailedTestCases = 0;

	posit<nbits,es> pa, pb, psum, pres;
	float fa, fb, fres;
	fa = -0.125f;
	fb = 1.5f;
	pa = fa;
	pb = fb;
	fres = fa + fb;
	psum = pa + pb;
	pres = fres;
	cout << pa << " " << pb << " " << psum << " " << pres << " " << fres << endl;

	cout << components_to_string(pa) << endl;
	cout << components_to_string(pb) << endl;
	cout << components_to_string(pres) << endl;

	cout << "Exponent tests" << endl;

	exponent<nbits, es> e1, e2;

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
