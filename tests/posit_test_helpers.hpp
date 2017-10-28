//  posit_test_helpers.cpp : functions to aid in testing and test reporting on posit types.
// Needs to be included after posit type is declared.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

template<size_t nbits, size_t es>
void ReportUnaryArithmeticError(std::string test_case, std::string op, const posit<nbits, es>& lhs, const posit<nbits, es>& pref, const posit<nbits, es>& presult) {
	std::cerr << test_case
		<< " " << op << " "	
		<< std::setw(10) << lhs
		<< " != "
		<< std::setw(10) << pref << " instead it yielded "
		<< std::setw(10) << presult
		<< " " << components_to_string(presult) << std::endl;
}

template<size_t nbits, size_t es>
void ReportBinaryArithmeticError(std::string test_case, std::string op, const posit<nbits, es>& lhs, const posit<nbits, es>& rhs, const posit<nbits, es>& pref, const posit<nbits, es>& presult) {
	std::cerr << test_case
		<< std::setw(10) << lhs
		<< " " << op << " "
		<< std::setw(10) << rhs
		<< " != "
		<< std::setw(10) << pref <<    " instead it yielded "
		<< std::setw(10) << presult
		<< " " << components_to_string(presult) << std::endl;
}

template<size_t nbits, size_t es>
void ReportBinaryArithmeticSuccess(std::string test_case, std::string op, const posit<nbits, es>& lhs, const posit<nbits, es>& rhs, const posit<nbits, es>& pref, const posit<nbits, es>& presult) {
	std::cerr << test_case
		<< std::setw(10) << lhs
		<< " " << op << " "
		<< std::setw(10) << rhs
		<< " == "
		<< std::setw(10) << presult << " reference value is "
		<< std::setw(10) << pref
		<< " " << components_to_string(presult) << std::endl;
}

template<size_t nbits, size_t es>
void ReportDecodeError(std::string test_case, const posit<nbits, es>& actual, double golden_value) {
	std::cerr << test_case << " actual " << actual << " required " << golden_value << std::endl;
}

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
		presult = pa + pb;
		preference = da + db;
		break;
	case OPCODE_SUB:
		presult = pa - pb;
		preference = da - db;
		break;
	case OPCODE_MUL:
		presult = pa * pb;
		preference = da * db;
		break;
	case OPCODE_DIV:
		presult = pa / pb;
		preference = da / db;
		break;
	}
}

// generate a random set of operands to test the binary operators for a posit configuration
// Basic design is that we generate nrOfRandom posit values and store them in an operand array.
// We will then execute the binary operator nrOfRandom combinations.
template<size_t nbits, size_t es>
int ValidateThroughRandoms(std::string tag, bool bReportIndividualTestCases, int opcode, uint32_t nrOfRandoms) {
	const size_t SIZE_STATE_SPACE = nrOfRandoms;
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
	std::vector<double> operand_values(SIZE_STATE_SPACE);
	for (uint32_t i = 0; i < SIZE_STATE_SPACE; i++) {
		presult.set_raw_bits(std::rand());
		operand_values[i] = presult.to_double();
	}
	double da, db;
	unsigned ia, ib;  // random indices for picking operands to test
	for (unsigned i = 1; i < nrOfRandoms; i++) {
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