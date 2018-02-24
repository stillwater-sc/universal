//  quire_accumulations.cpp : computational path experiments with quires
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

// set to 1 if you want to generate hw test vectors
#define HARDWARE_QA_OUTPUT 0

// type definitions for the important types, posit<> and quire<>
#include "../../posit/posit.hpp"
#include "../../posit/quire.hpp"
// test support functions
#include "../tests/test_helpers.hpp"
#include "../tests/posit_test_helpers.hpp"
#include "../tests/quire_test_helpers.hpp"


using namespace std;
using namespace sw::unum;

template<size_t nbits, size_t es>
void PrintTestVector(std::ostream& ostr, const std::vector< posit<nbits,es> >& pv) {
	for (typename std::vector< posit<nbits,es> >::const_iterator it = pv.begin(); it != pv.end(); it++) {
		ostr << *it << std::endl;
	}
}

template<size_t nbits, size_t es, size_t capacity>
int GenerateQuireAccumulationTestCase(bool bReportIndividualTestCases, size_t nrOfElements, const posit<nbits,es>& seed) {
	int nrOfFailedTestCases = 0;
	std::stringstream ss;
	ss << "quire<" << nbits << "," << es << "," << capacity << ">";
	std::vector< posit<nbits, es> > t = GenerateVectorForZeroValueFDP(nrOfElements, seed);
	nrOfFailedTestCases += ReportTestResult(ValidateQuireAccumulation<nbits, es, capacity>(bReportIndividualTestCases, t), ss.str(), "accumulation");
	return nrOfFailedTestCases;
}

int ValidateQuireMagnitudeComparison() {
	quire<16, 1, 2> q = 0xAAAA;
	value<20> v;
	v = 0xAAAB;
	cout << "quire: " << q << endl;
	cout << "value: " << v.get_fixed_point() << " " << components(v) << endl;
	cout << (q < v ? "correct" : "incorrect") << endl;
	cout << (q > v ? "incorrect" : "correct") << endl;
	v = 0xAAAA;
	cout << "value: " << v.get_fixed_point() << " " << components(v) << endl;
	cout << (q == v ? "correct" : "incorrect") << endl;
	return 0;
}

template<size_t nbits, size_t es, size_t capacity = 2>
int ValidateSignMagnitudeTransitions() {
	int nrOfFailedTestCases = 0;

	// moving through the four quadrants of a sign/magnitue adder/subtractor
	posit<nbits, es> minpos, next_code_above_minpos, maxpos, next_code_below_maxpos;
	minpos = next_code_above_minpos = minpos_value<nbits, es>();
	next_code_above_minpos++;
	maxpos = next_code_below_maxpos = maxpos_value<nbits, es>();
	next_code_below_maxpos--;
	cout << "minpos         " << minpos.get() << " " << minpos << endl;
	cout << "minpos++       " << next_code_above_minpos.get() << " " << next_code_above_minpos << endl;
	cout << "maxpos--       " << next_code_below_maxpos.get() << " " << next_code_below_maxpos << endl;
	cout << "maxpos         " << maxpos.get() << " " << maxpos << endl;

	quire<nbits, es, capacity> q;
	// start in the positive, SE quadrant with minpos^2
	q += quire_mul(minpos, minpos);
	cout << q << endl;
	// move to the negative SW quadrant by adding negative value that is bigger
	q += quire_mul(next_code_above_minpos, -next_code_above_minpos);
	cout << q << endl;
	// remove minpos^2 from the quire by subtracting it
	q -= quire_mul(minpos, minpos);
	cout << q << endl;

	return nrOfFailedTestCases;
}

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main()
try {
	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	cout << "Quire experiments" << endl;

	std::string tag = "Quire Accumulation failed";

#if MANUAL_TESTING
	std::vector< posit<16, 1> > t;

//	t = GenerateVectorForZeroValueFDP(16, maxpos<16,1>());
//	PrintTestVector(cout, t);

	quire<8, 1, 2> q;
	posit<8, 1> minpos = minpos_value<8, 1>();
	q += quire_mul(minpos, minpos);
	value<3> v3 = q.to_value().round_to<3>();
	value<5> v5 = q.to_value().round_to<5>();
	value<7> v7 = q.to_value().round_to<7>();
	cout << components(v3) << endl;
	cout << components(v5) << endl;
	cout << components(v7) << endl;

	nrOfFailedTestCases += ValidateSignMagnitudeTransitions<8, 1>();


	//nrOfFailedTestCases += GenerateQuireAccumulationTestCase<8, 1, 2>(bReportIndividualTestCases, 16, minpos<8, 1>());

#else

	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<8, 0, 2>(bReportIndividualTestCases, 16, minpos<8, 0>());
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<8, 1, 2>(bReportIndividualTestCases, 16, minpos<8, 1>());
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<8, 2, 2>(bReportIndividualTestCases, 16, minpos<8, 2>());
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<8, 0, 5>(bReportIndividualTestCases, 16, maxpos<8, 0>());
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<8, 1, 5>(bReportIndividualTestCases, 16, maxpos<8, 1>());
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<8, 2, 5>(bReportIndividualTestCases, 16, maxpos<8, 2>());

	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<16, 0, 2>(bReportIndividualTestCases, 256, minpos<16, 0>());
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<16, 1, 2>(bReportIndividualTestCases, 256, minpos<16, 1>());
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<16, 2, 2>(bReportIndividualTestCases, 256, minpos<16, 2>());
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<16, 0, 5>(bReportIndividualTestCases, 16, maxpos<16, 0>());
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<16, 1, 5>(bReportIndividualTestCases, 16, maxpos<16, 1>());
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<16, 2, 5>(bReportIndividualTestCases, 16, maxpos<16, 2>());

	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<24, 0, 2>(bReportIndividualTestCases, 4096, minpos<24, 0>());
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<24, 1, 2>(bReportIndividualTestCases, 4096, minpos<24, 1>());
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<24, 2, 2>(bReportIndividualTestCases, 4096, minpos<24, 2>());

	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<32, 0, 2>(bReportIndividualTestCases, 65536, minpos<32, 0>());
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<32, 1, 2>(bReportIndividualTestCases, 65536, minpos<32, 1>());
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<32, 2, 2>(bReportIndividualTestCases, 65536, minpos<32, 2>());
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<32, 0, 5>(bReportIndividualTestCases, 16, maxpos<32, 0>());
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<32, 1, 5>(bReportIndividualTestCases, 16, maxpos<32, 1>());
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<32, 2, 5>(bReportIndividualTestCases, 16, maxpos<32, 2>());

#ifdef STRESS_TESTING


#endif // STRESS_TESTING


#endif // MANUAL_TESTING
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
catch (...) {
	cerr << "Caught unknown exception" << endl;
	return EXIT_FAILURE;
}
