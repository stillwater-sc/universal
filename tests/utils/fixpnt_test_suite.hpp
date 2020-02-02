#pragma once
//  fixpnt_test_suite.hpp : arithmetic/logic test suite for arbitrary fixed point number systems
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <vector>
#include <iostream>
#include <typeinfo>
#include <random>
#include <limits>
// test helpers, such as, ReportTestResults
#include "test_helpers.hpp"

namespace sw {
namespace unum {

#define FIXPNT_TABLE_WIDTH 20
template<size_t nbits, size_t rbits>
void ReportBinaryArithmeticError(std::string test_case, std::string op, const fixpnt<nbits, rbits>& lhs, const fixpnt<nbits, rbits>& rhs, const fixpnt<nbits, rbits>& ref, const fixpnt<nbits, rbits>& result) {
	auto old_precision = std::cerr.precision();
	std::cerr << test_case << " "
		<< std::setprecision(20)
		<< std::setw(FIXPNT_TABLE_WIDTH) << lhs
		<< " " << op << " "
		<< std::setw(FIXPNT_TABLE_WIDTH) << rhs
		<< " != "
		<< std::setw(FIXPNT_TABLE_WIDTH) << ref << " instead it yielded "
		<< std::setw(FIXPNT_TABLE_WIDTH) << result
		<< " " << to_binary(ref) << " vs " << to_binary(result)
		<< std::setprecision(old_precision)
		<< std::endl;
}

template<size_t nbits, size_t rbits>
void ReportBinaryArithmeticSuccess(std::string test_case, std::string op, const fixpnt<nbits, rbits>& lhs, const fixpnt<nbits, rbits>& rhs, const fixpnt<nbits, rbits>& ref, const fixpnt<nbits, rbits>& result) {
	auto old_precision = std::cerr.precision();
	std::cerr << test_case << " "
		<< std::setprecision(20)
		<< std::setw(FIXPNT_TABLE_WIDTH) << lhs
		<< " " << op << " "
		<< std::setw(FIXPNT_TABLE_WIDTH) << rhs
		<< " == "
		<< std::setw(FIXPNT_TABLE_WIDTH) << ref << " matches reference "
		<< std::setw(FIXPNT_TABLE_WIDTH) << result
		<< " " << to_binary(ref) << " vs " << to_binary(result)
		<< std::setprecision(old_precision)
		<< std::endl;
}


// enumerate all addition cases for an fixpnt<nbits,rbits> configuration
template<size_t nbits, size_t rbits>
int VerifyAddition(std::string tag, bool bReportIndividualTestCases) {
	constexpr size_t NR_VALUES = (size_t(1) << nbits);
	int nrOfFailedTests = 0;
	fixpnt<nbits, rbits> a, b, result, cref;
	double ref;

	double da, db;
	for (size_t i = 0; i < NR_VALUES; i++) {
		a.set_raw_bits(i);
		da = double(a);
		for (size_t j = 0; j < NR_VALUES; j++) {
			b.set_raw_bits(j);
			db = double(b);
			ref = da + db;
#if FIXPNT_THROW_ARITHMETIC_EXCEPTION
			// catching overflow
			try {
				result = a + b;
			}
			catch (...) {
				if (ref > max_int<nbits>() || iref < min_int<nbits>()) {
					// correctly caught the exception
					continue;
				}
				else {
					nrOfFailedTests++;
				}
			}

#else
			result = a + b;
#endif // FIXPNT_THROW_ARITHMETIC_EXCEPTION
			cref = ref;
			if (result != cref) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "+", a, b, cref, result);
			}
			else {
				//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "+", a, b, cref, result);
			}
			if (nrOfFailedTests > 100) return nrOfFailedTests;
		}
		if (i % 1024 == 0) std::cout << '.';
	}
	std::cout << std::endl;
	return nrOfFailedTests;
}


