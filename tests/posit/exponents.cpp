//  exponents.cpp : tests on posit exponents
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"
#include "../../posit/posit_manipulators.hpp"

using namespace std;

template<size_t es>
int ValidateExponentOperations() {
	int nrOfFailedTestCases = 0;

	return nrOfFailedTestCases;
}

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;


#if MANUAL_TESTING

	// generate individual testcases to hand trace/debug
	cout << "Manual Exponent tests" << endl;
	constexpr size_t nbits = 6;
	constexpr size_t es = 2;
	regime<nbits, es> r;
	exponent<nbits, es> e;
	for (int scale = -16; scale < 17; scale++) {
		int regime_size = r.assign_regime_pattern(scale >> es);
		int exp_size = e.assign_exponent_bits(scale, regime_size);
		if (scale < 0) {
			cout << "in value = " << setw(12) << 1.0/(unsigned(1) << -scale) << " scale = " << setw(3) << scale << " r(" << r << ")  e(" << e << ")     projected value " << r.value() * e.value() << endl;
		}
		else {
			cout << "in value = " << setw(12) << (unsigned(1) << scale)      << " scale = " << setw(3) << scale << " r(" << r << ")  e(" << e << ")     projected value " << r.value() * e.value() << endl;
		}
	}

#else

	cout << "Exponent tests" << endl;

	exponent<nbits, es> e1, e2;

#endif

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
