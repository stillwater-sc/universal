// arithmetic_subtract.cpp: functional tests for subtraction
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include "stdafx.h"

// when you define POSIT_VERBOSE_OUTPUT executing an SUB the code will print intermediate results
#define POSIT_VERBOSE_OUTPUT
//#define POSIT_TRACE_SUB

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
	posit<nbits, es> pa, pb, pref, pdif;
	pa = a;
	pb = b;
	reference = a - b;
	pref = reference;
	pdif = pa - pb;
	cout << pa << " - " << pb << " ref "  << reference << " posit ref " << pref << " actual " << pdif << endl << endl;
}

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

#include "../bitset_test_helpers.hpp"

int main(int argc, char** argv)
try {
	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "Subtraction failed: ";

#if MANUAL_TESTING

	//ValidateBitsetSubtraction<4>(true);

	// generate individual testcases to hand trace/debug
	GenerateTestCase<4, 0>(0.25, 0.75);
	GenerateTestCase<4, 0>(0.25, -0.75);
	//GenerateTestCase<8, 0>(1.0, 0.25);
	//GenerateTestCase<8, 0>(1.0, 0.125);
	//GenerateTestCase<8, 0>(1.0, 1.0);

	// manual exhaustive testing
	std::string positCfg = "posit<4,0>";
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<4, 0>("Manual Testing", true), positCfg, "subtraction");

#else

	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<3, 0>(tag, bReportIndividualTestCases), "posit<3,0>", "subtraction");

	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<4, 0>(tag, bReportIndividualTestCases), "posit<4,0>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<4, 1>(tag, bReportIndividualTestCases), "posit<4,1>", "subtraction");

	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<5, 0>(tag, bReportIndividualTestCases), "posit<5,0>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<5, 1>(tag, bReportIndividualTestCases), "posit<5,1>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<5, 2>(tag, bReportIndividualTestCases), "posit<5,2>", "subtraction");

	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<6, 0>(tag, bReportIndividualTestCases), "posit<6,0>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<6, 1>(tag, bReportIndividualTestCases), "posit<6,1>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<6, 2>(tag, bReportIndividualTestCases), "posit<6,2>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<6, 3>(tag, bReportIndividualTestCases), "posit<6,3>", "subtraction");

	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<7, 0>(tag, bReportIndividualTestCases), "posit<7,0>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<7, 1>(tag, bReportIndividualTestCases), "posit<7,1>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<7, 2>(tag, bReportIndividualTestCases), "posit<7,2>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<7, 3>(tag, bReportIndividualTestCases), "posit<7,3>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<7, 4>(tag, bReportIndividualTestCases), "posit<7,4>", "subtraction");

	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<8, 0>(tag, bReportIndividualTestCases), "posit<8,0>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<8, 1>(tag, bReportIndividualTestCases), "posit<8,1>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<8, 2>(tag, bReportIndividualTestCases), "posit<8,2>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<8, 3>(tag, bReportIndividualTestCases), "posit<8,3>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<8, 4>(tag, bReportIndividualTestCases), "posit<8,4>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<8, 5>(tag, bReportIndividualTestCases), "posit<8,5>", "subtraction");

        nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<16, 1>(tag, bReportIndividualTestCases, OPCODE_SUB, 1000), "posit<16,1>", "subtraction");
        nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<24, 1>(tag, bReportIndividualTestCases, OPCODE_SUB, 1000), "posit<24,1>", "subtraction");
        nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<32, 1>(tag, bReportIndividualTestCases, OPCODE_SUB, 1000), "posit<32,1>", "subtraction");
        nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<32, 2>(tag, bReportIndividualTestCases, OPCODE_SUB, 1000), "posit<32,2>", "subtraction");
        nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<48, 2>(tag, bReportIndividualTestCases, OPCODE_SUB, 1000), "posit<48,2>", "subtraction");

#if STRESS_TESTING
        // nbits=64 requires long double compiler support
        nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<64, 2>(tag, bReportIndividualTestCases, OPCODE_SUB, 1000), "posit<64,2>", "subtraction");
        nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<64, 3>(tag, bReportIndividualTestCases, OPCODE_SUB, 1000), "posit<64,3>", "subtraction");
        nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<64, 4>(tag, bReportIndividualTestCases, OPCODE_SUB, 1000), "posit<64,4>", "subtraction");

	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<10, 1>(tag, bReportIndividualTestCases), "posit<10,1>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<12, 1>(tag, bReportIndividualTestCases), "posit<12,1>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<14, 1>(tag, bReportIndividualTestCases), "posit<14,1>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(ValidateSubtraction<16, 1>(tag, bReportIndividualTestCases), "posit<16,1>", "subtraction");
#endif

#endif

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
