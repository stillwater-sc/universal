// logic.cpp : test suite for bitblock logic operators
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#define BITBLOCK_THROW_ARITHMETIC_EXCEPTION 1
#undef BITBLOCK_ROUND_TIES_AWAY_FROM_ZERO
#undef BITBLOCK_ROUND_TIES_TO_ZERO
#include "universal/bitblock/bitblock.hpp"
// test helpers, such as, ReportTestResults
#include "../utils/test_helpers.hpp"

template<size_t nbits>
int VerifyBitsetLogicLessThan() {
	const size_t NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTestCases = 0;
	sw::unum::bitblock<nbits> a, b;
	bool ref, bref;

	for (unsigned i = 0; i < NR_TEST_CASES; i++) {
		a = sw::unum::convert_to_bitblock<nbits, unsigned>(i);
		for (unsigned j = 0; j < NR_TEST_CASES; j++) {
			b = sw::unum::convert_to_bitblock<nbits, unsigned>(j);
			ref = i < j;
			bref = a < b;
			if (ref != bref) {
				nrOfFailedTestCases++;
				std::cout << a << " < " << b << " fails: reference is " << ref << " actual is " << bref << std::endl;
			}
		}
	}
	return nrOfFailedTestCases;
}

template<size_t nbits>
int VerifyBitsetLogicGreaterThan() {
	const size_t NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTestCases = 0;
	sw::unum::bitblock<nbits> a, b;
	bool ref, bref;

	for (unsigned i = 0; i < NR_TEST_CASES; i++) {
		a = sw::unum::convert_to_bitblock<nbits, unsigned>(i);
		for (unsigned j = 0; j < NR_TEST_CASES; j++) {
			b = sw::unum::convert_to_bitblock<nbits, unsigned>(j);
			ref = i > j;
			bref = a > b;
			if (ref != bref) {
				nrOfFailedTestCases++;
				std::cout << a << " > " << b << " fails: reference is " << ref << " actual is " << bref << std::endl;
			}
		}
	}
	return nrOfFailedTestCases;
}

template<size_t nbits>
int VerifyBitsetLogicEqual() {
	const size_t NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTestCases = 0;
	sw::unum::bitblock<nbits> a, b;
	bool ref, bref;

	for (unsigned i = 0; i < NR_TEST_CASES; i++) {
		a = sw::unum::convert_to_bitblock<nbits, unsigned>(i);
		for (unsigned j = 0; j < NR_TEST_CASES; j++) {
			b = sw::unum::convert_to_bitblock<nbits, unsigned>(j);
			ref = i == j;
			bref = a == b;
			if (ref != bref) {
				nrOfFailedTestCases++;
				std::cout << a << " == " << b << " fails: reference is " << ref << " actual is " << bref << std::endl;
			}
		}
	}
	return nrOfFailedTestCases;
}

template<size_t nbits>
int VerifyBitsetLogicNotEqual() {
	const size_t NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTestCases = 0;
	sw::unum::bitblock<nbits> a, b;
	bool ref, bref;

	for (unsigned i = 0; i < NR_TEST_CASES; i++) {
		a = sw::unum::convert_to_bitblock<nbits, unsigned>(i);
		for (unsigned j = 0; j < NR_TEST_CASES; j++) {
			b = sw::unum::convert_to_bitblock<nbits, unsigned>(j);
			ref = i != j;
			bref = a != b;
			if (ref != bref) {
				nrOfFailedTestCases++;
				std::cout << a << " != " << b << " fails: reference is " << ref << " actual is " << bref << std::endl;
			}
		}
	}
	return nrOfFailedTestCases;
}

template<size_t nbits>
int VerifyBitsetLogicLessOrEqualThan() {
	const size_t NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTestCases = 0;
	sw::unum::bitblock<nbits> a, b;
	bool ref, bref;

	for (unsigned i = 0; i < NR_TEST_CASES; i++) {
		a = sw::unum::convert_to_bitblock<nbits, unsigned>(i);
		for (unsigned j = 0; j < NR_TEST_CASES; j++) {
			b = sw::unum::convert_to_bitblock<nbits, unsigned>(j);
			ref = i <= j;
			bref = a <= b;
			if (ref != bref) {
				nrOfFailedTestCases++;
				std::cout << a << " <= " << b << " fails: reference is " << ref << " actual is " << bref << std::endl;
			}
		}
	}
	return nrOfFailedTestCases;
}

