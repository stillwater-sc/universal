//  regimes.cpp : tests on posit regimes
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"
#include "../../posit/posit_manipulators.hpp"

using namespace std;

template<size_t nbits, size_t es>
int ValidateRegimeOperations() {
	int nrOfFailedTestCases = 0;

	return nrOfFailedTestCases;
}

int main()
try
{
	const size_t nbits = 5;
	const size_t es = 0;

	int nrOfFailedTests = 0;

	cout << "Regime tests" << endl;



	return nrOfFailedTests;
}
catch (char* msg) {
	cerr << msg << endl;
	return 1;
}
