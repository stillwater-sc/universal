// components.cpp : tests for regime/exponent/fraction components of a posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include "../../posit/posit.hpp"
#include "../tests/test_helpers.hpp"

using namespace std;
using namespace sw::unum;

template<size_t nbits, size_t es>
int ValidatePositLogicLessThan() {
	const size_t NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTestCases = 0;
	posit<nbits, es> a, b;
	bool ref, pref;

	for (unsigned i = 0; i < NR_TEST_CASES; i++) {
		a = convert_to_bitset<nbits, unsigned>(i);
		for (unsigned j = 0; j < NR_TEST_CASES; j++) {
			b = convert_to_bitset<nbits, unsigned>(j);
			ref = a.to_double() < b.to_double();
			pref = a < b;
			if (ref != pref) {
				nrOfFailedTestCases++;
				std::cout << a << " < " << b << " fails: reference is " << ref << " actual is " << pref << std::endl;
			}
		}
	}
	return nrOfFailedTestCases;
}

template<size_t nbits, size_t es>
int ValidatePositLogicGreaterThan() {
	const size_t NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTestCases = 0;
	posit<nbits, es> a, b;
	bool ref, pref;

	for (unsigned i = 0; i < NR_TEST_CASES; i++) {
		a = convert_to_bitset<nbits, unsigned>(i);
		for (unsigned j = 0; j < NR_TEST_CASES; j++) {
			b = convert_to_bitset<nbits, unsigned>(j);
			ref = a.to_double() > b.to_double();
			pref = a > b;
			if (ref != pref) {
				nrOfFailedTestCases++;
				std::cout << a << " > " << b << " fails: reference is " << ref << " actual is " << pref << std::endl;
			}
		}
	}
	return nrOfFailedTestCases;
}

template<size_t nbits, size_t es>
int ValidatePositLogicEqual() {
	const size_t NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTestCases = 0;
	posit<nbits, es> a, b;
	bool ref, pref;

	for (unsigned i = 0; i < NR_TEST_CASES; i++) {
		a = convert_to_bitset<nbits, unsigned>(i);
		for (unsigned j = 0; j < NR_TEST_CASES; j++) {
			b = convert_to_bitset<nbits, unsigned>(j);
			ref = a.to_double() == b.to_double();
			pref = a == b;
			if (ref != pref) {
				nrOfFailedTestCases++;
				std::cout << a << " == " << b << " fails: reference is " << ref << " actual is " << pref << std::endl;
			}
		}
	}
	return nrOfFailedTestCases;
}

template<size_t nbits, size_t es>
int ValidatePositLogicNotEqual() {
	const size_t NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTestCases = 0;
	posit<nbits, es> a, b;
	bool ref, pref;

	for (unsigned i = 0; i < NR_TEST_CASES; i++) {
		a = convert_to_bitset<nbits, unsigned>(i);
		for (unsigned j = 0; j < NR_TEST_CASES; j++) {
			b = convert_to_bitset<nbits, unsigned>(j);
			ref = a.to_double() != b.to_double();
			pref = a != b;
			if (ref != pref) {
				nrOfFailedTestCases++;
				std::cout << a << " != " << b << " fails: reference is " << ref << " actual is " << pref << std::endl;
			}
		}
	}
	return nrOfFailedTestCases;
}

template<size_t nbits, size_t es>
int ValidatePositLogicLessOrEqualThan() {
	const size_t NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTestCases = 0;
	posit<nbits, es> a, b;
	bool ref, pref;

	for (unsigned i = 0; i < NR_TEST_CASES; i++) {
		a = convert_to_bitset<nbits, unsigned>(i);
		for (unsigned j = 0; j < NR_TEST_CASES; j++) {
			b = convert_to_bitset<nbits, unsigned>(j);
			ref = a.to_double() <= b.to_double();
			pref = a <= b;
			if (ref != pref) {
				nrOfFailedTestCases++;
				std::cout << a << " <= " << b << " fails: reference is " << ref << " actual is " << pref << std::endl;
			}
		}
	}
	return nrOfFailedTestCases;
}

