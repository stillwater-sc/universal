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

#define MANUAL_TESTING 0
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

	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<8, 1, 2>(bReportIndividualTestCases, 16, minpos<8, 1>());

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
