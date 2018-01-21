// arithmetic_divide.cpp: functional tests for division
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

// when you define POSIT_VERBOSE_OUTPUT executing an DIV the code will print intermediate results
//#define POSIT_VERBOSE_OUTPUT
#define POSIT_TRACE_DIV

#include "../../bitset/bitset_helpers.hpp"
#include "../../posit/posit.hpp"
#include "../../posit/posit_manipulators.hpp"
#include "../tests/test_helpers.hpp"
#include "../tests/posit_test_helpers.hpp"

using namespace std;
using namespace sw::unum;

// generate specific test case that you can trace with the trace conditions in posit.h
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCase(Ty a, Ty b) {
	Ty reference;
	posit<nbits, es> pa, pb, pdiv;
	pa = a;
	pb = b;
	reference = a / b;
	pdiv = pa / pb;
	cout << "reference " << reference << " result " << pdiv << endl << endl;
}

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "Division failed: ";

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug
	const size_t nbits = 20;
	const size_t es = 0;
	float a = 91.34375f;
	float b = 0.14453125f;
	posit<nbits, es> pa(a), pb(b);
	std::cout << pa.get() << " / " << pb.get() << std::endl;
	GenerateTestCase<nbits, es, float>(91.34375f, 0.14453125);
	

	//nrOfFailedTestCases += ReportTestResult(ValidateDivision<3, 0>("Manual Testing", true), "posit<3,0>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<8, 0>("Manual Testing", true), "posit<8,0>", "division");

#else

	cout << "Posit division validation" << endl;

	nrOfFailedTestCases += ReportTestResult(ValidateDivision<3, 0>(tag, bReportIndividualTestCases), "posit<3,0>", "division");

	nrOfFailedTestCases += ReportTestResult(ValidateDivision<4, 0>(tag, bReportIndividualTestCases), "posit<4,0>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<4, 1>(tag, bReportIndividualTestCases), "posit<4,1>", "division");

	nrOfFailedTestCases += ReportTestResult(ValidateDivision<5, 0>(tag, bReportIndividualTestCases), "posit<5,0>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<5, 1>(tag, bReportIndividualTestCases), "posit<5,1>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<5, 2>(tag, bReportIndividualTestCases), "posit<5,2>", "division");

	nrOfFailedTestCases += ReportTestResult(ValidateDivision<6, 0>(tag, bReportIndividualTestCases), "posit<6,0>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<6, 1>(tag, bReportIndividualTestCases), "posit<6,1>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<6, 2>(tag, bReportIndividualTestCases), "posit<6,2>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<6, 3>(tag, bReportIndividualTestCases), "posit<6,3>", "division");

	nrOfFailedTestCases += ReportTestResult(ValidateDivision<7, 0>(tag, bReportIndividualTestCases), "posit<7,0>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<7, 1>(tag, bReportIndividualTestCases), "posit<7,1>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<7, 2>(tag, bReportIndividualTestCases), "posit<7,2>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<7, 3>(tag, bReportIndividualTestCases), "posit<7,3>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<7, 4>(tag, bReportIndividualTestCases), "posit<7,4>", "division");

	nrOfFailedTestCases += ReportTestResult(ValidateDivision<8, 0>(tag, bReportIndividualTestCases), "posit<8,0>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<8, 1>(tag, bReportIndividualTestCases), "posit<8,1>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<8, 2>(tag, bReportIndividualTestCases), "posit<8,2>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<8, 3>(tag, bReportIndividualTestCases), "posit<8,3>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<8, 4>(tag, bReportIndividualTestCases), "posit<8,4>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<8, 5>(tag, bReportIndividualTestCases), "posit<8,5>", "division");

#if STRESS_TESTING
        nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<16, 1>(tag, bReportIndividualTestCases, OPCODE_DIV, 1000), "posit<16,1>", "division");
        nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<24, 1>(tag, bReportIndividualTestCases, OPCODE_DIV, 1000), "posit<24,1>", "division");
        nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<32, 1>(tag, bReportIndividualTestCases, OPCODE_DIV, 1000), "posit<32,1>", "division");
        nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<32, 2>(tag, bReportIndividualTestCases, OPCODE_DIV, 1000), "posit<32,2>", "division");
        nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<48, 2>(tag, bReportIndividualTestCases, OPCODE_DIV, 1000), "posit<48,2>", "division");

        // nbits=64 requires long double compiler support
        nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<64, 2>(tag, bReportIndividualTestCases, OPCODE_DIV, 1000), "posit<64,2>", "division");
        nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<64, 3>(tag, bReportIndividualTestCases, OPCODE_DIV, 1000), "posit<64,3>", "division");
        // posit<64,4> is hitting subnormal numbers
        nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<64, 4>(tag, bReportIndividualTestCases, OPCODE_MUL, 1000), "posit<64,4>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(ValidateDivision<10, 0>(tag, bReportIndividualTestCases), "posit<10,0>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<12, 1>(tag, bReportIndividualTestCases), "posit<12,1>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<14, 1>(tag, bReportIndividualTestCases), "posit<14,1>", "division");
	nrOfFailedTestCases += ReportTestResult(ValidateDivision<16, 1>(tag, bReportIndividualTestCases), "posit<16,1>", "division");

#endif // STRESS_TESTING

#endif // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}

