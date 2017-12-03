//  quire_accumulations.cpp : computational path experiments with quires
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include "../../posit/posit.hpp"
#include "../../posit/quire.hpp"

using namespace std;
using namespace sw::unum;


#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main()
try {
	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "Quire Accumulation failed";

#if MANUAL_TESTING
	const size_t nbits = 8;
	const size_t es = 1;
	const size_t capacity = 2; // for testing the accumulation capacity of the quire can be small
	const size_t fbits = 5;

	posit<nbits, es> p1, p2, minpos, maxpos;
	quire<nbits, es, 2> q1, q2;
	p1 = 1; ++p1;
	p2 = 1; --p2;
	std::cout << "p1 : " << p1 << " p2 : " << p2 << endl;
	q1 += p1 + p2;
	cout << "q  : " << q1 << endl;
	minpos = 0; ++minpos;
	maxpos = INFINITY; --maxpos;
	cout << "minpos : " << minpos << " maxpos : " << maxpos << endl;
	cout << "minpos * p1 = " << minpos * p1 << " minpos * p2 = " << minpos * p2 << endl;
	q2 += maxpos * (minpos * p1 + minpos * p2);
	cout << "q  : " << q2 << endl;
#else

	cout << "Quire experiments" << endl;
	

#ifdef STRESS_TESTING


#endif // STRESS_TESTING


#endif // MANUAL_TESTING
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char* msg) {
	cerr << "Exception thrown: " << msg << endl;
	return EXIT_FAILURE;
}
