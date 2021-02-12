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
	int VerifyEqual(const std::string& tag, bool bReportIndividualTestCases) {
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits> ia, ib;
		bool iresult, iref;

		int64_t i64a, i64b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.set_raw_bits(i);
			i64a = int64_t(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.set_raw_bits(j);
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
	int VerifyNotEqual(const std::string& tag, bool bReportIndividualTestCases) {
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits> ia, ib;
		bool iresult, iref;

		int64_t i64a, i64b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.set_raw_bits(i);
			i64a = int64_t(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.set_raw_bits(j);
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
	int VerifyLessThan(const std::string& tag, bool bReportIndividualTestCases) {
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits> ia, ib;
		bool iresult, iref;

		int64_t i64a, i64b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.set_raw_bits(i);
			i64a = int64_t(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.set_raw_bits(j);
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
	int VerifyLessOrEqualThan(const std::string& tag, bool bReportIndividualTestCases) {
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits> ia, ib;
		bool iresult, iref;

		int64_t i64a, i64b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.set_raw_bits(i);
			i64a = int64_t(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.set_raw_bits(j);
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
	int VerifyGreaterThan(const std::string& tag, bool bReportIndividualTestCases) {
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits> ia, ib;
		bool iresult, iref;

		int64_t i64a, i64b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.set_raw_bits(i);
			i64a = int64_t(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.set_raw_bits(j);
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
	int VerifyGreaterOrEqualThan(const std::string& tag, bool bReportIndividualTestCases) {
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits> ia, ib;
		bool iresult, iref;

		int64_t i64a, i64b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.set_raw_bits(i);
			i64a = int64_t(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.set_raw_bits(j);
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

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

std::string convert_to_string(const std::vector<char>& v) {
	std::stringstream ss;
	for (std::vector<char>::const_reverse_iterator rit = v.rbegin(); rit != v.rend(); ++rit) {
		ss << (int)*rit;
	}
	return ss.str();
}

int main()
try {
	using namespace std;
	using namespace sw::universal;

	std::string tag = "Integer Logic Operator tests failed";

#if MANUAL_TESTING

	ReportTestResult(VerifyEqual<4>("manual test", true), "integer<4>", "==");
	ReportTestResult(VerifyNotEqual<4>("manual test", true), "integer<4>", "!=");
	ReportTestResult(VerifyLessThan<4>("manual test", true), "integer<4>", "<");
	ReportTestResult(VerifyLessOrEqualThan<4>("manual test", true), "integer<4>", "<=");
	ReportTestResult(VerifyGreaterThan<4>("manual test", true), "integer<4>", ">");
	ReportTestResult(VerifyGreaterOrEqualThan<4>("manual test", true), "integer<4>", ">=");

	cout << "done" << endl;

	return EXIT_SUCCESS;
#else
	std::cout << "Integer Logic Operator verfication" << std::endl;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	nrOfFailedTestCases += ReportTestResult(VerifyEqual<8>(tag, bReportIndividualTestCases), "integer<8>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyNotEqual<8>(tag, bReportIndividualTestCases), "integer<8>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLessThan<8>(tag, bReportIndividualTestCases), "integer<8>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLessOrEqualThan<8>(tag, bReportIndividualTestCases), "integer<8>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyGreaterThan<8>(tag, bReportIndividualTestCases), "integer<8>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyGreaterOrEqualThan<8>(tag, bReportIndividualTestCases), "integer<8>", ">=");

	nrOfFailedTestCases += ReportTestResult(VerifyEqual<12>(tag, bReportIndividualTestCases), "integer<12>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyNotEqual<12>(tag, bReportIndividualTestCases), "integer<12>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLessThan<12>(tag, bReportIndividualTestCases), "integer<12>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLessOrEqualThan<12>(tag, bReportIndividualTestCases), "integer<12>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyGreaterThan<12>(tag, bReportIndividualTestCases), "integer<12>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyGreaterOrEqualThan<12>(tag, bReportIndividualTestCases), "integer<12>", ">=");

#if STRESS_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifyEqual<16>(tag, bReportIndividualTestCases), "integer<16>", "==");
	nrOfFailedTestCases += ReportTestResult(VerifyNotEqual<16>(tag, bReportIndividualTestCases), "integer<16>", "!=");
	nrOfFailedTestCases += ReportTestResult(VerifyLessThan<16>(tag, bReportIndividualTestCases), "integer<16>", "<");
	nrOfFailedTestCases += ReportTestResult(VerifyLessOrEqualThan<16>(tag, bReportIndividualTestCases), "integer<16>", "<=");
	nrOfFailedTestCases += ReportTestResult(VerifyGreaterThan<16>(tag, bReportIndividualTestCases), "integer<16>", ">");
	nrOfFailedTestCases += ReportTestResult(VerifyGreaterOrEqualThan<16>(tag, bReportIndividualTestCases), "integer<16>", ">=");
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
