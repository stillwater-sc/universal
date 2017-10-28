// increment.cpp: functional tests for increment operator
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

using namespace std;

// Generate ordered set from -maxpos to +maxpos for a particular posit config <nbits, es>
template<size_t nbits, size_t es>
void GenerateOrderedPositSet(std::vector<posit<nbits, es>>& set) {
	const size_t NR_OF_REALS = (unsigned(1) << nbits);
	std::vector<posit<nbits, es>> s(NR_OF_REALS);
	posit<nbits, es> p;
	// generate raw set, remove infinite as it is not 'reachable' through arithmetic operations
	for (int i = 0; i < NR_OF_REALS; i++) {
		p.set_raw_bits(i);
		s[i] = p;
	}
	// sort the set
	std::sort(s.begin(), s.end());
	set = s;
}

// validate the increment operator++
template<size_t nbits, size_t es>
int ValidateIncrement(std::string tag, bool bReportIndividualTestCases)
{
	const size_t NrOfReals = (unsigned(1) << nbits);
	std::vector<posit<nbits, es>> set;
	GenerateOrderedPositSet(set);  // this has -inf at first position

	int nrOfFailedTestCases = 0;

	posit<nbits, es> p, ref;
	// from -maxpos to maxpos through zero
	for (std::vector < posit<nbits, es> >::iterator it = set.begin() + 1; it != set.end()-1; it++) {
		p = *it;
		p++;
		ref = *(it + 1);
		if (p != ref) {
			if (bReportIndividualTestCases) cout << tag << " FAIL " << p << " != " << ref << endl;
			nrOfFailedTestCases++;
		}
	}

	return nrOfFailedTestCases;
}


