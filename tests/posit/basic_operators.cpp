//  basic_operators.cpp : functional tests for conversion operators of posit numbers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"
#include <sstream>

#include "../../posit/posit.hpp"

using namespace std;
using namespace sw::unum;

template<size_t nbits, size_t es>
void checkSpecialCases(posit<nbits, es> p) {
	cout << "posit is " << (p.isZero() ? "zero " : "non-zero ") << (p.isPositive() ? "positive " : "negative ") << (p.isInfinite() ? "+-infinite" : "not infinite") << endl;
}

void BasicOperators() {
	const size_t nbits = 16;
	const size_t es = 1;
	posit<nbits, es> p1, p2, p3, p4, p5, p6;

	double minpos = minpos_value<nbits, es>();
	double maxpos = maxpos_value<nbits, es>();

	p1 = 0;
	//p1 = (int8_t)-1;  TODO: this triggers an assert because the number of bits of the fraction of the input is smaller than nf in the conversion
	p1 = 1;
	p2 = 2;

	p3 = p1 + p2;
	p4 = p2 - p1;
	p5 = p2 * p3;
	p6 = p5 / p3;

	cout << "p1: " << p1 << "\n";
	cout << "p2: " << p2 << "\n";
	cout << "p3: " << p3 << "\n";
	cout << "p4: " << p4 << "\n";
	cout << "p5: " << p5 << "\n";
	cout << "p6: " << p6 << "\n";

	cout << "p1++ " << p1++ << " " << p1 << "\n";
	cout << "++p1 " << ++p1 << "\n";
	cout << "p1-- " << p1-- << " " << p1 << "\n";
	cout << "--p1 " << --p1 << "\n";

	// negative regime
	p1 = -1; checkSpecialCases(p1);
}


int main()
try {
	int nrOfFailedTestCases = 0;

	BasicOperators();
    
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}



