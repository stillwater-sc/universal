// 128bit_posit.cpp: Functionality tests for standard 128-bit posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"
#include "../../posit/posit_manipulators.hpp"

using namespace std;

/*
Standard posits with nbits = 128 have 4 exponent bits.
*/

int main(int argc, char** argv)
{
	cout << "Standard posit<128,4> configuration tests" << endl;
	try {
        const size_t nbits = 128;
        const size_t es = 4;
        posit<nbits,es> p;

		cout << spec_to_string(p) << endl;
	}
	catch (char* e) {
		cerr << e << endl;
	}

	return 0;
}
