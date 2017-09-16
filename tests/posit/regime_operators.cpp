// regime_operators.cpp: functional tests of the regime api
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"
#include <sstream>

#include "../../posit/posit.hpp"

using namespace std;

/*
Regime operators tests the regime manipulation of the posit class
*/

int main(int argc, char** argv)
{
	const size_t nbits = 64;
	const size_t es = 0;
    posit<nbits,es> p;
	try {
		for (int i = 0; i < 2*nbits-1; i++) {
			int k = i - nbits + 1;
			p.reset();
			cout << "k = " << setw(3) << k; 
			cout << " # Pattern bits = " << setw(3) << p.assign_regime_pattern(k);
			cout << " Regime = " << to_binary(p.get()) << endl;
		}
	}
	catch (char* e) {
		cerr << e << endl;
	}

	return 0;
}
