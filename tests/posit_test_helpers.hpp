//  posit_test_helpers.cpp : functions to aid in testing and test reporting on posit types.
// Needs to be included after posit type is declared.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

template<size_t nbits, size_t es>
void ReportUnaryArithmeticError(std::string test_case, std::string op, const posit<nbits, es>& rhs, const posit<nbits, es>& pref, const posit<nbits, es>& presult) {
	std::cerr << test_case
		<< " " << op << " "	
		<< std::setw(10) << rhs
		<< " != "
		<< std::setw(10) << pref << " instead it yielded "
		<< std::setw(10) << presult
		<< " " << components_to_string(presult) << std::endl;
}

template<size_t nbits, size_t es>
void ReportUnaryArithmeticSuccess(std::string test_case, std::string op, const posit<nbits, es>& rhs, const posit<nbits, es>& pref, const posit<nbits, es>& presult) {
	std::cerr << test_case
		<< " " << op << " "
		<< std::setw(10) << rhs
		<< " == "
		<< std::setw(10) << pref << " reference value is "
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

/////////////////////////////// VALIDATION TEST SUITES ////////////////////////////////


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
int ValidateIncrement(std::string tag, bool bReportIndividualTestCases)
{
	const size_t NrOfReals = (unsigned(1) << nbits);
	std::vector< posit<nbits, es> > set;
	GenerateOrderedPositSet(set);  // this has -inf at first position

	int nrOfFailedTestCases = 0;

	posit<nbits, es> p, ref;
	// from -maxpos to maxpos through zero
	for (typename std::vector < posit<nbits, es> >::iterator it = set.begin() + 1; it != set.end() - 1; it++) {
		p = *it;
		p++;
		ref = *(it + 1);
		if (p != ref) {
			if (bReportIndividualTestCases) std::cout << tag << " FAIL " << p << " != " << ref << std::endl;
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
	std::vector< posit<nbits, es> > set;
	GenerateOrderedPositSet(set); // this has -inf at the first position

	int nrOfFailedTestCases = 0;

	posit<nbits, es> p, ref;
	// from maxpos to -maxpos through zero
	for (typename std::vector < posit<nbits, es> >::iterator it = set.end() - 1; it != set.begin() + 1; it--) {
		p = *it;
		p--;
		ref = *(it - 1);
		if (p != ref) {
			if (bReportIndividualTestCases) std::cout << tag << " FAIL " << p << " != " << ref << std::endl;
			nrOfFailedTestCases++;
		}
	}

	return nrOfFailedTestCases;
}


// validate the postfix operator++
template<size_t nbits, size_t es>
int ValidatePostfix(std::string tag, bool bReportIndividualTestCases)
{
	const size_t NrOfReals = (unsigned(1) << nbits);
	std::vector< posit<nbits, es> > set;
	GenerateOrderedPositSet(set);  // this has -inf at first position

	int nrOfFailedTestCases = 0;

	posit<nbits, es> p, ref;
	// from -maxpos to maxpos through zero
	for (typename std::vector < posit<nbits, es> >::iterator it = set.begin() + 1; it != set.end() - 1; it++) {
		p = *it;
		p++;
		ref = *(it + 1);
		if (p != ref) {
			if (bReportIndividualTestCases) std::cout << tag << " FAIL " << p << " != " << ref << std::endl;
			nrOfFailedTestCases++;
		}
	}

	return nrOfFailedTestCases;
}


// validate the prefix operator++
template<size_t nbits, size_t es>
int ValidatePrefix(std::string tag, bool bReportIndividualTestCases)
{
	const size_t NrOfReals = (unsigned(1) << nbits);
	std::vector< posit<nbits, es> > set;
	GenerateOrderedPositSet(set);  // this has -inf at first position

	int nrOfFailedTestCases = 0;

	posit<nbits, es> p, ref;
	// from -maxpos to maxpos through zero
	for (typename std::vector < posit<nbits, es> >::iterator it = set.begin() + 1; it != set.end() - 1; it++) {
		p = *it;
		++p;
		ref = *(it + 1);
		if (p != ref) {
			if (bReportIndividualTestCases) std::cout << tag << " FAIL " << p << " != " << ref << std::endl;
			nrOfFailedTestCases++;
		}
	}

	return nrOfFailedTestCases;
}


// enerate all negation cases for a posit configuration: executes within 10 sec till about nbits = 14
template<size_t nbits, size_t es>
int ValidateNegation(std::string tag, bool bReportIndividualTestCases) {
	const int NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, pneg, pref;

	double input_values[NR_TEST_CASES];
	for (int i = 0; i < NR_TEST_CASES; i++) {
		pref.set_raw_bits(i);
		input_values[i] = pref.to_double();
	}
	double da;
	for (int i = 1; i < NR_TEST_CASES; i++) {
		pa.set_raw_bits(i);
		pneg = -pa;
		// generate reference
		da = pa.to_double();
		pref = -da;
		if (fabs(pneg.to_double() - pref.to_double()) > 0.000000001) {
			nrOfFailedTests++;
			if (bReportIndividualTestCases)	ReportUnaryArithmeticError("FAIL", "-", pa, pref, pneg);
		}
		else {
			if (bReportIndividualTestCases) ReportUnaryArithmeticSuccess("PASS", "-", pa, pref, pneg);
		}
	}
	return nrOfFailedTests;
}

// enumerate all addition cases for a posit configuration: is within 10sec till about nbits = 14
template<size_t nbits, size_t es>
int ValidateAddition(std::string error_tag, bool bReportIndividualTestCases) {
	const int NR_POSITS = (unsigned(1) << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, pb, psum, pref;

	double da, db;
	for (int i = 1; i < NR_POSITS; i++) {
		pa.set_raw_bits(i);
		da = pa.to_double();
		for (int j = 2; j < NR_POSITS; j++) {
			pb.set_raw_bits(j);
			db = pb.to_double();
			psum = pa + pb;
			pref = da + db;
			if (fabs(psum.to_double() - pref.to_double()) > 0.0001) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "+", pa, pb, pref, psum);
			}
			else {
				if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "+", pa, pb, pref, psum);
			}
		}
	}

	return nrOfFailedTests;
}

// enumerate all subtraction cases for a posit configuration: is within 10sec till about nbits = 14
template<size_t nbits, size_t es>
int ValidateSubtraction(std::string error_tag, bool bReportIndividualTestCases) {
	const int NR_POSITS = (1 << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, pb, pref, pdif;

	double da, db;
	for (int i = 1; i < NR_POSITS; i++) {
		pa.set_raw_bits(i);
		da = pa.to_double();
		for (int j = 2; j < NR_POSITS; j++) {
			pb.set_raw_bits(j);
			db = pb.to_double();
			pdif = pa - pb;
			pref = da - db;
			if (fabs(pdif.to_double() - pref.to_double()) > 0.0001) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "-", pa, pb, pref, pdif);
			}
			else {
				if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "-", pa, pb, pref, pdif);
			}
		}
	}

	return nrOfFailedTests;
}