template<size_t nbits>
int VerifyBitsetLogicGreaterOrEqualThan() {
	const size_t NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTestCases = 0;
	sw::unum::bitblock<nbits> a, b;
	bool ref, bref;

	for (unsigned i = 0; i < NR_TEST_CASES; i++) {
		a = sw::unum::convert_to_bitblock<nbits, unsigned>(i);
		for (unsigned j = 0; j < NR_TEST_CASES; j++) {
			b = sw::unum::convert_to_bitblock<nbits, unsigned>(j);
			ref = i >= j;
			bref = a >= b;
			if (ref != bref) {
				nrOfFailedTestCases++;
				std::cout << a << " >= " << b << " fails: reference is " << ref << " actual is " << bref << std::endl;
			}
		}
	}
	return nrOfFailedTestCases;
}

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace std;
	using namespace sw::unum;

	//bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "Bitblock logic operation failed";

#if MANUAL_TESTING
	bitblock<nbits> a, b = convert_to_bitblock<nbits, uint32_t>(-2);

	bool gt = a > b; 
	bool let = a <= b;
	std::cout << gt << " " << let << endl;


	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicEqual<3>(), "bitblock<3>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicNotEqual<3>(), "bitblock<3>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicLessThan<3>(), "bitblock<3>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicGreaterThan<3>(), "bitblock<3>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicLessOrEqualThan<3>(), "bitblock<3>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicGreaterOrEqualThan<3>(), "bitblock<3>", ">=");

#else

	cout << "Logic: operator==()" << endl;
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicEqual<3>(), "bitblock<3>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicEqual<4>(), "bitblock<4>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicEqual<5>(), "bitblock<5>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicEqual<6>(), "bitblock<6>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicEqual<7>(), "bitblock<7>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicEqual<8>(), "bitblock<8>", "==");

	cout << "Logic: operator!=()" << endl;
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicNotEqual<3>(), "bitblock<3>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicNotEqual<4>(), "bitblock<4>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicNotEqual<5>(), "bitblock<5>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicNotEqual<6>(), "bitblock<6>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicNotEqual<7>(), "bitblock<7>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicNotEqual<8>(), "bitblock<8>", "!=");

	std::cout << "Logic: operator<()" << endl;
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicLessThan<3>(), "bitblock<3>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicLessThan<4>(), "bitblock<4>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicLessThan<5>(), "bitblock<5>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicLessThan<6>(), "bitblock<6>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicLessThan<7>(), "bitblock<7>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicLessThan<8>(), "bitblock<8>", "<");

	std::cout << "Logic: operator<=()" << endl;
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicLessOrEqualThan<3>(), "bitblock<3>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicLessOrEqualThan<4>(), "bitblock<4>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicLessOrEqualThan<5>(), "bitblock<5>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicLessOrEqualThan<6>(), "bitblock<6>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicLessOrEqualThan<7>(), "bitblock<7>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicLessOrEqualThan<8>(), "bitblock<8>", "<=");

	std::cout << "Logic: operator>()" << endl;
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicGreaterThan<3>(), "bitblock<3>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicGreaterThan<4>(), "bitblock<4>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicGreaterThan<5>(), "bitblock<5>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicGreaterThan<6>(), "bitblock<6>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicGreaterThan<7>(), "bitblock<7>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicGreaterThan<8>(), "bitblock<8>", ">");

	std::cout << "Logic: operator>=()" << endl;
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicGreaterOrEqualThan<3>(), "bitblock<3>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicGreaterOrEqualThan<4>(), "bitblock<4>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicGreaterOrEqualThan<5>(), "bitblock<5>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicGreaterOrEqualThan<6>(), "bitblock<6>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicGreaterOrEqualThan<7>(), "bitblock<7>", ">=");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetLogicGreaterOrEqualThan<8>(), "bitblock<8>", ">=");

#if STRESS_TESTING

#endif // STRESS_TESTING

#endif // MANUAL_TESTING

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



