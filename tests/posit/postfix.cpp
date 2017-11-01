// postfix.cpp functional tests for postfix operators
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
	std::vector< posit<nbits, es> > s(NR_OF_REALS);
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
int ValidatePostfix(std::string tag, bool bReportIndividualTestCases)
{
	const size_t NrOfReals = (unsigned(1) << nbits);
	std::vector< posit<nbits, es> > set;
	GenerateOrderedPositSet(set);  // this has -inf at first position

	int nrOfFailedTestCases = 0;

	posit<nbits, es> p, ref;
	// from -maxpos to maxpos through zero
	for (typename std::vector < posit<nbits, es> >::iterator it = set.begin() + 1; it != set.end()-1; it++) {
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

	const size_t nbits = 4;
	const size_t es = 0;
	posit<nbits, es> p = 0.0f;
	--(--(--(p++)++)++);
	if (!p.isZero()) nrOfFailedTestCases++;
	++(++(++(p--)--)--);
	if (!p.isZero()) nrOfFailedTestCases++;
	--p++;
	if (!p.isZero()) nrOfFailedTestCases++;
	----------p++++++++++;
	if (!p.isZero()) nrOfFailedTestCases++;
	p++++++++++;
	if (p != posit<nbits, es>(1.0f)) nrOfFailedTestCases++;

	nrOfFailedTestCases += ReportTestResult(ValidatePostfix<3, 0>("Increment failed", bReportIndividualTestCases), "posit<3,0>", "posit++");

	nrOfFailedTestCases += ReportTestResult(ValidatePostfix<4, 0>("Increment failed", bReportIndividualTestCases), "posit<4,0>", "posit++");
	nrOfFailedTestCases += ReportTestResult(ValidatePostfix<4, 1>("Increment failed", bReportIndividualTestCases), "posit<4,1>", "posit++");

	return nrOfFailedTestCases;
}
catch (char* msg) {
	cerr << msg << endl;
	return 1;
}
