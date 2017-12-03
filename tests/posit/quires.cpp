//  quires.cpp : test suite for quires
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include "../../posit/posit.hpp"
#include "../../posit/quire.hpp"

using namespace std;
using namespace sw::unum;


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
		cerr << "Caught the exception: " << msg << ". RHS was " << i << endl;
	}
}

template<size_t nbits, size_t es, size_t capacity, size_t fbits = 1>
void GenerateValueAssignments() {
	quire<nbits, es, capacity> q;
	int upper_range = q.upper_range();
	int lower_range = q.lower_range();
	cout << "Max scale = " << upper_range-1 << " Minimum scale = " << -lower_range << endl;
	int scale;
	value<fbits> v;
	double d = pow(2.0, upper_range);
	for (scale = upper_range; scale >= -lower_range - 1; scale--) {
		v = d;
		try {
			q = v;
			cout << setw(10) << v << q << endl;
		}
		catch (char* msg) {
			cerr << "Caught the exception: " << msg << ". RHS was " << v << " " << components(v) << endl;
		}
		d /= 2.0;
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
	const size_t fbits = 5;

	//GenerateUnsignedIntAssignments<nbits, es, capacity>();
	//GenerateSignedIntAssignments<nbits, es, capacity>();
	//GenerateUnsignedIntAssignments<8, 2, capacity>();

	GenerateValueAssignments<nbits, es, capacity, fbits>();

	cout << endl;
	cout << "Nothing prohibiting us from creating quires for float and double arithmetic" << endl;
	float f = 1.555555555555e-10f;
	quire<10, 2, 2> fquire(f);
	cout << "float:  " << setw(15) << f << " " << fquire << endl;

	double d = 1.555555555555e16;
	quire<10, 2, 2> dquire(d);
	cout << "double: " << setw(15) << d << " " << dquire << endl;

	cout << endl;
	cout << "Addition" << endl;
	quire<nbits, es, capacity> q;
	value<5> v(32.0);
	q += v;
	cout << q << endl;
	q += v;
	cout << q << endl;

	q = 0;
	v = 0.5;
	q += v;
	cout << q << endl;
	q += v;
	cout << q << endl;

	q = 0;
	v = 3.875 + 0.0625; cout << "v " << components(v) << endl;
	q += v;
	cout << q << endl;
	q += v;
	cout << q << endl;
	q += v;
	cout << q << endl;


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
