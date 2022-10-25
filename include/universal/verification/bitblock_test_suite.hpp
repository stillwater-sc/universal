#pragma once
//  bitblock_test_suite.hpp : bitblock-based arithmetic test suite
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal { namespace internal {

template<unsigned nbits, unsigned rbits>
void ReportBinaryArithmeticError(const std::string& test_case, const std::string& op, const bitblock<nbits>& lhs, const bitblock<nbits>& rhs, const bitblock<rbits>& ref, const bitblock<rbits>& result) {
	constexpr unsigned OPERAND_COLUMN_WIDTH = nbits;
	constexpr unsigned RESULT_COLUMN_WIDTH = rbits;
	std::cerr << test_case << " "
		<< std::setw(OPERAND_COLUMN_WIDTH) << lhs
		<< " " << op << " "
		<< std::setw(OPERAND_COLUMN_WIDTH) << rhs
		<< " != "
		<< std::setw(RESULT_COLUMN_WIDTH) << ref << " instead it yielded "
		<< std::setw(RESULT_COLUMN_WIDTH) << result
		<< std::endl;
}

template<unsigned nbits, unsigned rbits>
void ReportBinaryArithmeticSuccess(const std::string& test_case, const std::string& op, const bitblock<nbits>& lhs, const bitblock<nbits>& rhs, const bitblock<rbits>& ref, const bitblock<rbits>& result) {
	constexpr unsigned OPERAND_COLUMN_WIDTH = nbits;
	constexpr unsigned RESULT_COLUMN_WIDTH = rbits;
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
template<unsigned nbits>
int VerifyBitsetAddition(bool bReportIndividualTestCases = false) {
	constexpr unsigned NR_TEST_CASES = (1u << nbits);
	int nrOfFailedTestCases = 0;
	bool carry;
	bitblock<nbits> a, b;
	bitblock<nbits + 1> bsum, bref;
	int ref;

	for (unsigned i = 0; i < NR_TEST_CASES; i++) {
		a = convert_to_bitblock<nbits, unsigned>(i);
		for (unsigned j = 0; j < NR_TEST_CASES; j++) {
			b = convert_to_bitblock<nbits, unsigned>(j);
			ref = static_cast<int>(i + j);
			bref = convert_to_bitblock<nbits + 1, unsigned>(static_cast<unsigned>(ref));
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
template<unsigned nbits>
int VerifyBitsetSubtraction(bool bReportIndividualTestCases = false) {
	constexpr unsigned NR_TEST_CASES = (1u << nbits);
	int nrOfFailedTestCases = 0;
	bool borrow = false;
	bitblock<nbits> a, b;
	bitblock<nbits + 1> bsub, bref;
	int ref;

	for (unsigned i = 0; i < NR_TEST_CASES; i++) {
		a = convert_to_bitblock<nbits, unsigned>(i);
		for (unsigned j = 0; j < NR_TEST_CASES; j++) {
			b = convert_to_bitblock<nbits, unsigned>(j);
			ref = static_cast<int>(i - j);
			bref = convert_to_bitblock<nbits + 1, unsigned>(static_cast<unsigned>(ref));
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
template<unsigned nbits>
int VerifyBitsetMultiplication(bool bReportIndividualTestCases = false) {
	constexpr unsigned rbits = 2 * nbits;
	constexpr unsigned NR_TEST_CASES = (1u << nbits);
	int nrOfFailedTestCases = 0;
	bitblock<nbits> a, b;
	bitblock<rbits> bmul, bref;
	int ref;

	for (unsigned i = 0; i < NR_TEST_CASES; i++) {
		a = convert_to_bitblock<nbits, unsigned>(i);
		for (unsigned j = 0; j < NR_TEST_CASES; j++) {
			b = convert_to_bitblock<nbits, unsigned>(j);
			ref = static_cast<int>(i * j);
			bref = convert_to_bitblock<rbits, unsigned>(static_cast<unsigned>(ref));
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
template<unsigned nbits>
int VerifyBitsetDivision(bool bReportIndividualTestCases = false) {
	constexpr unsigned rbits = 2 * nbits;
	constexpr unsigned NR_TEST_CASES = (1u << nbits);
	int nrOfFailedTestCases = 0;
	bitblock<nbits> a, b;
	bitblock<rbits> bdiv, bref;
	unsigned ref;

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

}}} // namespace sw::universal::internal
