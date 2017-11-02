// arithmetic_multiply.cpp: functional tests for multiplication
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include <vector>

#include "../../bitset/bitset_helpers.hpp"
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
	cout << "reference " << pref << " result " << pmul << endl << endl;
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
	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	float fa, fb;
	fa = 2.0f; fb = -1.0f;
	cout << fa << " * " << fb << " = " << fa*fb << endl;
	GenerateTestCase<3,0>(fa, fb);

	double da, db;
	posit<3, 0> pa, pb, pmul, pref;
	pa.set_raw_bits(0);
	da = pa.to_double();
	pb.set_raw_bits(4);
	db = pb.to_double();
	pmul = pa * pb;
	pref = da * db;
	cout << pa << " * " << pb << " == " << pmul << " ref " << pref << " result " << da*db << endl;


	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<3, 0>("multiplication", bReportIndividualTestCases), "posit<3,0>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<4, 0>("multiplication", bReportIndividualTestCases), "posit<4,0>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<4, 1>("multiplication", bReportIndividualTestCases), "posit<4,1>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<5, 0>("multiplication", bReportIndividualTestCases), "posit<5,0>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<5, 1>("multiplication", bReportIndividualTestCases), "posit<5,1>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<5, 2>("multiplication", bReportIndividualTestCases), "posit<5,2>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<6, 0>("multiplication", bReportIndividualTestCases), "posit<6,0>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<6, 1>("multiplication", bReportIndividualTestCases), "posit<6,1>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<6, 2>("multiplication", bReportIndividualTestCases), "posit<6,2>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<6, 3>("multiplication", bReportIndividualTestCases), "posit<6,3>", "multiplication");

	return nrOfFailedTestCases;
}
catch (char* msg) {
	cerr << msg << endl;
	return 1;
}