// enumerate all multiplication cases for a posit configuration: is within 10sec till about nbits = 14
template<size_t nbits, size_t es>
int ValidateMultiplication(std::string tag, bool bReportIndividualTestCases) {
	int nrOfFailedTests = 0;
	const size_t NR_POSITS = (unsigned(1) << nbits);

	posit<nbits, es> pa, pb, pmul, pref;
	double da, db;
	for (int i = 0; i < NR_POSITS; i++) {
		pa.set_raw_bits(i);
		da = pa.to_double();
		for (int j = 0; j < NR_POSITS; j++) {
			pb.set_raw_bits(j);
			db = pb.to_double();
			pmul = pa * pb;
			pref = da * db;
			if (fabs(pmul.to_double() - pref.to_double()) > 0.000000001) {
				if (bReportIndividualTestCases) ReportBinaryArithmeticError("FAIL", "*", pa, pb, pref, pmul);
				nrOfFailedTests++;
			}
			else {
				if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "*", pa, pb, pref, pmul);
			}
		}
	}
	return nrOfFailedTests;
}

// enumerate all division cases for a posit configuration: is within 10sec till about nbits = 14
template<size_t nbits, size_t es>
int ValidateDivision(std::string tag, bool bReportIndividualTestCases) {
	int nrOfFailedTests = 0;
	const size_t NR_POSITS = (unsigned(1) << nbits);

	posit<nbits, es> pa, pb, pdiv, pref;
	double da, db;
	for (int i = 0; i < NR_POSITS; i++) {
		pa.set_raw_bits(i);
		da = pa.to_double();
		for (int j = 0; j < NR_POSITS; j++) {
			pb.set_raw_bits(j);
			db = pb.to_double();
			pdiv = pa / pb;
			pref = da / db;
			if (fabs(pdiv.to_double() - pref.to_double()) > 0.000000001) {
				if (bReportIndividualTestCases) ReportBinaryArithmeticError("FAIL", "/", pa, pb, pref, pdiv);
				nrOfFailedTests++;
			}
			else {
				if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "/", pa, pb, pref, pdiv);
			}
		}
	}
	return nrOfFailedTests;
}

//////////////////////////////////// RANDOMIZED TEST SUITE FOR BINARY OPERATORS ////////////////////////

// for testing posit configs that are > 14-15, we need a more efficient approach.
// One simple, brute force approach is to generate randoms.
// A more white box approach is to focus on the testcases 
// where something special happens in the posit arithmetic, such as rounding.

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
