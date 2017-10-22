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
	const size_t nbits = 4;
	const size_t es = 0;
	const size_t size = 128;

	posit<nbits,es> myPosit;

	cout << "Fraction tests" << endl;


	return 0;
}
catch (char* msg) {
	cerr << msg << endl;
	return 1;
}
