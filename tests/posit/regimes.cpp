//  regimes.cpp : tests on posit regimes
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include "../../posit/posit.hpp"
#include "../../posit/posit_manipulators.hpp"
#include "../tests/test_helpers.hpp"

using namespace std;
using namespace sw::unum;


/*
Regime range example for a posit<6,es>
	 regime      scale
	 00000          ~   associated with either 0 or NaR (Not a Real)
	 00001         -4
	 0001-         -3
	 001--         -2
	 01---         -1
	 10---          0
	 110--          1
	 1110-          2
	 11110          3
	 11111          4
*/
template<size_t nbits, size_t es>
int ValidateRegimeOperations(std::string tag, bool bReportIndividualTestCases) {
	const int NR_TEST_CASES = nbits;
	int nrOfFailedTestCases = 0;

	regime<nbits, es> r;
	for (int k = -NR_TEST_CASES; k < NR_TEST_CASES + 1; k++) {
		int reference = r.regime_size(k);
		int nrRegimeBits = r.assign_regime_pattern(k);
		if (nrRegimeBits != reference) {
			nrOfFailedTestCases++;
			if (bReportIndividualTestCases) cout << "FAIL: k = " << setw(3) << k << " regime is " << r << " bits " << nrRegimeBits << " reference " << reference << endl;
		}
		else {
			//if (bReportIndividualTestCases) cout << "PASS: k = " << setw(3) << k << " regime is " << r << " bits " << nrRegimeBits << " reference " << reference << endl;
		}
	}

	return nrOfFailedTestCases;
}


template<size_t nbits, size_t es>
int ValidateInwardProjection(std::string tag, bool bReportIndividualTestCases) {
	int nrOfFailedTests = 0;
	unsigned useed_scale = unsigned(1) << es;

	posit<nbits, es> p;
	// k represents the regime encoding 
	int size = int(nbits);
	for (int k = -size + 1; k <= size - 1; k++) {
		int scale = k*useed_scale;
		bool inward = p.check_inward_projection_range(k*useed_scale);
		bool reference = k < 0 ? k == -size + 1 : k == size - 1;
		if (inward != reference) {
			nrOfFailedTests++;
			cout << "FAIL : k = " << setw(3) << k << " scale = " << setw(3) << scale << " inward projection range " << inward << " reference " << reference << endl;
		}
		cout << "k = " << setw(3) << k << " scale = " << setw(3) << scale << " inward projection range " << inward << endl;

	}
	return nrOfFailedTests;
}

template<size_t nbits, size_t es>
int ValidateRegimeScales(std::string tag, bool bReportIndividualTestCases) {
	int nrOfFailedTests = 0;
	int useed_scale = int(1) << es;  // int because we are doing int math with it

	regime<nbits, es> r1;
	posit<nbits, es> p; // for check_inward_projection_range
	// scale represents the binary scale of a value to test
	int size = int(nbits);
	for (int k = (-size + 1); k <= (size - 1); k++) {
		int scale = k*useed_scale;
		r1.assign_regime_pattern(k);
		if (r1.scale() != scale) {
			if (p.check_inward_projection_range(scale)) {
				if (r1.scale() == (k - 1)*useed_scale || r1.scale() == (k + 1)*useed_scale) {
					continue;
				}
			}	
			nrOfFailedTests++;
			std::cout << "k = " << setw(3) << k
				<< " scale = " << setw(3) << scale
				<< " calc k " << setw(3) << r1.regime_k()
				<< " bits " << r1 << ":scale=" << r1.scale()
				<< " clamp " << p.check_inward_projection_range(scale) << std::endl;
		}

	}
	return nrOfFailedTests;
}

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "Regime conversion failed";

#if MANUAL_TESTING

	// generate individual testcases to hand trace/debug
#if 0
	regime<10, 2> r;
	for (int k = -7; k < 9; k++) {
		int regime_size = r.assign_regime_pattern(k);
		cout << "k = " << setw(3) << k << " regime is " << r << " regime size is " << regime_size << " bits" << endl;
	}
#endif

	nrOfFailedTestCases += ReportTestResult(ValidateRegimeScales<3, 0>(tag, bReportIndividualTestCases), "posit<3,0>", "regimes");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeScales<4, 1>(tag, bReportIndividualTestCases), "posit<4,1>", "regimes");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeScales<5, 2>(tag, bReportIndividualTestCases), "posit<5,2>", "regimes");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeScales<6, 3>(tag, bReportIndividualTestCases), "posit<6,3>", "regimes");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeScales<7, 4>(tag, bReportIndividualTestCases), "posit<7,4>", "regimes");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeScales<8, 5>(tag, bReportIndividualTestCases), "posit<8,5>", "regimes");
	return 0;
	nrOfFailedTestCases += ReportTestResult(ValidateInwardProjection<3, 0>(tag, bReportIndividualTestCases), "posit<3,0>", "regimes");
	nrOfFailedTestCases += ReportTestResult(ValidateInwardProjection<4, 1>(tag, bReportIndividualTestCases), "posit<4,1>", "regimes");
	nrOfFailedTestCases += ReportTestResult(ValidateInwardProjection<5, 2>(tag, bReportIndividualTestCases), "posit<5,2>", "regimes");
	nrOfFailedTestCases += ReportTestResult(ValidateInwardProjection<6, 3>(tag, bReportIndividualTestCases), "posit<6,3>", "regimes");

