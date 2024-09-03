//  logic.cpp : test suite runner for logic operators on fixed-sized, arbitrary precision integers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <string>
// configure the integer arithmetic class
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/integer/integer.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult used by test suite runner
#include <universal/verification/test_reporters.hpp>

/*
   The goal of the arbitrary integers is to provide a fast fixed-size integer type
   that enables fast computation with exceptions for overflow, so that the type
   can be used for forward error analysis studies.
*/

namespace sw { namespace universal {

#define INTEGER_TABLE_WIDTH 20
	template<unsigned nbits, typename bt>
	void ReportBinaryLogicError(const std::string& test_case, const std::string& op, const integer<nbits, bt>& lhs, const integer<nbits, bt>& rhs, bool iref, bool iresult) {
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
	template<unsigned nbits, typename bt>
	int VerifyEqual(bool reportTestCases) {
		constexpr size_t NR_INTEGERS = (1ull << nbits);
		int nrOfFailedTests = 0;
		integer<nbits, bt> ia, ib;
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
					if (reportTestCases)	ReportBinaryLogicError("FAIL", "==", ia, ib, iref, iresult);
				}
				else {
					//if (reportTestCases) ReportBinaryLogicSuccess("PASS", "==", ia, ib, iref, iresult);
				}
				if (nrOfFailedTests > 100) return nrOfFailedTests;
			}
			if (i % 1024 == 0) std::cout << '.';
		}
		std::cout << std::endl;
		return nrOfFailedTests;
	}

	// enumerate all less than or equal cases for an integer<nbits> configuration
	template<unsigned nbits, typename bt>
	int VerifyNotEqual(bool reportTestCases) {
		constexpr size_t NR_INTEGERS = (1ull << nbits);
		int nrOfFailedTests = 0;
		integer<nbits, bt> ia, ib;
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
					if (reportTestCases)	ReportBinaryLogicError("FAIL", "!=", ia, ib, iref, iresult);
				}
				else {
					//if (reportTestCases) ReportBinaryLogicSuccess("PASS", "!=", ia, ib, iref, iresult);
				}
				if (nrOfFailedTests > 100) return nrOfFailedTests;
			}
			if (i % 1024 == 0) std::cout << '.';
		}
		std::cout << std::endl;
		return nrOfFailedTests;
	}

	// enumerate all less than cases for an integer<nbits> configuration
	template<unsigned nbits, typename bt>
	int VerifyLessThan(bool reportTestCases) {
		constexpr size_t NR_INTEGERS = (1ull << nbits);
		int nrOfFailedTests = 0;
		integer<nbits, bt> ia, ib;
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
					if (reportTestCases)	ReportBinaryLogicError("FAIL", "<", ia, ib, iref, iresult);
				}
				else {
					//if (reportTestCases) ReportBinaryLogicSuccess("PASS", "<", ia, ib, iref, iresult);
				}
				if (nrOfFailedTests > 100) return nrOfFailedTests;
			}
			if (i % 1024 == 0) std::cout << '.';
		}
		std::cout << std::endl;
		return nrOfFailedTests;
	}

	// enumerate all less than or equal cases for an integer<nbits> configuration
	template<unsigned nbits, typename bt>
	int VerifyLessOrEqualThan(bool reportTestCases) {
		constexpr size_t NR_INTEGERS = (1ull << nbits);
		int nrOfFailedTests = 0;
		integer<nbits, bt> ia, ib;
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
					if (reportTestCases)	ReportBinaryLogicError("FAIL", "<=", ia, ib, iref, iresult);
				}
				else {
					//if (reportTestCases) ReportBinaryLogicSuccess("PASS", "<=", ia, ib, iref, iresult);
				}
				if (nrOfFailedTests > 100) return nrOfFailedTests;
			}
			if (i % 1024 == 0) std::cout << '.';
		}
		std::cout << std::endl;
		return nrOfFailedTests;
	}

	// enumerate all greater than cases for an integer<nbits> configuration
	template<unsigned nbits, typename bt>
	int VerifyGreaterThan(bool reportTestCases) {
		constexpr size_t NR_INTEGERS = (1ull << nbits);
		int nrOfFailedTests = 0;
		integer<nbits, bt> ia, ib;
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
					if (reportTestCases)	ReportBinaryLogicError("FAIL", ">", ia, ib, iref, iresult);
				}
				else {
					//if (reportTestCases) ReportBinaryLogicSuccess("PASS", ">", ia, ib, iref, iresult);
				}
				if (nrOfFailedTests > 100) return nrOfFailedTests;
			}
			if (i % 1024 == 0) std::cout << '.';
		}
		std::cout << std::endl;
		return nrOfFailedTests;
	}

	// enumerate all greater than or equal cases for an integer<nbits> configuration
	template<unsigned nbits, typename bt>
	int VerifyGreaterOrEqualThan(bool reportTestCases) {
		constexpr size_t NR_INTEGERS = (1ull << nbits);
		int nrOfFailedTests = 0;
		integer<nbits, bt> ia, ib;
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
					if (reportTestCases)	ReportBinaryLogicError("FAIL", ">=", ia, ib, iref, iresult);
				}
				else {
					//if (reportTestCases) ReportBinaryLogicSuccess("PASS", ">=", ia, ib, iref, iresult);
				}
				if (nrOfFailedTests > 100) return nrOfFailedTests;
			}
			if (i % 1024 == 0) std::cout << '.';
		}
		std::cout << std::endl;
		return nrOfFailedTests;
	}

}} // namespace sw::universal

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
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

	std::string test_suite  = "Integer logic operator verification\n";
	std::string test_tag    = "logic";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyEqual<4, std::uint8_t>(reportTestCases), "integer<4>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyNotEqual<4, std::uint8_t>(reportTestCases), "integer<4>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLessThan<4, std::uint8_t>(reportTestCases), "integer<4>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLessOrEqualThan<4, std::uint8_t>(reportTestCases), "integer<4>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyGreaterThan<4, std::uint8_t>(reportTestCases), "integer<4>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyGreaterOrEqualThan<4, std::uint8_t>(reportTestCases), "integer<4>", ">=");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyEqual<4, std::uint8_t>(reportTestCases), "integer<4>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyNotEqual<4, std::uint8_t>(reportTestCases), "integer<4>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLessThan<4, std::uint8_t>(reportTestCases), "integer<4>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLessOrEqualThan<4, std::uint8_t>(reportTestCases), "integer<4>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyGreaterThan<4, std::uint8_t>(reportTestCases), "integer<4>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyGreaterOrEqualThan<4, std::uint8_t>(reportTestCases), "integer<4>", ">=");

	nrOfFailedTestCases += ReportTestResult(VerifyEqual<8, std::uint8_t>(reportTestCases), "integer<8>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyNotEqual<8, std::uint8_t>(reportTestCases), "integer<8>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLessThan<8, std::uint8_t>(reportTestCases), "integer<8>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLessOrEqualThan<8, std::uint8_t>(reportTestCases), "integer<8>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyGreaterThan<8, std::uint8_t>(reportTestCases), "integer<8>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyGreaterOrEqualThan<8, std::uint8_t>(reportTestCases), "integer<8>", ">=");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyEqual<10, std::uint8_t>(reportTestCases), "integer<10>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyNotEqual<10, std::uint8_t>(reportTestCases), "integer<10>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLessThan<10, std::uint8_t>(reportTestCases), "integer<10>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLessOrEqualThan<10, std::uint8_t>(reportTestCases), "integer<10>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyGreaterThan<10, std::uint8_t>(reportTestCases), "integer<10>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyGreaterOrEqualThan<10, std::uint8_t>(reportTestCases), "integer<10>", ">=");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyEqual<12, std::uint8_t>(reportTestCases), "integer<12>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyNotEqual<12, std::uint8_t>(reportTestCases), "integer<12>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLessThan<12, std::uint8_t>(reportTestCases), "integer<12>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLessOrEqualThan<12, std::uint8_t>(reportTestCases), "integer<12>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyGreaterThan<12, std::uint8_t>(reportTestCases), "integer<12>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyGreaterOrEqualThan<12, std::uint8_t>(reportTestCases), "integer<12>", ">=");
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyEqual<13, std::uint8_t>(reportTestCases), "integer<13>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyNotEqual<13, std::uint8_t>(reportTestCases), "integer<13>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLessThan<13, std::uint8_t>(reportTestCases), "integer<13>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLessOrEqualThan<13, std::uint8_t>(reportTestCases), "integer<13>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyGreaterThan<13, std::uint8_t>(reportTestCases), "integer<13>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyGreaterOrEqualThan<13, std::uint8_t>(reportTestCases), "integer<13>", ">=");
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