int main(int argc, char** argv)
try
{
	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	// INCREMENT tests
	cout << endl << "INCREMENT tests" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidateIncrement<3, 0>("Increment failed", bReportIndividualTestCases), "posit<3,0>", "operator++");

	nrOfFailedTestCases += ReportTestResult(ValidateIncrement<4, 0>("Increment failed", bReportIndividualTestCases), "posit<4,0>", "operator++");
	nrOfFailedTestCases += ReportTestResult(ValidateIncrement<4, 1>("Increment failed", bReportIndividualTestCases), "posit<4,1>", "operator++");

	nrOfFailedTestCases += ReportTestResult(ValidateIncrement<5, 0>("Increment failed", bReportIndividualTestCases), "posit<5,0>", "operator++");
	nrOfFailedTestCases += ReportTestResult(ValidateIncrement<5, 1>("Increment failed", bReportIndividualTestCases), "posit<5,1>", "operator++");
	nrOfFailedTestCases += ReportTestResult(ValidateIncrement<5, 2>("Increment failed", bReportIndividualTestCases), "posit<5,2>", "operator++");

	nrOfFailedTestCases += ReportTestResult(ValidateIncrement<6, 0>("Increment failed", bReportIndividualTestCases), "posit<6,0>", "operator++");
	nrOfFailedTestCases += ReportTestResult(ValidateIncrement<6, 1>("Increment failed", bReportIndividualTestCases), "posit<6,1>", "operator++");
	nrOfFailedTestCases += ReportTestResult(ValidateIncrement<6, 2>("Increment failed", bReportIndividualTestCases), "posit<6,2>", "operator++");
	nrOfFailedTestCases += ReportTestResult(ValidateIncrement<6, 3>("Increment failed", bReportIndividualTestCases), "posit<6,3>", "operator++");

	nrOfFailedTestCases += ReportTestResult(ValidateIncrement<7, 0>("Increment failed", bReportIndividualTestCases), "posit<7,0>", "operator++");
	nrOfFailedTestCases += ReportTestResult(ValidateIncrement<7, 1>("Increment failed", bReportIndividualTestCases), "posit<7,1>", "operator++");
	nrOfFailedTestCases += ReportTestResult(ValidateIncrement<7, 2>("Increment failed", bReportIndividualTestCases), "posit<7,2>", "operator++");
	nrOfFailedTestCases += ReportTestResult(ValidateIncrement<7, 3>("Increment failed", bReportIndividualTestCases), "posit<7,3>", "operator++");

	nrOfFailedTestCases += ReportTestResult(ValidateIncrement<8, 0>("Increment failed", bReportIndividualTestCases), "posit<8,0>", "operator++");
	nrOfFailedTestCases += ReportTestResult(ValidateIncrement<8, 1>("Increment failed", bReportIndividualTestCases), "posit<8,1>", "operator++");
	nrOfFailedTestCases += ReportTestResult(ValidateIncrement<8, 2>("Increment failed", bReportIndividualTestCases), "posit<8,2>", "operator++");
	nrOfFailedTestCases += ReportTestResult(ValidateIncrement<8, 3>("Increment failed", bReportIndividualTestCases), "posit<8,3>", "operator++");

	if (argc == 2 && std::string(argv[1]) == std::string("-l")) {
		// AD/DA adapted data path configurations
		nrOfFailedTestCases += ReportTestResult(ValidateIncrement<10, 0>("Increment failed", bReportIndividualTestCases), "posit<10,0>", "operator++");
		nrOfFailedTestCases += ReportTestResult(ValidateIncrement<10, 1>("Increment failed", bReportIndividualTestCases), "posit<10,1>", "operator++");

		nrOfFailedTestCases += ReportTestResult(ValidateIncrement<12, 0>("Increment failed", bReportIndividualTestCases), "posit<12,0>", "operator++");
		nrOfFailedTestCases += ReportTestResult(ValidateIncrement<12, 1>("Increment failed", bReportIndividualTestCases), "posit<12,1>", "operator++");

		nrOfFailedTestCases += ReportTestResult(ValidateIncrement<14, 0>("Increment failed", bReportIndividualTestCases), "posit<14,0>", "operator++");
		nrOfFailedTestCases += ReportTestResult(ValidateIncrement<14, 1>("Increment failed", bReportIndividualTestCases), "posit<14,1>", "operator++");

		nrOfFailedTestCases += ReportTestResult(ValidateIncrement<15, 0>("Increment failed", bReportIndividualTestCases), "posit<15,0>", "operator++");
		nrOfFailedTestCases += ReportTestResult(ValidateIncrement<15, 1>("Increment failed", bReportIndividualTestCases), "posit<15,1>", "operator++");

		nrOfFailedTestCases += ReportTestResult(ValidateIncrement<16, 0>("Increment failed", bReportIndividualTestCases), "posit<16,0>", "operator++");
		nrOfFailedTestCases += ReportTestResult(ValidateIncrement<16, 1>("Increment failed", bReportIndividualTestCases), "posit<16,1>", "operator++");
		nrOfFailedTestCases += ReportTestResult(ValidateIncrement<16, 2>("Increment failed", bReportIndividualTestCases), "posit<16,2>", "operator++");

		nrOfFailedTestCases += ReportTestResult(ValidateIncrement<18, 0>("Increment failed", bReportIndividualTestCases), "posit<18,0>", "operator++");
		nrOfFailedTestCases += ReportTestResult(ValidateIncrement<18, 1>("Increment failed", bReportIndividualTestCases), "posit<18,1>", "operator++");
		nrOfFailedTestCases += ReportTestResult(ValidateIncrement<18, 2>("Increment failed", bReportIndividualTestCases), "posit<18,2>", "operator++");

		nrOfFailedTestCases += ReportTestResult(ValidateIncrement<20, 1>("Increment failed", bReportIndividualTestCases), "posit<20,1>", "operator++");
		
		// legit float replacement
		//nrOfFailedTestCases += ReportTestResult(ValidateIncrement<24, 1>("Increment failed", bReportIndividualTestCases), "posit<24,1>", "operator++");
		//nrOfFailedTestCases += ReportTestResult(ValidateIncrement<28, 2>("Increment failed", bReportIndividualTestCases), "posit<28,2>", "operator++");

		// legit double replacement
		//nrOfFailedTestCases += ReportTestResult(ValidateIncrement<32, 2>("Increment failed", bReportIndividualTestCases), "posit<32,2>", "operator++");

	}

	return nrOfFailedTestCases;
}
catch (char* msg) {
	cerr << msg << endl;
	return 1;
}
