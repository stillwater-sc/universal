// lu_decomposition.cpp example program comparing float vs posit equation solver
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// enable the mathematical constants in cmath: old-style preprocessor magic which isn't best practice anymore
#include "stdafx.h"

#include <iostream>
#include <typeinfo>

#include <posit>

using namespace std;

// Turn it of for now
#define USE_POSIT

int main(int argc, char** argv)
try {
	const size_t nbits = 16;
	const size_t es = 1;
	const size_t vecSize = 32;


	int nrOfFailedTestCases = 0;
	return nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
