// arithmetic_multiply.cpp: functional tests for multiplication
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

// when you define POSIT_VERBOSE_OUTPUT executing an MUL the code will print intermediate results
//#define POSIT_VERBOSE_OUTPUT
#define POSIT_TRACE_MUL

// minimum set of include files to reflect source code dependencies
#include "../../bitset/bitset_helpers.hpp"
#include "../../posit/posit.hpp"
#include "../../posit/posit_manipulators.hpp"
#include "../tests/test_helpers.hpp"
#include "../tests/posit_test_helpers.hpp"

using namespace std;
using namespace sw::unum;

// generate specific test case that you can trace with the trace conditions in posit.h
// for most bugs they are traceable with _trace_conversion and _trace_mul
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCase(Ty a, Ty b) {
	Ty ref;
	posit<nbits, es> pa, pb, pref, pmul;
	pa = a;
	pb = b;
	ref = a * b;
	pref = ref;
	pmul = pa * pb;
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " * " << std::setw(nbits) << b << " = " << std::setw(nbits) << ref << std::endl;
	std::cout << pa.get() << " * " << pb.get() << " = " << pmul.get() << " (reference: " << pref.get() << ")   ";
	std::cout << (pref == pmul ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
}

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {	
	bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	cout << "Posit multiplication validation" << endl;

	std::string tag = "Multiplication failed: ";

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

	float fa, fb;
	fa = 0.0f; fb = INFINITY;
	std::cout << fa << " " << fb << std::endl;
	GenerateTestCase<4,0, float>(fa, fb);
	GenerateTestCase<16, 1, float>(float(minpos_value<16, 1>()), float(maxpos_value<16, 1>()));

	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<3, 0>("Manual Testing: ", bReportIndividualTestCases), "posit<3,0>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<4, 0>("Manual Testing: ", bReportIndividualTestCases), "posit<4,0>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<5, 0>("Manual Testing: ", bReportIndividualTestCases), "posit<5,0>", "multiplication");

#else

	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<3, 0>(tag, bReportIndividualTestCases), "posit<3,0>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<4, 0>(tag, bReportIndividualTestCases), "posit<4,0>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<4, 1>(tag, bReportIndividualTestCases), "posit<4,1>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<5, 0>(tag, bReportIndividualTestCases), "posit<5,0>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<5, 1>(tag, bReportIndividualTestCases), "posit<5,1>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<5, 2>(tag, bReportIndividualTestCases), "posit<5,2>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<6, 0>(tag, bReportIndividualTestCases), "posit<6,0>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<6, 1>(tag, bReportIndividualTestCases), "posit<6,1>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<6, 2>(tag, bReportIndividualTestCases), "posit<6,2>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<6, 3>(tag, bReportIndividualTestCases), "posit<6,3>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<7, 0>(tag, bReportIndividualTestCases), "posit<7,0>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<7, 1>(tag, bReportIndividualTestCases), "posit<7,1>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<7, 2>(tag, bReportIndividualTestCases), "posit<7,2>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<7, 3>(tag, bReportIndividualTestCases), "posit<7,3>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<7, 4>(tag, bReportIndividualTestCases), "posit<7,4>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<8, 0>(tag, bReportIndividualTestCases), "posit<8,0>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<8, 1>(tag, bReportIndividualTestCases), "posit<8,1>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<8, 2>(tag, bReportIndividualTestCases), "posit<8,2>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<8, 3>(tag, bReportIndividualTestCases), "posit<8,3>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<8, 4>(tag, bReportIndividualTestCases), "posit<8,4>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<8, 5>(tag, bReportIndividualTestCases), "posit<8,5>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<16, 1>(tag, bReportIndividualTestCases, OPCODE_MUL, 1000), "posit<16,1>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<24, 1>(tag, bReportIndividualTestCases, OPCODE_MUL, 1000), "posit<24,1>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<32, 1>(tag, bReportIndividualTestCases, OPCODE_MUL, 1000), "posit<32,1>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<32, 2>(tag, bReportIndividualTestCases, OPCODE_MUL, 1000), "posit<32,2>", "multiplication");


#if STRESS_TESTING
	// nbits=48 is also showing failures
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<48, 2>(tag, bReportIndividualTestCases, OPCODE_MUL, 1000), "posit<48,2>", "multiplication");

	// disabled until we can get long doubles to work: -> test is 64bit_posits.cpp
	// nbits=64 requires long double compiler support
	//nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<64, 2>(tag, bReportIndividualTestCases, OPCODE_MUL, 1000), "posit<64,2>", "multiplication");
	//nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<64, 3>(tag, bReportIndividualTestCases, OPCODE_MUL, 1000), "posit<64,3>", "multiplication");
	// posit<64,4> is hitting subnormal numbers
	//nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<64, 4>(tag, bReportIndividualTestCases, OPCODE_MUL, 1000), "posit<64,4>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<10, 0>(tag, bReportIndividualTestCases), "posit<10,0>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<10, 1>(tag, bReportIndividualTestCases), "posit<10,1>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<10, 2>(tag, bReportIndividualTestCases), "posit<10,1>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<10, 3>(tag, bReportIndividualTestCases), "posit<10,1>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<12, 1>(tag, bReportIndividualTestCases), "posit<12,1>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<14, 1>(tag, bReportIndividualTestCases), "posit<14,1>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<16, 1>(tag, bReportIndividualTestCases), "posit<16,1>", "multiplication");

#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

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

