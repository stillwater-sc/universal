//  logic.cpp : test suite runner for logic operators on abitrary precision integers
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
// configure the integer arithmetic class
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/integer/integer.hpp>
#include <universal/number/integer/numeric_limits.hpp>
// is representable
#include <universal/functions/isrepresentable.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult used by test suite runner

/*
   The goal of the arbitrary integers is to provide a fast fixed-size integer type
   that enables fast computation with exceptions for overflow, so that the type
   can be used for forward error analysis studies.
*/

namespace sw::universal {

#define INTEGER_TABLE_WIDTH 20
	template<size_t nbits>
	void ReportBinaryLogicError(const std::string& test_case, const std::string& op, const integer<nbits>& lhs, const integer<nbits>& rhs, bool iref, bool iresult) {
		auto old_precision = std::cerr.precision();
		std::cerr << test_case << " "
			<< std::setprecision(20)
			<< std::setw(INTEGER_TABLE_WIDTH) << lhs
			<< " " << op << " "
			<< std::setw(INTEGER_TABLE_WIDTH) << rhs
			<< " != "
			<< std::setw(INTEGER_TABLE_WIDTH) << iref << " instead it yielded "
			<< std::setw(INTEGER_TABLE_WIDTH) << iresult
			<< std::setprecision(old_precision)
			<< std::endl;
	}

	// enumerate all less than cases for an integer<nbits> configuration
	template<size_t nbits>
	int VerifyEqual(bool bReportIndividualTestCases) {
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits> ia, ib;
		bool iresult, iref;

		int64_t i64a, i64b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.setbits(i);
			i64a = int64_t(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.setbits(j);
				i64b = int64_t(ib);
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

	// enumerate all less than or equal cases for an integer<nbits> configuration
	template<size_t nbits>
	int VerifyNotEqual(bool bReportIndividualTestCases) {
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits> ia, ib;
		bool iresult, iref;

		int64_t i64a, i64b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.setbits(i);
			i64a = int64_t(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.setbits(j);
				i64b = int64_t(ib);
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

	// enumerate all less than cases for an integer<nbits> configuration
	template<size_t nbits>
	int VerifyLessThan(bool bReportIndividualTestCases) {
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits> ia, ib;
		bool iresult, iref;

		int64_t i64a, i64b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.setbits(i);
			i64a = int64_t(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.setbits(j);
				i64b = int64_t(ib);
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

	// enumerate all less than or equal cases for an integer<nbits> configuration
	template<size_t nbits>
	int VerifyLessOrEqualThan(bool bReportIndividualTestCases) {
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits> ia, ib;
		bool iresult, iref;

		int64_t i64a, i64b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.setbits(i);
			i64a = int64_t(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.setbits(j);
				i64b = int64_t(ib);
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

	// enumerate all greater than cases for an integer<nbits> configuration
	template<size_t nbits>
	int VerifyGreaterThan(bool bReportIndividualTestCases) {
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits> ia, ib;
		bool iresult, iref;

		int64_t i64a, i64b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.setbits(i);
			i64a = int64_t(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.setbits(j);
				i64b = int64_t(ib);
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

	// enumerate all greater than or equal cases for an integer<nbits> configuration
	template<size_t nbits>
	int VerifyGreaterOrEqualThan(bool bReportIndividualTestCases) {
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits> ia, ib;
		bool iresult, iref;

		int64_t i64a, i64b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.setbits(i);
			i64a = int64_t(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.setbits(j);
				i64b = int64_t(ib);
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

} // namespace sw::universal

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
#ifndef REGRESSION_LEVEL_OVERRIDE
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

std::string convert_to_string(const std::vector<char>& v) {
	std::stringstream ss;
	for (std::vector<char>::const_reverse_iterator rit = v.rbegin(); rit != v.rend(); ++rit) {
		ss << (int)*rit;
	}
	return ss.str();
}

int main()
try {
	using namespace sw::universal;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;
	
	std::string tag = "Integer Logic Operator tests failed";

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyEqual<4>(bReportIndividualTestCases), "integer<4>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyNotEqual<4>(bReportIndividualTestCases), "integer<4>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLessThan<4>(bReportIndividualTestCases), "integer<4>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLessOrEqualThan<4>(bReportIndividualTestCases), "integer<4>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyGreaterThan<4>(bReportIndividualTestCases), "integer<4>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyGreaterOrEqualThan<4>(bReportIndividualTestCases), "integer<4>", ">=");

	std::cout << "done" << std::endl;

	return EXIT_SUCCESS;
#else
	std::cout << "Integer Logic Operator verfication\n";

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyEqual<4>(bReportIndividualTestCases), "integer<4>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyNotEqual<4>(bReportIndividualTestCases), "integer<4>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLessThan<4>(bReportIndividualTestCases), "integer<4>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLessOrEqualThan<4>(bReportIndividualTestCases), "integer<4>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyGreaterThan<4>(bReportIndividualTestCases), "integer<4>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyGreaterOrEqualThan<4>(bReportIndividualTestCases), "integer<4>", ">=");

	nrOfFailedTestCases += ReportTestResult(VerifyEqual<8>(bReportIndividualTestCases), "integer<8>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyNotEqual<8>(bReportIndividualTestCases), "integer<8>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLessThan<8>(bReportIndividualTestCases), "integer<8>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLessOrEqualThan<8>(bReportIndividualTestCases), "integer<8>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyGreaterThan<8>(bReportIndividualTestCases), "integer<8>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyGreaterOrEqualThan<8>(bReportIndividualTestCases), "integer<8>", ">=");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyEqual<10>(bReportIndividualTestCases), "integer<10>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyNotEqual<10>(bReportIndividualTestCases), "integer<10>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLessThan<10>(bReportIndividualTestCases), "integer<10>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLessOrEqualThan<10>(bReportIndividualTestCases), "integer<10>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyGreaterThan<10>(bReportIndividualTestCases), "integer<10>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyGreaterOrEqualThan<10>(bReportIndividualTestCases), "integer<10>", ">=");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyEqual<12>(bReportIndividualTestCases), "integer<12>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyNotEqual<12>(bReportIndividualTestCases), "integer<12>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLessThan<12>(bReportIndividualTestCases), "integer<12>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLessOrEqualThan<12>(bReportIndividualTestCases), "integer<12>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyGreaterThan<12>(bReportIndividualTestCases), "integer<12>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyGreaterOrEqualThan<12>(bReportIndividualTestCases), "integer<12>", ">=");
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyEqual<13>(bReportIndividualTestCases), "integer<13>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyNotEqual<13>(bReportIndividualTestCases), "integer<13>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLessThan<13>(bReportIndividualTestCases), "integer<13>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLessOrEqualThan<13>(bReportIndividualTestCases), "integer<13>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyGreaterThan<13>(bReportIndividualTestCases), "integer<13>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyGreaterOrEqualThan<13>(bReportIndividualTestCases), "integer<13>", ">=");
#endif

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
