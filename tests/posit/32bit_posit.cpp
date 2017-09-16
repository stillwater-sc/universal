// 32bit_posit.cpp: Functionality tests for standard 32-bit posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"

using namespace std;

/*
Standard posits with nbits = 32 have 2 exponent bits.
*/

int main(int argc, char** argv)
{
	cout << "Standard posit<32,2> configuration tests" << endl;
	try {
        const size_t nbits = 32;
        const size_t es = 2;
        posit<nbits,es> p;

		cout << spec_to_string(p) << endl;
	}
	catch (char* e) {
		cerr << e << endl;
	}

	return 0;
}
