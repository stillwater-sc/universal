// lu_decomposition.cpp example program comparing float vs posit equation solver
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// enable the mathematical constants in cmath: old-style preprocessor magic which isn't best practice anymore
#include "stdafx.h"

#include <vector>
#include <posit>

using namespace std;

int main(int argc, char** argv)
try 
{
	const size_t nbits = 16;
	const size_t es = 1;
	const size_t vecSize = 32;

	posit<nbits, es> p;
	vector< posit<nbits,es> > sinusoid(vecSize), cosinusoid(vecSize);

	// dot product
	posit<nbits, es> dot_product;
	dot_product = 0.0f;
	for (int i = 0; i < vecSize; i++) {
		dot_product += sinusoid[i] * cosinusoid[i];
	}

	cout << "Dot product is " << dot_product << endl;

	return 0;

}
catch (char* msg) {
	cerr << msg << endl;
	return 1;
}
