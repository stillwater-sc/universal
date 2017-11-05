// enumeration.cpp: functional tests for enumerating the posit state space
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include "../../posit/posit_regime_lookup.hpp"
#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"
#include "../../posit/posit_manipulators.hpp"

using namespace std;


int main(int argc, char** argv)
try {
	int nrOfFailedTestCases = 0;

	posit<4, 0> pa, pb, psum;
	pa = 0.25f;
	pb = 0.5f;
	psum = pb + pa;
	cout << components_to_string(psum) << endl;
	
	return 0;

	const size_t nbits = 5;
	const size_t es = 1;
	// generate minpos
	posit<nbits, es> p_minpos(uint64_t(0));
	p_minpos++; // next posit to ZERO

	// generate maxpos  FAILING....
	float inf = FP_INFINITE;
	posit<5,1> p_maxpos(inf);
	p_maxpos = inf;
	cout << components_to_string(p_maxpos) << endl;
	p_maxpos--; // previous posit to INFINITE
	p_maxpos.to_double();
	cout << components_to_string(p_maxpos) << endl;

	cout << "minpos : " << p_minpos << " maxpos : " << p_maxpos << " "  << endl;

	cout << "State space enumeration" << endl;
	posit<nbits, es> p(uint64_t(0));
	for (int i = 0; i < uint32_t(1) << nbits; i++) {
		cout << components_to_string(p) << endl;
		p++;
	}
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
