//  logic.cpp : logic operators test suite for block binary numbers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <iostream>
#include <iomanip>
#include <string>

#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult

namespace sw {
namespace universal {

#define INTEGER_TABLE_WIDTH 20
template<unsigned nbits, typename BlockType>
void ReportBinaryLogicError(const std::string& test_case, const std::string& op, const blockbinary<nbits, BlockType>& lhs, const blockbinary<nbits, BlockType>& rhs, bool iref, bool iresult) {
	auto old_precision = std::cerr.precision();
	std::cerr << test_case << " "
		<< std::setprecision(20)
		<< std::setw(INTEGER_TABLE_WIDTH) << to_hex(lhs, true)
		<< " " << op << " "
		<< std::setw(INTEGER_TABLE_WIDTH) << to_hex(rhs, true)
		<< " != "
		<< std::setw(INTEGER_TABLE_WIDTH) << iref << " instead it yielded "
		<< std::setw(INTEGER_TABLE_WIDTH) << iresult
		<< std::setprecision(old_precision)
		<< std::endl;
}

// enumerate all less than cases for an blockbinary<nbits, BlockType> configuration
template<unsigned nbits, typename BlockType>
int VerifyEqual(bool bReportIndividualTestCases) {
	constexpr unsigned NR_INTEGERS = (unsigned(1) << nbits);
	int nrOfFailedTests = 0;
	blockbinary<nbits, BlockType> ia, ib;
	bool iresult, iref;

	int64_t i64a, i64b;
	for (unsigned i = 0; i < NR_INTEGERS; i++) {
		ia.setbits(i);
		i64a = (long long)(ia);
		for (unsigned j = 0; j < NR_INTEGERS; j++) {
			ib.setbits(j);
			i64b = (long long)(ib);
			iref = i64a == i64b;
			iresult = ia == ib;
			if (iresult != iref) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportBinaryLogicError("FAIL", "==", ia, ib, iref, iresult);
			}
			else {
				//if (bReportIndividualTestCases) ReportBinaryLogicSuccess("PASS", "==", ia, ib, iref, iresult);
			}
			if (nrOfFailedTests > 100) return nrOfFailedTests;
		}
		if (i % 1024 == 0) std::cout << '.';
	}
	std::cout << std::endl;
	return nrOfFailedTests;
}

// enumerate all less than or equal cases for an blockbinary<nbits, BlockType> configuration
template<unsigned nbits, typename BlockType>
int VerifyNotEqual(bool bReportIndividualTestCases) {
	constexpr unsigned NR_INTEGERS = (unsigned(1) << nbits);
	int nrOfFailedTests = 0;
	blockbinary<nbits, BlockType> ia, ib;
	bool iresult, iref;

	int64_t i64a, i64b;
	for (unsigned i = 0; i < NR_INTEGERS; i++) {
		ia.setbits(i);
		i64a = (long long)(ia);
		for (unsigned j = 0; j < NR_INTEGERS; j++) {
			ib.setbits(j);
			i64b = (long long)(ib);
			iref = i64a != i64b;
			iresult = ia != ib;
			if (iresult != iref) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportBinaryLogicError("FAIL", "!=", ia, ib, iref, iresult);
			}
			else {
				//if (bReportIndividualTestCases) ReportBinaryLogicSuccess("PASS", "!=", ia, ib, iref, iresult);
			}
			if (nrOfFailedTests > 100) return nrOfFailedTests;
		}
		if (i % 1024 == 0) std::cout << '.';
	}
	std::cout << std::endl;
	return nrOfFailedTests;
}

