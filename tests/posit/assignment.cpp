// assignment.cpp : tests for native type literal assignments for posits
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include "../../posit/posit.hpp"
#include "../../posit/posit_manipulators.hpp"
#include "../tests/test_helpers.hpp"

using namespace std;
using namespace sw::unum;

#define FLOAT_TABLE_WIDTH 20

template<size_t nbits, size_t es, typename Ty>
void ReportAssignmentError(std::string test_case, std::string op, const posit<nbits, es>& pref, const posit<nbits, es>& presult, const Ty& value) {
	std::cerr << test_case
		<< " " << op << " "
		<< std::setw(FLOAT_TABLE_WIDTH) << value
		<< " != "
		<< std::setw(FLOAT_TABLE_WIDTH) << pref << " instead it yielded "
		<< std::setw(FLOAT_TABLE_WIDTH) << presult
		<< " " << presult.get() << " vs " << pref.get() << std::endl;
}

template<size_t nbits, size_t es, typename Ty>
void ReportAssignmentSuccess(std::string test_case, std::string op, const posit<nbits, es>& pref, const posit<nbits, es>& presult, const Ty& value) {
	std::cerr << test_case
		<< " " << op << " "
		<< std::setw(FLOAT_TABLE_WIDTH) << value
		<< " == "
		<< std::setw(FLOAT_TABLE_WIDTH) << presult << " reference value is "
		<< std::setw(FLOAT_TABLE_WIDTH) << pref
		<< "               posit fields " << pretty_print(presult) << std::endl;
}

template<size_t nbits, size_t es, typename Ty>
Ty GenerateValue(const posit<nbits,es>& p) {
	Ty value = 0;
	if (std::numeric_limits<Ty>::is_exact) {
		if (std::numeric_limits<Ty>::is_signed) {
			value = (long long)(p);
		}
		else {
			value = (unsigned long long)(p);
		}
	}
	else {
		value = (long double)(p);
	}
	return value;
}

template<size_t nbits, size_t es, typename Ty>
int ValidateAssignment(bool bReportIndividualTestCases) {
	const size_t NR_POSITS = (size_t(1) << nbits);
	int nrOfFailedTestCases = 0;

	// use only valid posit values
	// posit_raw -> to value in Ty -> assign to posit -> compare posits
	posit<nbits, es> p, assigned;
	for (size_t i = 0; i < NR_POSITS; i++) {
		p.set_raw_bits(i); // std::cout << p.get() << endl;
		if (p.isNaR() && std::numeric_limits<Ty>::is_exact) continue; // can't assign NaR for integer types
		Ty value = (Ty)(p);
		assigned = value;
		if (p != assigned) {
			nrOfFailedTestCases++;
			if (bReportIndividualTestCases) ReportAssignmentError("FAIL", "=", p, assigned, value);
		}
		else {
			//if (bReportIndividualTestCases) ReportAssignmentSuccess("PASS", "=", p, assigned, value);
		}
	}
	return nrOfFailedTestCases;
}

int main()
try {
	const size_t nbits = 8;
	const size_t es = 2;

	bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;
	std::string tag = "Assignment operator";

	posit<nbits, es> p;

	nrOfFailedTestCases = ReportTestResult(ValidateAssignment<3, 0, int>(bReportIndividualTestCases), tag, "assignment");
	nrOfFailedTestCases = ReportTestResult(ValidateAssignment<3, 0, float>(bReportIndividualTestCases), tag, "assignment");

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



