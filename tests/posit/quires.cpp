//  quires.cpp : test suite for quires
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"
#include "../../posit/quire.hpp"
#include "../../posit/quire_operators.hpp"

using namespace std;

int TestQuireAccumulationResult(int nrOfFailedTests, string descriptor)
{
	if (nrOfFailedTests > 0) {
		cout << descriptor << " quire accumulation FAIL" << endl;
	}
	else {
		cout << descriptor << " quire accumulation PASS" << endl;
	}
	return nrOfFailedTests;
}

template<size_t nbits, size_t es, size_t capacity>
int ValidateQuireAccumulation() {
	const size_t NR_TEST_CASES = size_t(1) << nbits;

	int nrOfFailedTests = 0;

	return nrOfFailedTests;
}

template<size_t nbits, size_t es, size_t capacity>
void GenerateTestCase(int input, const quire<nbits, es, capacity>& reference, const quire<nbits, es, capacity>& qresult) {

	cout << endl;
}

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "Quire Accumulation failed";

#if MANUAL_TESTING

#else

	cout << "Quire validation" << endl;
	TestQuireAccumulationResult(ValidateQuireAccumulation<8,0,5>(), "quire<8,0,5>");

#ifdef STRESS_TESTING


#endif // STRESS_TESTING


#endif // MANUAL_TESTING
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
