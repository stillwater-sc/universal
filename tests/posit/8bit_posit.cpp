// 8bit_posit.cpp: Functionality tests for standard 8-bit posits
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
Standard posits with nbits = 8 have no exponent bits.
*/

int main(int argc, char** argv)
{
	cout << "Standard posit<8,0> configuration tests" << endl;
	try {
        const size_t nbits = 8;
        const size_t es = 0;
        posit<nbits,es> p;

		cout << spec_to_string(p) << endl;

		ReportTestResult(ValidateAddition<8,0>("Posit<8,0> addition failed: "), "posit<8,0>", "addition") ;
	}
	catch (char* e) {
		cerr << e << endl;
	}

	return 0;
}
