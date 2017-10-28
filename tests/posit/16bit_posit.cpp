// 16bit_posit.cpp: Functionality tests for standard 16-bit posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include "../../posit/posit.hpp"
#include "../../posit/posit_operators.hpp"
#include "../../posit/posit_manipulators.hpp"
#include "../tests/test_helpers.hpp"
#include "../tests/posit_test_helpers.hpp"

using namespace std;

/*
Standard posits with nbits = 16 have 1 exponent bit.
*/

// operation opcodes
const int OPCODE_NOP = 0;
const int OPCODE_ADD = 1;
const int OPCODE_SUB = 2;
const int OPCODE_MUL = 3;
const int OPCODE_DIV = 4;

template<size_t nbits, size_t es>
void execute(int opcode, double da, double db, const posit<nbits, es>& pa, const posit<nbits, es>& pb, posit<nbits, es>& preference, posit<nbits, es>& presult) {
	switch (opcode) {
	default:
	case OPCODE_NOP:
		preference.reset();
		presult.reset();
		break;
	case OPCODE_ADD:
		presult    = pa + pb;
		preference = da + db;
		break;
	case OPCODE_SUB:
		presult    = pa - pb;
		preference = da - db;
		break;
	case OPCODE_MUL:
		presult    = pa * pb;
		preference = da * db;
		break;
	case OPCODE_DIV:
		presult    = pa / pb;
		preference = da / db;
		break;
	}
}

// generate a random set of operands to test the binary operators for a posit configuration
template<size_t nbits, size_t es>
int ValidateThroughRandoms(std::string tag, bool bReportIndividualTestCases, int opcode, uint32_t nrOfRandoms) {
	const size_t SIZE_STATE_SPACE = (uint32_t(1) << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, pb, presult, preference;

	std::string operation_string;
	switch (opcode) {
	default:
	case OPCODE_NOP:
		operation_string = "nop";
		break;
	case OPCODE_ADD:
		operation_string = "+";
		break;
	case OPCODE_SUB:
		operation_string = "-";
		break;
	case OPCODE_MUL:
		operation_string = "*";
		break;
	case OPCODE_DIV:
		operation_string = "/";
		break;
	}
	// generate the full state space set of valid posit values
	double operand_values[SIZE_STATE_SPACE];
	for (unsigned i = 0; i < SIZE_STATE_SPACE; i++) {
		presult.set_raw_bits(i);
		operand_values[i] = presult.to_double();
	}
	double da, db;
	unsigned ia, ib;  // random indices for picking operands to test
	for (int i = 1; i < nrOfRandoms; i++) {
		ia = std::rand() % SIZE_STATE_SPACE;
		da = operand_values[ia];
		pa = da;
		ib = std::rand() % SIZE_STATE_SPACE;
		db = operand_values[ib];
		pb = db;
		execute(opcode, da, db, pa, pb, preference, presult);
		if (fabs(presult.to_double() - preference.to_double()) > 0.000000001) {
			nrOfFailedTests++;
			if (bReportIndividualTestCases) ReportBinaryArithmeticError("FAIL", operation_string, pa, pb, preference, presult);

		}
		else {
			if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", operation_string, pa, pb, preference, presult);
		}
	}

	return nrOfFailedTests;
}

int main(int argc, char** argv)
try
{
	const size_t RND_TEST_CASES = 10000;
	int nrOfFailedTestCases = 0;
	bool bReportIndividualTestCases = false;

	cout << "Standard posit<16,1> configuration tests" << endl;
	const size_t nbits = 16;
	const size_t es = 1;
	posit<nbits, es> p;
	cout << spec_to_string(p) << endl << endl;

	cout << "Arithmetic test randoms" << endl;
	cout << "Addition      :                 " << RND_TEST_CASES << " randoms" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<16, 1>("  posit<16,1>", bReportIndividualTestCases, OPCODE_ADD, RND_TEST_CASES), " posit<16,1>", "addition      ");
	cout << "Subtraction   :                 " << RND_TEST_CASES << " randoms" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<16, 1>("  posit<16,1>", bReportIndividualTestCases, OPCODE_SUB, RND_TEST_CASES), " posit<16,1>", "subtraction   ");
	cout << "Multiplication:                 " << RND_TEST_CASES << " randoms" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<16, 1>("  posit<16,1>", bReportIndividualTestCases, OPCODE_MUL, RND_TEST_CASES), " posit<16,1>", "multiplication");
	cout << "Division      :                 " << RND_TEST_CASES << " randoms" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<16, 1>("  posit<16,1>", bReportIndividualTestCases, OPCODE_DIV, RND_TEST_CASES), " posit<16,1>", "division      ");

	return nrOfFailedTestCases;
}
catch (char* e) {
	cerr << e << endl;
	return -1;
}
