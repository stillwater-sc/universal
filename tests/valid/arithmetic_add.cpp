// arithmetic_add.cpp: functional tests for addition
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include "../../posit/exceptions.hpp"
#include "../../bitblock/bitblock.hpp"
#include "../../posit/value.hpp"
#include "../../posit/posit.hpp"
#include "../../valid/valid.hpp"
#include "../../valid/valid_manipulators.hpp"
#include "../tests/test_helpers.hpp"
#include "../tests/valid_test_helpers.hpp"


// generate specific test case that you can trace with the trace conditions in posit.h
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCase(Ty a, Ty b) {
	Ty reference;
	sw::unum::valid<nbits, es> pa, pb, psum;
	pa = a;
	pb = b;
	reference = a + b;
	//psum = pa + pb;
	std::cout << "reference " << reference << " result " << psum << '\n' << std::endl;
}

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	//bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;
	std::string tag = "Addition failed: ";

	cout << "Valid addition validation" << endl;

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug
	constexpr size_t nbits = 16;
	constexpr size_t es = 1;
	valid<nbits, es> v1, v2;

	v1.clear();
	cout << v1 << endl;

	v2.setToInclusive();
	cout << v2 << endl;

	v1 = 1;
	cout << v1 << endl;

	posit<nbits, es> lb(1.25f), ub(1.375f);
	v2.setLowerBound(lb, false);
	v2.setUpperBound(ub, true);
	cout << v2 << endl;


#else

	nrOfFailedTestCases += ReportTestResult(ValidateAddition<3, 0>(tag, bReportIndividualTestCases), "valid<3,0>", "addition");

	nrOfFailedTestCases += ReportTestResult(ValidateAddition<4, 0>(tag, bReportIndividualTestCases), "valid<4,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<4, 1>(tag, bReportIndividualTestCases), "valid<4,1>", "addition");

	nrOfFailedTestCases += ReportTestResult(ValidateAddition<5, 0>(tag, bReportIndividualTestCases), "valid<5,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<5, 1>(tag, bReportIndividualTestCases), "valid<5,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<5, 2>(tag, bReportIndividualTestCases), "valid<5,2>", "addition");

	nrOfFailedTestCases += ReportTestResult(ValidateAddition<6, 0>(tag, bReportIndividualTestCases), "valid<6,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<6, 1>(tag, bReportIndividualTestCases), "valid<6,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<6, 2>(tag, bReportIndividualTestCases), "valid<6,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<6, 3>(tag, bReportIndividualTestCases), "valid<6,3>", "addition");

	nrOfFailedTestCases += ReportTestResult(ValidateAddition<7, 0>(tag, bReportIndividualTestCases), "valid<7,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<7, 1>(tag, bReportIndividualTestCases), "valid<7,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<7, 2>(tag, bReportIndividualTestCases), "valid<7,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<7, 3>(tag, bReportIndividualTestCases), "valid<7,3>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<7, 4>(tag, bReportIndividualTestCases), "valid<7,4>", "addition");

	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 0>(tag, bReportIndividualTestCases), "valid<8,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 1>(tag, bReportIndividualTestCases), "valid<8,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 2>(tag, bReportIndividualTestCases), "valid<8,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 3>(tag, bReportIndividualTestCases), "valid<8,3>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 4>(tag, bReportIndividualTestCases), "valid<8,4>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 5>(tag, bReportIndividualTestCases), "valid<8,5>", "addition");

	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<16, 1>(tag, bReportIndividualTestCases, OPCODE_ADD, 1000), "valid<16,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<24, 1>(tag, bReportIndividualTestCases, OPCODE_ADD, 1000), "valid<24,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<32, 1>(tag, bReportIndividualTestCases, OPCODE_ADD, 1000), "valid<32,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<32, 2>(tag, bReportIndividualTestCases, OPCODE_ADD, 1000), "valid<32,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<48, 2>(tag, bReportIndividualTestCases, OPCODE_ADD, 1000), "valid<48,2>", "addition");

#if STRESS_TESTING
	// nbits=64 requires long double compiler support
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<64, 2>(tag, bReportIndividualTestCases, OPCODE_ADD, 1000), "valid<64,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<64, 3>(tag, bReportIndividualTestCases, OPCODE_ADD, 1000), "valid<64,3>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<64, 4>(tag, bReportIndividualTestCases, OPCODE_ADD, 1000), "valid<64,4>", "addition");


	nrOfFailedTestCases += ReportTestResult(ValidateAddition<10, 1>(tag, bReportIndividualTestCases), "valid<10,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<12, 1>(tag, bReportIndividualTestCases), "valid<12,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<14, 1>(tag, bReportIndividualTestCases), "valid<14,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<16, 1>(tag, bReportIndividualTestCases), "valid<16,1>", "addition");
#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
