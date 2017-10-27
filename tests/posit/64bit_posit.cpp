// 64bit_posit.cpp: Functionality tests for standard 64-bit posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"
#include "../../posit/posit_manipulators.hpp"
#include "../tests/test_helpers.hpp"
#include "../tests/posit_test_helpers.hpp"

using namespace std;

/*
Standard posits with nbits = 64 have 3 exponent bits.
*/

int main(int argc, char** argv)
try {
	int nrOfFailedTestCases = 0;
	const size_t nbits = 32;
	const size_t es = 2;
	posit<nbits, es> p;

	cout << "Standard posit<64,3> configuration tests" << endl;

	cout << spec_to_string(p) << endl;

	nrOfFailedTestCases++; // we don't have any yet
	return nrOfFailedTestCases;
}
catch (char* e) {
	cerr << e << endl;
	return -1;
}