#else
	cout << "Regime tests" << endl;

	// TEST REGIME DECODE
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeOperations<3, 0>(tag, bReportIndividualTestCases), "regime<3,0>", "regime");

	nrOfFailedTestCases += ReportTestResult(ValidateRegimeOperations<4, 0>(tag, bReportIndividualTestCases), "regime<4,0>", "regime");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeOperations<4, 1>(tag, bReportIndividualTestCases), "regime<4,1>", "regime");

	nrOfFailedTestCases += ReportTestResult(ValidateRegimeOperations<5, 0>(tag, bReportIndividualTestCases), "regime<5,0>", "regime");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeOperations<5, 1>(tag, bReportIndividualTestCases), "regime<5,1>", "regime");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeOperations<5, 2>(tag, bReportIndividualTestCases), "regime<5,2>", "regime");

	nrOfFailedTestCases += ReportTestResult(ValidateRegimeOperations<6, 0>(tag, bReportIndividualTestCases), "regime<6,0>", "regime");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeOperations<6, 1>(tag, bReportIndividualTestCases), "regime<6,1>", "regime");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeOperations<6, 2>(tag, bReportIndividualTestCases), "regime<6,2>", "regime");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeOperations<6, 3>(tag, bReportIndividualTestCases), "regime<6,3>", "regime");

	nrOfFailedTestCases += ReportTestResult(ValidateRegimeOperations<7, 0>(tag, bReportIndividualTestCases), "regime<7,0>", "regime");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeOperations<7, 1>(tag, bReportIndividualTestCases), "regime<7,1>", "regime");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeOperations<7, 2>(tag, bReportIndividualTestCases), "regime<7,2>", "regime");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeOperations<7, 3>(tag, bReportIndividualTestCases), "regime<7,3>", "regime");

	nrOfFailedTestCases += ReportTestResult(ValidateRegimeOperations<8, 0>(tag, bReportIndividualTestCases), "regime<8,0>", "regime");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeOperations<8, 1>(tag, bReportIndividualTestCases), "regime<8,1>", "regime");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeOperations<8, 2>(tag, bReportIndividualTestCases), "regime<8,2>", "regime");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeOperations<8, 3>(tag, bReportIndividualTestCases), "regime<8,3>", "regime");

	// TEST REGIME CONVERSION
	// DIFFERENT WAY TO TEST REGIME CONSTRUCTION
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeScales<3, 0>(tag, bReportIndividualTestCases), "posit<3,0>", "scales");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeScales<4, 1>(tag, bReportIndividualTestCases), "posit<4,1>", "scales");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeScales<5, 2>(tag, bReportIndividualTestCases), "posit<5,2>", "scales");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeScales<6, 3>(tag, bReportIndividualTestCases), "posit<6,3>", "scales");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeScales<7, 4>(tag, bReportIndividualTestCases), "posit<7,4>", "scales");

	nrOfFailedTestCases += ReportTestResult(ValidateRegimeScales<8, 0>(tag, bReportIndividualTestCases), "posit<8,0>", "scales");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeScales<8, 1>(tag, bReportIndividualTestCases), "posit<8,1>", "scales");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeScales<8, 2>(tag, bReportIndividualTestCases), "posit<8,2>", "scales");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeScales<8, 3>(tag, bReportIndividualTestCases), "posit<8,3>", "scales");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeScales<8, 4>(tag, bReportIndividualTestCases), "posit<8,4>", "scales");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeScales<8, 5>(tag, bReportIndividualTestCases), "posit<8,5>", "scales");

	nrOfFailedTestCases += ReportTestResult(ValidateRegimeScales<16, 0>(tag, bReportIndividualTestCases), "posit<16,0>", "scales");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeScales<16, 1>(tag, bReportIndividualTestCases), "posit<16,1>", "scales");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeScales<16, 2>(tag, bReportIndividualTestCases), "posit<16,2>", "scales");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeScales<16, 3>(tag, bReportIndividualTestCases), "posit<16,3>", "scales");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeScales<16, 4>(tag, bReportIndividualTestCases), "posit<16,4>", "scales");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeScales<16, 5>(tag, bReportIndividualTestCases), "posit<16,5>", "scales");

	nrOfFailedTestCases += ReportTestResult(ValidateRegimeScales<32, 0>(tag, bReportIndividualTestCases), "posit<32,0>", "scales");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeScales<32, 1>(tag, bReportIndividualTestCases), "posit<32,1>", "scales");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeScales<32, 2>(tag, bReportIndividualTestCases), "posit<32,2>", "scales");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeScales<32, 3>(tag, bReportIndividualTestCases), "posit<32,3>", "scales");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeScales<32, 4>(tag, bReportIndividualTestCases), "posit<32,4>", "scales");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeScales<32, 5>(tag, bReportIndividualTestCases), "posit<32,5>", "scales");

	nrOfFailedTestCases += ReportTestResult(ValidateRegimeScales<64, 0>(tag, bReportIndividualTestCases), "posit<64,0>", "scales");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeScales<64, 1>(tag, bReportIndividualTestCases), "posit<64,1>", "scales");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeScales<64, 2>(tag, bReportIndividualTestCases), "posit<64,2>", "scales");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeScales<64, 3>(tag, bReportIndividualTestCases), "posit<64,3>", "scales");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeScales<64, 4>(tag, bReportIndividualTestCases), "posit<64,4>", "scales");
	nrOfFailedTestCases += ReportTestResult(ValidateRegimeScales<64, 5>(tag, bReportIndividualTestCases), "posit<64,5>", "scales");
#endif


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
