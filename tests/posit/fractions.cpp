//  fractions.cpp : tests on posit fractions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"
#include "../../posit/posit_manipulators.hpp"

using namespace std;

int main()
try
{
	const size_t nbits = 3;
	const size_t es = 0;
	posit<3, 0> trouble;
	trouble = 1.0f;

	int nrOfFailedTestCases = 0;

	cout << "Fraction tests" << endl;

	nrOfFailedTestCases++; // we don't have any yet

	return nrOfFailedTestCases;
}
catch (char* msg) {
	cerr << msg << endl;
	return 1;
}