// enumerate all less than cases for an blockbinary<nbits, BlockType> configuration
template<unsigned nbits, typename BlockType>
int VerifyLessThan(bool bReportIndividualTestCases) {
	constexpr unsigned NR_INTEGERS = (unsigned(1) << nbits);
	int nrOfFailedTests = 0;
	blockbinary<nbits, BlockType> ia, ib;
	bool iresult, iref;

	int64_t i64a, i64b;
	for (unsigned i = 0; i < NR_INTEGERS; i++) {
		ia.setbits(i);
		i64a = (long long)(ia);
		for (unsigned j = 0; j < NR_INTEGERS; j++) {
			ib.setbits(j);
			i64b = (long long)(ib);
			iref = i64a < i64b;
			iresult = ia < ib;
			if (iresult != iref) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportBinaryLogicError("FAIL", "<", ia, ib, iref, iresult);
			}
			else {
				//if (bReportIndividualTestCases) ReportBinaryLogicSuccess("PASS", "<", ia, ib, iref, iresult);
			}
			if (nrOfFailedTests > 100) return nrOfFailedTests;
		}
		if (i % 1024 == 0) std::cout << '.';
	}
	std::cout << std::endl;
	return nrOfFailedTests;
}

// enumerate all less than or equal cases for an blockbinary<nbits, BlockType> configuration
template<unsigned nbits, typename BlockType>
int VerifyLessOrEqualThan(bool bReportIndividualTestCases) {
	constexpr unsigned NR_INTEGERS = (unsigned(1) << nbits);
	int nrOfFailedTests = 0;
	blockbinary<nbits, BlockType> ia, ib;
	bool iresult, iref;

	int64_t i64a, i64b;
	for (unsigned i = 0; i < NR_INTEGERS; i++) {
		ia.setbits(i);
		i64a = (long long)(ia);
		for (unsigned j = 0; j < NR_INTEGERS; j++) {
			ib.setbits(j);
			i64b = (long long)(ib);
			iref = i64a <= i64b;
			iresult = ia <= ib;
			if (iresult != iref) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportBinaryLogicError("FAIL", "<=", ia, ib, iref, iresult);
			}
			else {
				//if (bReportIndividualTestCases) ReportBinaryLogicSuccess("PASS", "<=", ia, ib, iref, iresult);
			}
			if (nrOfFailedTests > 100) return nrOfFailedTests;
		}
		if (i % 1024 == 0) std::cout << '.';
	}
	std::cout << std::endl;
	return nrOfFailedTests;
}

// enumerate all greater than cases for an blockbinary<nbits, BlockType> configuration
template<unsigned nbits, typename BlockType>
int VerifyGreaterThan(bool bReportIndividualTestCases) {
	constexpr unsigned NR_INTEGERS = (unsigned(1) << nbits);
	int nrOfFailedTests = 0;
	blockbinary<nbits, BlockType> ia, ib;
	bool iresult, iref;

	int64_t i64a, i64b;
	for (unsigned i = 0; i < NR_INTEGERS; i++) {
		ia.setbits(i);
		i64a = (long long)(ia);
		for (unsigned j = 0; j < NR_INTEGERS; j++) {
			ib.setbits(j);
			i64b = (long long)(ib);
			iref = i64a < i64b;
			iresult = ia < ib;
			if (iresult != iref) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportBinaryLogicError("FAIL", ">", ia, ib, iref, iresult);
			}
			else {
				//if (bReportIndividualTestCases) ReportBinaryLogicSuccess("PASS", ">", ia, ib, iref, iresult);
			}
			if (nrOfFailedTests > 100) return nrOfFailedTests;
		}
		if (i % 1024 == 0) std::cout << '.';
	}
	std::cout << std::endl;
	return nrOfFailedTests;
}

// enumerate all greater than or equal cases for an blockbinary<nbits, BlockType> configuration
template<unsigned nbits, typename BlockType>
int VerifyGreaterOrEqualThan(bool bReportIndividualTestCases) {
	constexpr unsigned NR_INTEGERS = (unsigned(1) << nbits);
	int nrOfFailedTests = 0;
	blockbinary<nbits, BlockType> ia, ib;
	bool iresult, iref;

	int64_t i64a, i64b;
	for (unsigned i = 0; i < NR_INTEGERS; i++) {
		ia.setbits(i);
		i64a = (long long)(ia);
		for (unsigned j = 0; j < NR_INTEGERS; j++) {
			ib.setbits(j);
			i64b = (long long)(ib);
			iref = i64a >= i64b;
			iresult = ia >= ib;
			if (iresult != iref) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportBinaryLogicError("FAIL", ">=", ia, ib, iref, iresult);
			}
			else {
				//if (bReportIndividualTestCases) ReportBinaryLogicSuccess("PASS", ">=", ia, ib, iref, iresult);
			}
			if (nrOfFailedTests > 100) return nrOfFailedTests;
		}
		if (i % 1024 == 0) std::cout << '.';
	}
	std::cout << std::endl;
	return nrOfFailedTests;
}

} // namespace universal
} // namespace sw

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace sw::universal;

	std::string tag = "blockbinary logic operator tests";

