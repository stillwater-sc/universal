// incr_decr.cpp: functional tests for increment and decrement operators
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include "../../bitset/bitset_helpers.hpp"
#include "../../posit/posit_regime_lookup.hpp"
#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"
#include "../../posit/posit_manipulators.hpp"
#include "../tests/test_helpers.hpp"

using namespace std;

// validate the increment operator++
template<size_t nbits, size_t es>
int ValidateIncrement(std::string tag, bool bReportIndividualTestCases) 
{
	const size_t NrOfReals = (unsigned(1) << nbits);
	const size_t NrOfPositiveReals = (unsigned(1) << (nbits-1));
	const size_t NrOfNegativeReals = (unsigned(1) << (nbits-1));

	int nrOfFailedTestCases = 0;

	posit<nbits, es> p, ref;
	// from zero to inf via positive regime, back to zero via negative regime
	for (unsigned i = 0; i < NrOfReals-1; i++) {
		p.set_raw_bits(i);
		p++;
		ref.set_raw_bits(i+1);
		if (p != ref) {
			if (bReportIndividualTestCases) cout << tag << " FAIL [" << i << "] " << p << " != " << ref << endl;
			nrOfFailedTestCases++;
		}
	}

	return nrOfFailedTestCases;
}

// validate the decrement operator--
template<size_t nbits, size_t es>
int ValidateDecrement(std::string tag, bool bReportIndividualTestCases)
{
	const size_t NrOfReals = (unsigned(1) << nbits);
	const size_t NrOfPositiveReals = (unsigned(1) << (nbits - 1));
	const size_t NrOfNegativeReals = (unsigned(1) << (nbits - 1));

	int nrOfFailedTestCases = 0;

	posit<nbits, es> p, ref;
	// from zero to inf via negative regime, back to zero via positive regime
	for (int i = NrOfReals-1; i >= 0; i--) {
		p.set_raw_bits(i);
		p--;
		ref.set_raw_bits(i - 1);
		if (p != ref) {
			if (bReportIndividualTestCases) cout << tag << " FAIL [" << i << "] " << p << " != " << ref << endl;
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

	{
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
	}

	{
		// DECREMENT tests
		cout << endl << "DECREMENT tests" << endl;
		nrOfFailedTestCases += ReportTestResult(ValidateDecrement<3, 0>("Decrement failed", bReportIndividualTestCases), "posit<3,0>", "operator--");

		nrOfFailedTestCases += ReportTestResult(ValidateDecrement<4, 0>("Decrement failed", bReportIndividualTestCases), "posit<4,0>", "operator--");
		nrOfFailedTestCases += ReportTestResult(ValidateDecrement<4, 1>("Decrement failed", bReportIndividualTestCases), "posit<4,1>", "operator--");

		nrOfFailedTestCases += ReportTestResult(ValidateDecrement<5, 0>("Decrement failed", bReportIndividualTestCases), "posit<5,0>", "operator--");
		nrOfFailedTestCases += ReportTestResult(ValidateDecrement<5, 1>("Decrement failed", bReportIndividualTestCases), "posit<5,1>", "operator--");
		nrOfFailedTestCases += ReportTestResult(ValidateDecrement<5, 2>("Decrement failed", bReportIndividualTestCases), "posit<5,2>", "operator--");

		nrOfFailedTestCases += ReportTestResult(ValidateDecrement<6, 0>("Decrement failed", bReportIndividualTestCases), "posit<6,0>", "operator--");
		nrOfFailedTestCases += ReportTestResult(ValidateDecrement<6, 1>("Decrement failed", bReportIndividualTestCases), "posit<6,1>", "operator--");
		nrOfFailedTestCases += ReportTestResult(ValidateDecrement<6, 2>("Decrement failed", bReportIndividualTestCases), "posit<6,2>", "operator--");
		nrOfFailedTestCases += ReportTestResult(ValidateDecrement<6, 3>("Decrement failed", bReportIndividualTestCases), "posit<6,3>", "operator--");

		nrOfFailedTestCases += ReportTestResult(ValidateDecrement<7, 0>("Decrement failed", bReportIndividualTestCases), "posit<7,0>", "operator--");
		nrOfFailedTestCases += ReportTestResult(ValidateDecrement<7, 1>("Decrement failed", bReportIndividualTestCases), "posit<7,1>", "operator--");
		nrOfFailedTestCases += ReportTestResult(ValidateDecrement<7, 2>("Decrement failed", bReportIndividualTestCases), "posit<7,2>", "operator--");
		nrOfFailedTestCases += ReportTestResult(ValidateDecrement<7, 3>("Decrement failed", bReportIndividualTestCases), "posit<7,3>", "operator--");

		nrOfFailedTestCases += ReportTestResult(ValidateDecrement<8, 0>("Decrement failed", bReportIndividualTestCases), "posit<8,0>", "operator--");
		nrOfFailedTestCases += ReportTestResult(ValidateDecrement<8, 1>("Decrement failed", bReportIndividualTestCases), "posit<8,1>", "operator--");
		nrOfFailedTestCases += ReportTestResult(ValidateDecrement<8, 2>("Decrement failed", bReportIndividualTestCases), "posit<8,2>", "operator--");
		nrOfFailedTestCases += ReportTestResult(ValidateDecrement<8, 3>("Decrement failed", bReportIndividualTestCases), "posit<8,3>", "operator--");

	}
	return nrOfFailedTestCases;
}
catch (char* msg) {
	cerr << msg << endl;
	return 1;
}
