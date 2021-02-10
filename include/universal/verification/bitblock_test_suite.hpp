#pragma once
//  bitblock_test_suite.hpp : bitblock-based arithmetic test suite
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw::universal::internal {

template<size_t nbits, size_t rbits>
void ReportBinaryArithmeticError(const std::string& test_case, const std::string& op, const bitblock<nbits>& lhs, const bitblock<nbits>& rhs, const bitblock<rbits>& ref, const bitblock<rbits>& result) {
	constexpr size_t OPERAND_COLUMN_WIDTH = nbits;
	constexpr size_t RESULT_COLUMN_WIDTH = rbits;
	std::cerr << test_case << " "
		<< std::setw(OPERAND_COLUMN_WIDTH) << lhs
		<< " " << op << " "
		<< std::setw(OPERAND_COLUMN_WIDTH) << rhs
		<< " != "
		<< std::setw(RESULT_COLUMN_WIDTH) << ref << " instead it yielded "
		<< std::setw(RESULT_COLUMN_WIDTH) << result
		<< std::endl;
}

template<size_t nbits, size_t rbits>
void ReportBinaryArithmeticSuccess(const std::string& test_case, const std::string& op, const bitblock<nbits>& lhs, const bitblock<nbits>& rhs, const bitblock<rbits>& ref, const bitblock<rbits>& result) {
	constexpr size_t OPERAND_COLUMN_WIDTH = nbits;
	constexpr size_t RESULT_COLUMN_WIDTH = rbits;
	std::cerr << test_case << " "
		<< std::setw(OPERAND_COLUMN_WIDTH) << lhs
		<< " " << op << " "
		<< std::setw(OPERAND_COLUMN_WIDTH) << rhs
		<< " == "
		<< std::setw(RESULT_COLUMN_WIDTH) << result << " reference value is "
		<< std::setw(RESULT_COLUMN_WIDTH) << ref
		<< std::endl;
}

// verify bitset addition operator
template<size_t nbits>
int VerifyBitsetAddition(bool bReportIndividualTestCases = false) {
	const size_t NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTestCases = 0;
	bool carry;
	bitblock<nbits> a, b;
	bitblock<nbits + 1> bsum, bref;
	int ref;

	for (unsigned i = 0; i < NR_TEST_CASES; i++) {
		a = convert_to_bitblock<nbits, unsigned>(i);
		for (unsigned j = 0; j < NR_TEST_CASES; j++) {
			b = convert_to_bitblock<nbits, unsigned>(j);
			ref = i + j;
			bref = convert_to_bitblock<nbits + 1, unsigned>(ref);
			carry = add_unsigned(a, b, bsum);
			if (carry) {
				int maxNr = (int(1) << (nbits-1));
				if (ref < maxNr) ReportBinaryArithmeticError("FAIL", "+", a, b, bref, bsum);
			}
			if (bref != bsum) {
				nrOfFailedTestCases++;
				if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "+", a, b, bref, bsum);
			}
			else {
				if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "+", a, b, bref, bsum);
			}
		}
	}
	return nrOfFailedTestCases;
}

// verify bitset subtraction operator
template<size_t nbits>
int VerifyBitsetSubtraction(bool bReportIndividualTestCases = false) {
	const size_t NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTestCases = 0;
	bool borrow = false;
	bitblock<nbits> a, b;
	bitblock<nbits + 1> bsub, bref;
	int ref;

	for (unsigned i = 0; i < NR_TEST_CASES; i++) {
		a = convert_to_bitblock<nbits, unsigned>(i);
		for (unsigned j = 0; j < NR_TEST_CASES; j++) {
			b = convert_to_bitblock<nbits, unsigned>(j);
			ref = i - j;
			bref = convert_to_bitblock<nbits + 1, unsigned>(ref);
			borrow = subtract_unsigned(a, b, bsub);
			if (borrow) {
				if (a >= b) ReportBinaryArithmeticError("FAIL", "-", a, b, bref, bsub);
			}
			if (bref != bsub) {
				nrOfFailedTestCases++;
				if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "-", a, b, bref, bsub);
			}
			else {
				if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "-", a, b, bref, bsub);
			}
		}
	}
	return nrOfFailedTestCases;
}

// verify bitset multiplication operator
template<size_t nbits>
int VerifyBitsetMultiplication(bool bReportIndividualTestCases = false) {
	constexpr size_t rbits = 2 * nbits;
	const size_t NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTestCases = 0;
	bitblock<nbits> a, b;
	bitblock<rbits> bmul, bref;
	int ref;

	for (unsigned i = 0; i < NR_TEST_CASES; i++) {
		a = convert_to_bitblock<nbits, unsigned>(i);
		for (unsigned j = 0; j < NR_TEST_CASES; j++) {
			b = convert_to_bitblock<nbits, unsigned>(j);
			ref = i * j;
			bref = convert_to_bitblock<rbits, unsigned>(ref);
			multiply_unsigned(a, b, bmul);
			if (bref != bmul) {
				nrOfFailedTestCases++;
				if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "-", a, b, bref, bmul);
			}
			else {
				if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "-", a, b, bref, bmul);
			}
		}
	}
	return nrOfFailedTestCases;
}

// verify bitset division operator
template<size_t nbits>
int VerifyBitsetDivision(bool bReportIndividualTestCases = false) {
	constexpr size_t rbits = 2 * nbits;
	const size_t NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTestCases = 0;
	bitblock<nbits> a, b;
	bitblock<rbits> bdiv, bref;
	int ref;

	for (unsigned i = 0; i < NR_TEST_CASES; i++) {
		a = convert_to_bitblock<nbits, unsigned>(i);
		for (unsigned j = 1; j < NR_TEST_CASES; j++) {
			b = convert_to_bitblock<nbits, unsigned>(j);
			ref = i / j;
			bref = convert_to_bitblock<rbits, unsigned>(ref);
			integer_divide_unsigned(a, b, bdiv);
			if (bref != bdiv) {
				nrOfFailedTestCases++;
				if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "-", a, b, bref, bdiv);
			}
			else {
				if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "-", a, b, bref, bdiv);
			}
		}
	}
	return nrOfFailedTestCases;
}

} // namespace sw::universal::internal