#if MANUAL_TESTING

	// demonstration of the problem of maxneg in modular arithmetic
	int8_t ia, ib, ic;
	ia = 0; 
	ib = int8_t(0x80);
	ic = ia - ib;
	cout << to_binary(ia) << " - " << to_binary(ib) << " = " << to_binary(ic) << " " << int(ic) << endl;

	ReportTestResult(VerifyEqual<4, uint8_t>(true), "blockbinary<4,uint8_t>", "==");
	ReportTestResult(VerifyNotEqual<4, uint8_t>(true), "blockbinary<4,uint8_t>", "!=");
	ReportTestResult(VerifyLessThan<4, uint8_t>(true), "blockbinary<4,uint8_t>", "<");
	ReportTestResult(VerifyLessOrEqualThan<4, uint8_t>(true), "blockbinary<4,uint8_t>", "<=");
	ReportTestResult(VerifyGreaterThan<4, uint8_t>(true), "blockbinary<4,uint8_t>", ">");
	ReportTestResult(VerifyGreaterOrEqualThan<4, uint8_t>(true), "blockbinary<4,uint8_t>", ">=");

	cout << "done" << endl;

	return EXIT_SUCCESS;
#else
	std::cout << "blockbinary logic operator verfication" << std::endl;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	nrOfFailedTestCases += ReportTestResult(VerifyEqual<8, uint8_t>(bReportIndividualTestCases), "blockbinary<8,uint8_t>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyNotEqual<8, uint8_t>(bReportIndividualTestCases), "blockbinary<8,uint8_t>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLessThan<8, uint8_t>(bReportIndividualTestCases), "blockbinary<8,uint8_t>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLessOrEqualThan<8, uint8_t>(bReportIndividualTestCases), "blockbinary<8,uint8_t>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyGreaterThan<8, uint8_t>(bReportIndividualTestCases), "blockbinary<8,uint8_t>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyGreaterOrEqualThan<8, uint8_t>(bReportIndividualTestCases), "blockbinary<8,uint8_t>", ">=");

#if STRESS_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifyEqual<12, uint8_t>(bReportIndividualTestCases), "blockbinary<12,uint8_t>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyNotEqual<12, uint8_t>(bReportIndividualTestCases), "blockbinary<12,uint8_t>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLessThan<12, uint8_t>(bReportIndividualTestCases), "blockbinary<12,uint8_t>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLessOrEqualThan<12, uint8_t>(bReportIndividualTestCases), "blockbinary<12,uint8_t>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyGreaterThan<12, uint8_t>(bReportIndividualTestCases), "blockbinary<12,uint8_t>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyGreaterOrEqualThan<12, uint8_t>(bReportIndividualTestCases), "blockbinary<12,uint8_t>", ">=");

	nrOfFailedTestCases += ReportTestResult(VerifyEqual<16, uint16_t>(bReportIndividualTestCases), "blockbinary<16,uint16_t>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyNotEqual<16, uint16_t>(bReportIndividualTestCases), "blockbinary<16,uint16_t>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLessThan<16, uint16_t>(bReportIndividualTestCases), "blockbinary<16,uint16_t>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLessOrEqualThan<16, uint16_t>(bReportIndividualTestCases), "blockbinary<16,uint16_t>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyGreaterThan<16, uint16_t>(bReportIndividualTestCases), "blockbinary<16,uint16_t>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyGreaterOrEqualThan<16, uint16_t>(bReportIndividualTestCases), "blockbinary<16,uint16_t>", ">=");
#endif // STRESS_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