// enumerate all subtraction cases for an fixpnt<nbits,rbits> configuration
template<size_t nbits, size_t rbits>
int VerifySubtraction(std::string tag, bool bReportIndividualTestCases) {
	constexpr size_t NR_VALUES = (size_t(1) << nbits);
	int nrOfFailedTests = 0;
	fixpnt<nbits, rbits> a, b, result, cref;
	double ref;

	double da, db;
	for (size_t i = 0; i < NR_VALUES; i++) {
		a.set_raw_bits(i);
		da = double(a);
		for (size_t j = 0; j < NR_VALUES; j++) {
			b.set_raw_bits(j);
			db = double(b);
			ref = da - db;
#if FIXPNT_THROW_ARITHMETIC_EXCEPTION
			// catching overflow
			try {
				result = a - b;
			}
			catch (...) {
				if (ref > max_int<nbits>() || iref < min_int<nbits>()) {
					// correctly caught the exception
					continue;
				}
				else {
					nrOfFailedTests++;
				}
			}

#else
			result = a - b;
#endif // FIXPNT_THROW_ARITHMETIC_EXCEPTION
			cref = ref;
			if (result != cref) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "-", a, b, cref, result);
			}
			else {
				// if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "-", a, b, cref, result);
			}
			if (nrOfFailedTests > 100) return nrOfFailedTests;
		}
		if (i % 1024 == 0) std::cout << '.';
	}
	std::cout << std::endl;
	return nrOfFailedTests;
}

// enumerate all multiplication cases for an fixpnt<nbits,rbits> configuration
template<size_t nbits, size_t rbits>
int VerifyMultiplication(std::string tag, bool bReportIndividualTestCases) {
	constexpr size_t NR_VALUES = (size_t(1) << nbits);
	int nrOfFailedTests = 0;
	fixpnt<nbits, rbits> a, b, result, cref;
	double ref;

	double da, db;
	for (size_t i = 0; i < NR_VALUES; i++) {
		a.set_raw_bits(i);
		da = double(a);
		for (size_t j = 0; j < NR_VALUES; j++) {
			b.set_raw_bits(j);
			db = double(b);
			ref = da * db;
#if FIXPNT_THROW_ARITHMETIC_EXCEPTION
			// catching overflow
			try {
				result = a * b;
			}
			catch (...) {
				if (ref > max_int<nbits>() || iref < min_int<nbits>()) {
					// correctly caught the exception
					continue;
				}
				else {
					nrOfFailedTests++;
				}
			}

#else
			result = a * b;
#endif // FIXPNT_THROW_ARITHMETIC_EXCEPTION
			cref = ref;
			if (result != cref) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "*", a, b, cref, result);
			}
			else {
				// if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "*", a, b, cref, result);
			}
			if (nrOfFailedTests > 100) return nrOfFailedTests;
		}
		if (i % 1024 == 0) std::cout << '.';
	}
	std::cout << std::endl;
	return nrOfFailedTests;
}

// enumerate all division cases for an fixpnt<nbits,rbits> configuration
template<size_t nbits, size_t rbits>
int VerifyDivision(std::string tag, bool bReportIndividualTestCases) {
	constexpr size_t NR_VALUES = (size_t(1) << nbits);
	int nrOfFailedTests = 0;
	fixpnt<nbits, rbits> a, b, result, cref;
	double ref;

	double da, db;
	for (size_t i = 0; i < NR_VALUES; i++) {
		a.set_raw_bits(i);
		da = double(a);
		for (size_t j = 0; j < NR_VALUES; j++) {
			b.set_raw_bits(j);
			db = double(b);
			if (j != 0) {
				ref = da / db;
			}
			else {
				ref = 0;
			}
#if FIXPNT_THROW_ARITHMETIC_EXCEPTION
			try {
				result = a / b;
			}
			catch (...) {
				if (ref > max_int<nbits>() || iref < min_int<nbits>()) {
					// correctly caught the exception
					continue;
				}
				else {
					nrOfFailedTests++;
				}
			}

#else
			result = a / b;
#endif // FIXPNT_THROW_ARITHMETIC_EXCEPTION
			cref = ref;
			if (result != cref) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "/", a, b, cref, result);
			}
			else {
				// if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "/", a, b, cref, result);
			}
			if (nrOfFailedTests > 100) return nrOfFailedTests;
		}
		if (i % 1024 == 0) std::cout << '.';
	}
	std::cout << std::endl;
	return nrOfFailedTests;
}

} // namespace unum
} // namespace sw

