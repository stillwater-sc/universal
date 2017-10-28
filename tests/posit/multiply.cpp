// multiply.cpp: functional tests for multiplication
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
#include "../tests/test_helpers.hpp"
#include "../tests/posit_test_helpers.hpp"

using namespace std;

// generate specific test case that you can trace with the trace conditions in posit.h
// for most bugs they are traceable with _trace_conversion and _trace_mul
template<size_t nbits, size_t es>
void GenerateTestCase(float fa, float fb) {
	posit<nbits, es> pa, pb, pref, pmul;
	pa = fa;
	pb = fb;
	pref = fa * fb;
	pmul = pa * pb;
	cout << "reference " << pref << " result " << pdif << endl << endl;
}

template<size_t nbits, size_t es>
void GenerateTestCase(double da, double db) {
	posit<nbits, es> pa, pb, pref, pmul;
	pa = da;
	pb = db;
	pref = da * db;
	pmul = pa * pb;
	cout << "reference " << pref << " result " << pmul << endl << endl;
}


int main(int argc, char** argv)
try 
{
	bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<3, 0>("multiplication", bReportIndividualTestCases), "posit<3,0>", "multiplication");

	return nrOfFailedTestCases;
}
catch (char* msg) {
	cerr << msg << endl;
	return 1;
}
