// divide.cpp: functional tests for division
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include "../../bitset/bitset_helpers.hpp"
#include "../../posit/posit_regime_lookup.hpp"
#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"
#include "../../posit/posit_manipulators.hpp"

using namespace std;


int main(int argc, char** argv)
try 
{
	posit<5,1> pa, pb, pdiv;
	pa = 1.0f;
	pb = 1.0f;
	pdiv = pa / pb;
	cout << pa << " " << pb << " " << pdiv << endl;
	return 0;

}
catch (char* msg) {
	cerr << msg << endl;
	return 1;
}
