//  integers.cpp : test suite for abitrary precision integers
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <universal/integer/integer.hpp>
#include <universal/integer/numeric_limits.hpp>
// test helpers
#include "../test_helpers.hpp"

/*
   The goal of the arbitrary integers is to provide a constrained big integer type
   that enables fast computation with exceptions for overflow, so that the type
   can be used for forward error analysis studies.
*/

namespace sw {
namespace unum {

#define INTEGER_THROW_ARITHMETIC_EXCEPTION 1

#define FLOAT_TABLE_WIDTH 20
	template<size_t nbits>
	void ReportBinaryArithmeticError(std::string test_case, std::string op, const integer<nbits>& lhs, const integer<nbits>& rhs, const integer<nbits>& pref, const integer<nbits>& presult) {
		std::cerr << test_case << " "
			<< std::setprecision(20)
			<< std::setw(FLOAT_TABLE_WIDTH) << lhs
			<< " " << op << " "
			<< std::setw(FLOAT_TABLE_WIDTH) << rhs
			<< " != "
			<< std::setw(FLOAT_TABLE_WIDTH) << pref << " instead it yielded "
			<< std::setw(FLOAT_TABLE_WIDTH) << presult
			<< " " << to_binary(pref) << " vs " << to_binary(presult)
			<< std::setprecision(5)
			<< std::endl;
	}

	// enumerate all addition cases for an integer<16> configuration
	template<size_t nbits>
	int VerifyShortAddition(std::string tag, bool bReportIndividualTestCases) {
		static_assert(nbits == 16, "nbits needs to be 16");

		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits> ia, ib, iresult, iref;

		short i64a, i64b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.set_raw_bits(i);
			i64a = short(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.set_raw_bits(j);
				i64b = short(ib);
				iref = i64a + i64b;
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
				try {
					iresult = ia + ib;
				}
				catch (...) {
					if (iref > max_int<nbits>() || iref < min_int<nbits>()) {
						// correctly caught the exception

					}
					else {
						nrOfFailedTests++;
					}
				}

#else
				iresult = ia + ib;
#endif
				if (iresult != iref) {
					nrOfFailedTests++;
					if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "+", ia, ib, iref, iresult);
				}
				else {
					//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "+", ia, ib, iref, iresult);
				}
			}
			if (i % 1024 == 0) std::cout << '.';
		}
		std::cout << std::endl;

		return nrOfFailedTests;
	}
	// enumerate all division cases for an integer<16> configuration
	template<size_t nbits>
	int VerifyShortDivision(std::string tag, bool bReportIndividualTestCases) {
		static_assert(nbits == 16, "nbits needs to be 16");

		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits> ia, ib, iresult, iref;

		short i64a, i64b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.set_raw_bits(i);
			i64a = short(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.set_raw_bits(j);
				i64b = short(ib);
				iref = i64a / i64b;
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
				try {
					iresult = ia / ib;
				}
				catch (...) {
					if (iref > max_int<nbits>() || iref < min_int<nbits>()) {
						// correctly caught the exception

					}
					else {
						nrOfFailedTests++;
					}
				}

#else
				iresult = ia / ib;
#endif
				if (iresult != iref) {
					nrOfFailedTests++;
					if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "+", ia, ib, iref, iresult);
				}
				else {
					//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "+", ia, ib, iref, iresult);
				}
			}
			if (i % 1024 == 0) std::cout << '.';
		}
		std::cout << std::endl;

		return nrOfFailedTests;
	}


	// enumerate all addition cases for an integer<nbits> configuration
	template<size_t nbits>
	int VerifyAddition(std::string tag, bool bReportIndividualTestCases) {
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits> ia, ib, iresult, iref;

		int64_t i64a, i64b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.set_raw_bits(i);
			i64a = int64_t(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.set_raw_bits(j);
				i64b = int64_t(ib);
				iref = i64a + i64b;
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
				try {
					iresult = ia + ib;
				}
				catch (...) {
					if (iref > max_int<nbits>() || iref < min_int<nbits>()) {
						// correctly caught the exception
	
					}
					else {
						nrOfFailedTests++;
					}
				}

#else
				iresult = ia + ib;
#endif
				if (iresult != iref) {
					nrOfFailedTests++;
					if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "+", ia, ib, iref, iresult);
				}
				else {
					//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "+", ia, ib, iref, iresult);
				}
				if (nrOfFailedTests > 100) return nrOfFailedTests;
			}
			if (i % 1024 == 0) std::cout << '.';
		}
		std::cout << std::endl;
		return nrOfFailedTests;
	}

	template<size_t nbits>
	int VerifySubtraction(std::string tag, bool bReportIndividualTestCases) {
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits> ia, ib, iresult, iref;

		int64_t i64a, i64b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.set_raw_bits(i);
			i64a = int64_t(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.set_raw_bits(j);
				i64b = int64_t(ib);
				iref = i64a - i64b;
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
				try {
					iresult = ia - ib;
				}
				catch (...) {
					if (iref > max_int<nbits>() || iref < min_int<nbits>()) {
						// correctly caught the exception

					}
					else {
						nrOfFailedTests++;
					}
				}

#else
				iresult = ia - ib;
#endif
				if (iresult != iref) {
					nrOfFailedTests++;
					if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "+", ia, ib, iref, iresult);
				}
				else {
					//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "+", ia, ib, iref, iresult);
				}
				if (nrOfFailedTests > 100) return nrOfFailedTests;
			}
			if (i % 1024 == 0) std::cout << '.';
		}
		std::cout << std::endl;
		return nrOfFailedTests;
	}

	template<size_t nbits>
	int VerifyMultiplication(std::string tag, bool bReportIndividualTestCases) {
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits> ia, ib, iresult, iref;

		int64_t i64a, i64b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.set_raw_bits(i);
			i64a = int64_t(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.set_raw_bits(j);
				i64b = int64_t(ib);
				iref = i64a * i64b;
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
				try {
					iresult = ia * ib;
				}
				catch (...) {
					if (iref > max_int<nbits>() || iref < min_int<nbits>()) {
						// correctly caught the exception

					}
					else {
						nrOfFailedTests++;
					}
				}

#else
				iresult = ia * ib;
#endif
				if (iresult != iref) {
					nrOfFailedTests++;
					if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "+", ia, ib, iref, iresult);
				}
				else {
					//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "+", ia, ib, iref, iresult);
				}
				if (nrOfFailedTests > 100) return nrOfFailedTests;
			}
			if (i % 1024 == 0) std::cout << '.';
		}
		std::cout << std::endl;
		return nrOfFailedTests;
	}

	template<size_t nbits>
	int VerifyDivision(std::string tag, bool bReportIndividualTestCases) {
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits> ia, ib, iresult, iref;

		int64_t i64a, i64b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.set_raw_bits(i);
			i64a = int64_t(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.set_raw_bits(j);
				i64b = int64_t(ib);
				iref = i64a / i64b;
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
				try {
					iresult = ia / ib;
				}
				catch (...) {
					if (iref > max_int<nbits>() || iref < min_int<nbits>()) {
						// correctly caught the exception

					}
					else {
						nrOfFailedTests++;
					}
				}

#else
				iresult = ia / ib;
#endif
				if (iresult != iref) {
					nrOfFailedTests++;
					if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "+", ia, ib, iref, iresult);
				}
				else {
					//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "+", ia, ib, iref, iresult);
				}
				if (nrOfFailedTests > 100) return nrOfFailedTests;
			}
			if (i % 1024 == 0) std::cout << '.';
		}
		std::cout << std::endl;
		return nrOfFailedTests;
	}
}
}

