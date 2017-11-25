//  quires.cpp : test suite for quires
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include "../../posit/posit.hpp"
#include "../../posit/quire.hpp"

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

template<size_t nbits, size_t es, size_t capacity>
void GenerateUnsignedIntAssignments() {
	quire<nbits, es, capacity> q;
	unsigned upper_range = q.upper_range();
	cout << "Upper range = " << upper_range << endl;
	uint64_t i;
	q = 0; cout << q << endl;
	unsigned v = 1;
	for (i = 1; i < uint64_t(1) << (upper_range + capacity); i <<= 1) {
		q = i;
		cout << q << endl;
	}
	i <<= 1;
	try {
		q = i;
	}
	catch (char* msg) {
		cerr << "Caught the exception: " << msg << ". Value was " << i << endl;
	}
}

template<size_t nbits, size_t es, size_t capacity>
void GenerateSignedIntAssignments() {
	quire<nbits, es, capacity> q;
	unsigned upper_range = q.upper_range();
	cout << "Upper range = " << upper_range << endl;
	int64_t i, upper_limit = -(int64_t(1) << (upper_range + capacity));
	q = 0; cout << q << endl;
	unsigned v = 1;
	for (i = -1; i > upper_limit; i *= 2) {
		q = i;
		cout << q << endl;
	}
	i <<= 1;
	try {
		q = i;
	}
	catch (char* msg) {
		cerr << "Caught the exception: " << msg << ". Value was " << i << endl;
	}
}

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main()
try {
	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "Quire Accumulation failed";

#if MANUAL_TESTING
	const size_t nbits = 4;
	const size_t es = 1;
	const size_t capacity = 2; // for testing the accumulation capacity of the quire can be small

	GenerateUnsignedIntAssignments<nbits, es, capacity>();
	GenerateSignedIntAssignments<nbits, es, capacity>();
	//GenerateUnsignedIntAssignments<8, 2, capacity>();



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