template<size_t nbits, size_t es>
int ValidatePositLogicGreaterOrEqualThan() {
	const size_t NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTestCases = 0;
	posit<nbits, es> a, b;
	bool ref, pref;

	for (unsigned i = 0; i < NR_TEST_CASES; i++) {
		a = convert_to_bitset<nbits, unsigned>(i);
		for (unsigned j = 0; j < NR_TEST_CASES; j++) {
			b = convert_to_bitset<nbits, unsigned>(j);
			ref = a.to_double() >= b.to_double();
			pref = a >= b;
			if (ref != pref) {
				nrOfFailedTestCases++;
				std::cout << a << " >= " << b << " fails: reference is " << ref << " actual is " << pref << std::endl;
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
	double a = NAN;
	double b = INFINITY;
	double c = NAN;
	posit<nbits, es> pa(a), pb(b), pc(c);
	std::cout << pa << " " << pb << " " << pc << std::endl;
	
	bool ref = a < b;
	std::cout << (a == b) << " " << (pa == pb) << std::endl;
	std::cout << (a != b) << " " << (pa != pb) << std::endl;
	std::cout << (a <= b) << " " << (pa <= pb) << std::endl;
	std::cout << (a >= b) << " " << (pa >= pb) << std::endl;
	std::cout << (a <  b) << " " << (pa <  pb) << std::endl;
	std::cout << (a  > b) << " " << (pa  > pb) << std::endl;

	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<3, 0>(), "posit<3,0>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<3, 0>(), "posit<3,0>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<3, 0>(), "posit<3,0>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<3, 0>(), "posit<3,0>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<3, 0>(), "posit<3,0>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<3, 0>(), "posit<3,0>", ">=");

#else

	cout << "Logic: operator==()" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<3, 0>(), "posit<3,0>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<4, 0>(), "posit<4,0>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<4, 1>(), "posit<4,1>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<5, 0>(), "posit<5,0>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<5, 1>(), "posit<5,1>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<5, 2>(), "posit<5,2>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<6, 0>(), "posit<6,0>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<6, 1>(), "posit<6,1>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<6, 2>(), "posit<6,2>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<6, 3>(), "posit<6,3>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<7, 0>(), "posit<7,0>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<7, 1>(), "posit<7,1>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<7, 2>(), "posit<7,2>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<7, 3>(), "posit<7,3>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<8, 0>(), "posit<8,0>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<8, 1>(), "posit<8,1>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<8, 2>(), "posit<8,2>", "==");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicEqual<8, 3>(), "posit<8,3>", "==");

	cout << "Logic: operator!=()" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<3, 0>(), "posit<3,0>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<4, 0>(), "posit<4,0>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<4, 1>(), "posit<4,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<5, 0>(), "posit<5,0>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<5, 1>(), "posit<5,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<5, 2>(), "posit<5,2>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<6, 0>(), "posit<6,0>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<6, 1>(), "posit<6,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<6, 2>(), "posit<6,2>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<6, 3>(), "posit<6,3>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<7, 0>(), "posit<7,0>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<7, 1>(), "posit<7,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<7, 2>(), "posit<7,2>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<7, 3>(), "posit<7,3>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<8, 0>(), "posit<8,0>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<8, 1>(), "posit<8,1>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<8, 2>(), "posit<8,2>", "!=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicNotEqual<8, 3>(), "posit<8,3>", "!=");

	std::cout << "Logic: operator<()" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<3, 0>(), "posit<3,0>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<4, 0>(), "posit<4,0>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<4, 1>(), "posit<4,1>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<5, 0>(), "posit<5,0>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<5, 1>(), "posit<5,1>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<5, 2>(), "posit<5,2>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<6, 0>(), "posit<6,0>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<6, 1>(), "posit<6,1>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<6, 2>(), "posit<6,2>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<6, 3>(), "posit<6,3>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<7, 0>(), "posit<7,0>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<7, 1>(), "posit<7,1>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<7, 2>(), "posit<7,2>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<7, 3>(), "posit<7,3>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<8, 0>(), "posit<8,0>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<8, 1>(), "posit<8,1>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<8, 2>(), "posit<8,2>", "<");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessThan<8, 3>(), "posit<8,3>", "<");

	std::cout << "Logic: operator<=()" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<3, 0>(), "posit<3,0>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<4, 0>(), "posit<4,0>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<4, 1>(), "posit<4,1>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<5, 0>(), "posit<5,0>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<5, 1>(), "posit<5,1>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<5, 2>(), "posit<5,2>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<6, 0>(), "posit<6,0>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<6, 1>(), "posit<6,1>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<6, 2>(), "posit<6,2>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<6, 3>(), "posit<6,3>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<7, 0>(), "posit<7,0>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<7, 1>(), "posit<7,1>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<7, 2>(), "posit<7,2>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<7, 3>(), "posit<7,3>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<8, 0>(), "posit<8,0>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<8, 1>(), "posit<8,1>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<8, 2>(), "posit<8,2>", "<=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicLessOrEqualThan<8, 3>(), "posit<8,3>", "<=");

	std::cout << "Logic: operator>()" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<3, 0>(), "posit<3,0>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<4, 0>(), "posit<4,0>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<4, 1>(), "posit<4,1>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<5, 0>(), "posit<5,0>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<5, 1>(), "posit<5,1>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<5, 2>(), "posit<5,2>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<6, 0>(), "posit<6,0>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<6, 1>(), "posit<6,1>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<6, 2>(), "posit<6,2>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<6, 3>(), "posit<6,3>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<7, 0>(), "posit<7,0>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<7, 1>(), "posit<7,1>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<7, 2>(), "posit<7,2>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<7, 3>(), "posit<7,3>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<8, 0>(), "posit<8,0>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<8, 1>(), "posit<8,1>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<8, 2>(), "posit<8,2>", ">");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterThan<8, 3>(), "posit<8,3>", ">");

	std::cout << "Logic: operator>=()" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<3, 0>(), "posit<3,0>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<4, 0>(), "posit<4,0>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<4, 1>(), "posit<4,1>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<5, 0>(), "posit<5,0>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<5, 1>(), "posit<5,1>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<5, 2>(), "posit<5,2>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<6, 0>(), "posit<6,0>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<6, 1>(), "posit<6,1>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<6, 2>(), "posit<6,2>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<6, 3>(), "posit<6,3>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<7, 0>(), "posit<7,0>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<7, 1>(), "posit<7,1>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<7, 2>(), "posit<7,2>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<7, 3>(), "posit<7,3>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<8, 0>(), "posit<8,0>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<8, 1>(), "posit<8,1>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<8, 2>(), "posit<8,2>", ">=");
	nrOfFailedTestCases += ReportTestResult(ValidatePositLogicGreaterOrEqualThan<8, 3>(), "posit<8,3>", ">=");

#if STRESS_TESTING

#endif // STRESS_TESTING

#endif // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}