#include <typeinfo>
template<typename Scalar>
void GenerateAddTest(const Scalar& x, const Scalar& y, Scalar& z) {
	using namespace sw::unum;
	z = x + y;
	std::cout << typeid(Scalar).name() << ": " << x << " + " << y << " = " << z << std::endl;
}
template<typename Scalar>
void GenerateMulTest(const Scalar& x, const Scalar& y, Scalar& z) {
	using namespace sw::unum;
	z = x * y;
	std::cout << typeid(Scalar).name() << ": " << x << " * " << y << " = " << z << std::endl;
}

#define MANUAL_TESTING 1
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
	using namespace sw::unum;

	std::string tag = "Integer Arithmetic tests failed";

#if MANUAL_TESTING
	using int8 = integer<8>;
	using int64 = integer<64>;
	using int128 = integer<128>;

	int8 a;
	int64 k;
	int128 m;
	cout << "Nr of bytes\n";
	cout << typeid(a).name() << "  size in bytes " << a.nrBytes << endl;
	cout << typeid(k).name() << "  size in bytes " << k.nrBytes << endl;
	cout << typeid(m).name() << "  size in bytes " << m.nrBytes << endl;

	integer<128> i1, i2, i3;

	cout << "conversion" << endl;

	i1 = 123456789;
	cout << i1 << endl;
	//i2 = 1.23456789e8;
	//i3.parse("123456789");

	short s = 0;
	GenerateMulTest<short>(2, 16, s);
	integer<16> z = 0;
	GenerateMulTest<integer<16> >(2, 16, z);

	{
		integer<16> a = 0x0AA1;
		a <<= 1;
		cout << to_binary(a) << endl;
		a <<= 2;
		cout << to_binary(a) << endl;
	}

	ReportTestResult(VerifyDivision<4>("manual test", true), "integer<4>", "divides");

	cout << "done" << endl;

	return EXIT_SUCCESS;
#else
	std::cout << "Integer Arithmetic verfication" << std::endl;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	// allocation is the only functionality of integer<N> at this time

	// TODO: implement parsing, assigment, conversion, arithmetic
	std::string type = "integer<4>";
#define NBITS 4
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<NBITS>(tag, bReportIndividualTestCases), type, "addition");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<NBITS>(tag, bReportIndividualTestCases), type, "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<NBITS>(tag, bReportIndividualTestCases), type, "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<NBITS>(tag, bReportIndividualTestCases), type, "division");
#undef NBITS

	type = "integer<8>";
#define NBITS 8
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<NBITS>(tag, bReportIndividualTestCases), type, "addition");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<NBITS>(tag, bReportIndividualTestCases), type, "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<NBITS>(tag, bReportIndividualTestCases), type, "multiplication");
#undef NBITS

	type = "integer<12>";
#define NBITS 12
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<NBITS>(tag, bReportIndividualTestCases), type, "addition");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<NBITS>(tag, bReportIndividualTestCases), type, "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<NBITS>(tag, bReportIndividualTestCases), type, "multiplication");
#undef NBITS

#if STRESS_TESTING
	// VerifyShortAddition compares an integer<16> to native short type to make certain it has all the same behavior
	nrOfFailedTestCases += ReportTestResult(VerifyShortAddition<16>(tag, bReportIndividualTestCases), "integer<16>", "addition");
	// this is a 'standard' comparision against a native int64_t
	type = "integer<16>";
#define NBITS 16
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<NBITS>(tag, bReportIndividualTestCases), type, "addition");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<NBITS>(tag, bReportIndividualTestCases), type, "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<NBITS>(tag, bReportIndividualTestCases), type, "multiplication");
#undef NBITS

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
