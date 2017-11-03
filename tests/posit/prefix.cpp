// prefix.cpp functional tests for prefix operators
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"
#include <vector>
#include <algorithm>

#include "../../bitset/bitset_helpers.hpp"
#include "../../posit/posit_regime_lookup.hpp"
#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"
#include "../../posit/posit_manipulators.hpp"
#include "../tests/test_helpers.hpp"
#include "../tests/posit_test_helpers.hpp"

using namespace std;

int main(int argc, char** argv)
try
{
	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	// integer example
	// Oh no!
// 	int i = 0;
// 	cout << i << " " << --i << " " << i << " " << i++ << " " << i << endl;
// 	cout << i << " " << i++ << " " << i << endl;
// 	i = 0;
// 	cout << --i << " " << --i << " " << --i << endl;
// 	i = 0;
// 	cout << --(--(--i)) << endl;
// 	i = 0;
// 	cout << ------i << " " << endl;

	// equivalent posit example
	const size_t nbits = 4;
	const size_t es = 0;
	posit<nbits, es> result, p = 0.0f;
	cout << p << " " << --p << " " << p << " " << p++ << " " << p << endl;
	cout << p << " " << p++ << " " << p << endl;
	p = 0.0f;
	cout << --p << " " << --p << " " << --p << endl;
	p = 0.0f;
	cout << --(--(--p)) << endl;

	p = 0.0f;
	result = --p++;
	cout << "result " << result << endl;
	if (!p.isZero()) {
		cout << "FAIL 1 " << p << endl; nrOfFailedTestCases++;
	}
	p = 0.0f; --(--(--(p++)++)++);
	if (!p.isZero()) {
		cout << "FAIL 2 " << p << endl; nrOfFailedTestCases++;
	}
	p = 0.0f; ++(++(++(p--)--)--);
	if (!p.isZero()) {
		cout << "FAIL 3 " << p << endl; nrOfFailedTestCases++;
	}
	p = 0.0f; ----------p++++++++++;
	if (!p.isZero()) {
		cout << "FAIL 4 " << p << endl; nrOfFailedTestCases++;
	}
	p = 0.0f; p++++++++++;
	if (p != posit<nbits, es>(1.0f)) {
		cout << "FAIL 5 " << p << endl; nrOfFailedTestCases++;
	}

	nrOfFailedTestCases += ReportTestResult(ValidatePrefix<3, 0>("Increment failed", bReportIndividualTestCases), "posit<3,0>", "prefix++(posit)");

	nrOfFailedTestCases += ReportTestResult(ValidatePrefix<4, 0>("Increment failed", bReportIndividualTestCases), "posit<4,0>", "prefix++(posit)");
	nrOfFailedTestCases += ReportTestResult(ValidatePrefix<4, 1>("Increment failed", bReportIndividualTestCases), "posit<4,1>", "prefix++(posit)");

	return nrOfFailedTestCases;
}
catch (char* msg) {
	cerr << msg << endl;
	return 1;
}
