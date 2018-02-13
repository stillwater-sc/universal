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
	for (std::vector< posit<nbits,es> >::const_iterator it = pv.begin(); it != pv.end(); it++) {
		ostr << *it << std::endl;
	}
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

	nrOfFailedTestCases += ReportTestResult(ValidateQuireAccumulation<8, 1, 2>(bReportIndividualTestCases, 16, minpos<8,1>()), "quire<8,1,2>", "accumulation");

#else

	nrOfFailedTestCases += ReportTestResult(ValidateQuireAccumulation<8, 0, 2>(bReportIndividualTestCases, 16, minpos<8, 0>()), "quire<8,0,2>", "accumulation");
	nrOfFailedTestCases += ReportTestResult(ValidateQuireAccumulation<8, 1, 2>(bReportIndividualTestCases, 16, minpos<8, 1>()), "quire<8,1,2>", "accumulation");
	nrOfFailedTestCases += ReportTestResult(ValidateQuireAccumulation<8, 2, 2>(bReportIndividualTestCases, 16, minpos<8, 2>()), "quire<8,2,2>", "accumulation");

	nrOfFailedTestCases += ReportTestResult(ValidateQuireAccumulation<16, 0, 2>(bReportIndividualTestCases, 256, minpos<16, 0>()), "quire<16,0,2>", "accumulation");
	nrOfFailedTestCases += ReportTestResult(ValidateQuireAccumulation<16, 1, 2>(bReportIndividualTestCases, 256, minpos<16, 1>()), "quire<16,1,2>", "accumulation");
	nrOfFailedTestCases += ReportTestResult(ValidateQuireAccumulation<16, 2, 2>(bReportIndividualTestCases, 256, minpos<16, 2>()), "quire<16,2,2>", "accumulation");

	nrOfFailedTestCases += ReportTestResult(ValidateQuireAccumulation<24, 0, 2>(bReportIndividualTestCases, 4096, minpos<24, 0>()), "quire<24,0,2>", "accumulation");
	nrOfFailedTestCases += ReportTestResult(ValidateQuireAccumulation<24, 1, 2>(bReportIndividualTestCases, 4096, minpos<24, 1>()), "quire<24,1,2>", "accumulation");
	nrOfFailedTestCases += ReportTestResult(ValidateQuireAccumulation<24, 2, 2>(bReportIndividualTestCases, 4096, minpos<24, 2>()), "quire<24,2,2>", "accumulation");

	nrOfFailedTestCases += ReportTestResult(ValidateQuireAccumulation<32, 0, 2>(bReportIndividualTestCases, 65536, minpos<32, 0>()), "quire<32,0,2>", "accumulation");
	nrOfFailedTestCases += ReportTestResult(ValidateQuireAccumulation<32, 1, 2>(bReportIndividualTestCases, 65536, minpos<32, 1>()), "quire<32,1,2>", "accumulation");
	nrOfFailedTestCases += ReportTestResult(ValidateQuireAccumulation<32, 2, 2>(bReportIndividualTestCases, 65536, minpos<32, 2>()), "quire<32,2,2>", "accumulation");

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
