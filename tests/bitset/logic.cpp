//  logic.cpp : test suite for bitset logic operators
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include "../tests/test_helpers.hpp"
#include "../../bitset/bitset_logic.hpp"
#include "../../bitset/bitset_helpers.hpp"


using namespace std;
using namespace sw::unum;

template<size_t nbits>
int ValidateBitsetLogicLessThan() {
	const size_t NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTestCases = 0;
	bitset<nbits> a, b;
	bool ref, bref;

	for (unsigned i = 0; i < NR_TEST_CASES; i++) {
		a = convert_to_bitset<nbits, unsigned>(i);
		for (unsigned j = 0; j < NR_TEST_CASES; j++) {
			b = convert_to_bitset<nbits, unsigned>(j);
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
int ValidateBitsetLogicGreaterThan() {
	const size_t NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTestCases = 0;
	bitset<nbits> a, b;
	bool ref, bref;

	for (unsigned i = 0; i < NR_TEST_CASES; i++) {
		a = convert_to_bitset<nbits, unsigned>(i);
		for (unsigned j = 0; j < NR_TEST_CASES; j++) {
			b = convert_to_bitset<nbits, unsigned>(j);
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
int ValidateBitsetLogicEqual() {
	const size_t NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTestCases = 0;
	bitset<nbits> a, b;
	bool ref, bref;

	for (unsigned i = 0; i < NR_TEST_CASES; i++) {
		a = convert_to_bitset<nbits, unsigned>(i);
		for (unsigned j = 0; j < NR_TEST_CASES; j++) {
			b = convert_to_bitset<nbits, unsigned>(j);
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
int ValidateBitsetLogicNotEqual() {
	const size_t NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTestCases = 0;
	bitset<nbits> a, b;
	bool ref, bref;

	for (unsigned i = 0; i < NR_TEST_CASES; i++) {
		a = convert_to_bitset<nbits, unsigned>(i);
		for (unsigned j = 0; j < NR_TEST_CASES; j++) {
			b = convert_to_bitset<nbits, unsigned>(j);
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
int ValidateBitsetLogicLessOrEqualThan() {
	const size_t NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTestCases = 0;
	bitset<nbits> a, b;
	bool ref, bref;

	for (unsigned i = 0; i < NR_TEST_CASES; i++) {
		a = convert_to_bitset<nbits, unsigned>(i);
		for (unsigned j = 0; j < NR_TEST_CASES; j++) {
			b = convert_to_bitset<nbits, unsigned>(j);
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
int ValidateBitsetLogicGreaterOrEqualThan() {
	const size_t NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTestCases = 0;
	bitset<nbits> a, b;
	bool ref, bref;

	for (unsigned i = 0; i < NR_TEST_CASES; i++) {
		a = convert_to_bitset<nbits, unsigned>(i);
		for (unsigned j = 0; j < NR_TEST_CASES; j++) {
			b = convert_to_bitset<nbits, unsigned>(j);
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
	const size_t nbits = 8;
	const size_t es = 2;
	const bool _sign = false; // positive regime
	int nrOfFailedTestCases = 0;

#if MANUAL_TESTING
	bitset<nbits> a, b = convert_to_bitset<nbits, uint32_t>(-2);

	bool gt = a > b; 
	bool let = a <= b;
	std::cout << gt << " " << let << endl;


	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicEqual<3>(), "bitset<3>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicNotEqual<3>(), "bitset<3>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicLessThan<3>(), "bitset<3>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicGreaterThan<3>(), "bitset<3>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicLessOrEqualThan<3>(), "bitset<3>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicGreaterOrEqualThan<3>(), "bitset<3>", ">=");

#else

	cout << "Logic: operator==()" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicEqual<3>(), "bitset<3>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicEqual<4>(), "bitset<4>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicEqual<5>(), "bitset<5>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicEqual<6>(), "bitset<6>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicEqual<7>(), "bitset<7>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicEqual<8>(), "bitset<8>", "==");

	cout << "Logic: operator!=()" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicNotEqual<3>(), "bitset<3>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicNotEqual<4>(), "bitset<4>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicNotEqual<5>(), "bitset<5>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicNotEqual<6>(), "bitset<6>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicNotEqual<7>(), "bitset<7>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicNotEqual<8>(), "bitset<8>", "!=");

	std::cout << "Logic: operator<()" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicLessThan<3>(), "bitset<3>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicLessThan<4>(), "bitset<4>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicLessThan<5>(), "bitset<5>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicLessThan<6>(), "bitset<6>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicLessThan<7>(), "bitset<7>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicLessThan<8>(), "bitset<8>", "<");

	std::cout << "Logic: operator<=()" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicLessOrEqualThan<3>(), "bitset<3>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicLessOrEqualThan<4>(), "bitset<4>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicLessOrEqualThan<5>(), "bitset<5>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicLessOrEqualThan<6>(), "bitset<6>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicLessOrEqualThan<7>(), "bitset<7>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicLessOrEqualThan<8>(), "bitset<8>", "<=");

	std::cout << "Logic: operator>()" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicGreaterThan<3>(), "bitset<3>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicGreaterThan<4>(), "bitset<4>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicGreaterThan<5>(), "bitset<5>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicGreaterThan<6>(), "bitset<6>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicGreaterThan<7>(), "bitset<7>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicGreaterThan<8>(), "bitset<8>", ">");

	std::cout << "Logic: operator>=()" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicGreaterOrEqualThan<3>(), "bitset<3>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicGreaterOrEqualThan<4>(), "bitset<4>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicGreaterOrEqualThan<5>(), "bitset<5>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicGreaterOrEqualThan<6>(), "bitset<6>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicGreaterOrEqualThan<7>(), "bitset<7>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetLogicGreaterOrEqualThan<8>(), "bitset<8>", ">=");

#if STRESS_TESTING

#endif // STRESS_TESTING

#endif // MANUAL_TESTING

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



