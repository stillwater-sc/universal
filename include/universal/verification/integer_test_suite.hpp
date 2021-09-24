// integer_test_suite.hpp : arithmetic test suite for abitrary precision integers
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>

// the integer number class will be configured outside of this helper
//
/*
   The goal of the arbitrary integers is to provide a constrained big integer type
   that enables fast computation with exceptions for overflow, so that the type
   can be used for forward error analysis studies.
*/
#include <universal/verification/test_status.hpp> // ReportTestResult used by test suite runner
#include <universal/verification/test_reporters.hpp> 

namespace sw::universal {

	// enumerate all addition cases for an integer<16> configuration compared against native short
	template<typename BlockType>
	int VerifyShortAddition(bool bReportIndividualTestCases) {
		constexpr size_t nbits = 16;

		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits, BlockType> ia, ib, iresult, iref;

		short i64a, i64b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.setbits(i);
			i64a = short(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.setbits(j);
				i64b = short(ib);
				iref = i64a + i64b;
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
				try {
					iresult = ia + ib;
				}
				catch (...) {
					if (iref > max_int<nbits, BlockType>() || iref < min_int<nbits, BlockType>()) {
						// correctly caught the exception
						continue;
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
	// enumerate all subtraction cases for an integer<16> configuration compared against native short
	template<typename BlockType>
	int VerifyShortSubtraction(bool bReportIndividualTestCases) {
		constexpr size_t nbits = 16;

		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits, BlockType> ia, ib, iresult, iref;

		short i16a, i16b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.setbits(i);
			i16a = short(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.setbits(j);
				i16b = short(ib);
				iref = i16a - i16b;
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
				try {
					iresult = ia - ib;
				}
				catch (...) {
					if (iref > max_int<nbits, BlockType>() || iref < min_int<nbits, BlockType>()) {
						// correctly caught the exception
						continue;
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
					if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "-", ia, ib, iref, iresult);
				}
				else {
					//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "-", ia, ib, iref, iresult);
				}
			}
			if (i % 1024 == 0) std::cout << '.';
		}
		std::cout << std::endl;

		return nrOfFailedTests;
	}
	// enumerate all multiplication cases for an integer<16> configuration compared against native short
	template<typename BlockType>
	int VerifyShortMultiplication(bool bReportIndividualTestCases) {
		constexpr size_t nbits = 16;

		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits, BlockType> ia, ib, iresult, iref;

		short i16a, i16b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.setbits(i);
			i16a = short(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.setbits(j);
				i16b = short(ib);
				iref = i16a * i16b;
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
				try {
					iresult = ia * ib;
				}
				catch (...) {
					if (iref > max_int<nbits, BlockType>() || iref < min_int<nbits, BlockType>()) {
						// correctly caught the exception
						continue;
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
					if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "*", ia, ib, iref, iresult);
				}
				else {
					//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "*", ia, ib, iref, iresult);
				}
			}
			if (i % 1024 == 0) std::cout << '.';
		}
		std::cout << std::endl;

		return nrOfFailedTests;
	}
	// enumerate all division cases for an integer<16> configuration compared against native short
	template<typename BlockType>
	int VerifyShortDivision(bool bReportIndividualTestCases) {
		constexpr size_t nbits = 16;

		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits, BlockType> ia, ib, iresult, iref;

		short i16a, i16b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.setbits(i);
			i16a = short(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.setbits(j);
				i16b = short(ib);
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
				if (j == 0) {
					try {
						iresult = ia / ib;
					}
					catch (const integer_divide_by_zero& err) {
						// correctly caught overflow
						continue;
					}
					catch (...) {
						nrOfFailedTests++;
					}
				}
				iref = i16a / i16b; // protected by the continue above
				if (iref > max_int<nbits, BlockType>() || iref < min_int<nbits, BlockType>()) {
					try {
						iresult = ia / ib;
					}
					catch (const integer_overflow& err) {
						// correctly caught overflow
						continue;
					}
					catch (...) {
						nrOfFailedTests++;
					}
				}
#else
				if (j == 0) continue;
				iresult = ia / ib;
#endif

				if (iresult != iref) {
					nrOfFailedTests++;
					if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "/", ia, ib, iref, iresult);
				}
				else {
					//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "/", ia, ib, iref, iresult);
				}
			}
			if (i % 1024 == 0) std::cout << '.';
		}
		std::cout << std::endl;

		return nrOfFailedTests;
	}
	// enumerate all remainder cases for an integer<16> configuration compared against native short
	template<typename BlockType = uint8_t>
	int VerifyShortRemainder(bool bReportIndividualTestCases) {
		constexpr size_t nbits = 16;

		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits, BlockType> ia, ib, iresult, iref;

		short i16a, i16b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.setbits(i);
			i16a = short(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.setbits(j);
				i16b = short(ib);
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
				try {
					iresult = ia % ib;
				}
				catch (...) {
					if (iref > max_int<nbits, BlockType>() || iref < min_int<nbits, BlockType>()) {
						// correctly caught the exception
						continue;
					}
					else {
						nrOfFailedTests++;
					}
				}
#else
				iresult = ia % ib;
#endif
				iref = i16a % i16b;
				if (iresult != iref) {
					nrOfFailedTests++;
					if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "%", ia, ib, iref, iresult);
				}
				else {
					//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "%", ia, ib, iref, iresult);
				}
			}
			if (i % 1024 == 0) std::cout << '.';
		}
		std::cout << std::endl;

		return nrOfFailedTests;
	}

	// enumerate all addition cases for an integer<nbits, BlockType> configuration
	template<size_t nbits, typename BlockType>
	int VerifyAddition(bool bReportIndividualTestCases) {
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits, BlockType> ia, ib, iresult, iref;

		int64_t i64a, i64b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.setbits(i);
			i64a = int64_t(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.setbits(j);
				i64b = int64_t(ib);
				iref = i64a + i64b;
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
				try {
					iresult = ia + ib;
				}
				catch (...) {
					if (iref > max_int<nbits, BlockType>() || iref < min_int<nbits, BlockType>()) {
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
	// enumerate all subtraction cases for an integer<nbits, BlockType> configuration
	template<size_t nbits, typename BlockType>
	int VerifySubtraction(bool bReportIndividualTestCases) {
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits, BlockType> ia, ib, iresult, iref;

		int64_t i64a, i64b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.setbits(i);
			i64a = int64_t(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.setbits(j);
				i64b = int64_t(ib);
				iref = i64a - i64b;
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
				try {
					iresult = ia - ib;
				}
				catch (...) {
					if (iref > max_int<nbits, BlockType>() || iref < min_int<nbits, BlockType>()) {
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
					if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "-", ia, ib, iref, iresult);
				}
				else {
					//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "-", ia, ib, iref, iresult);
				}
				if (nrOfFailedTests > 100) return nrOfFailedTests;
			}
			if (i % 1024 == 0) std::cout << '.';
		}
		std::cout << std::endl;
		return nrOfFailedTests;
	}

	// enumerate all multiplication cases for an integer<nbits, BlockType> configuration
	template<size_t nbits, typename BlockType>
	int VerifyMultiplication(bool bReportIndividualTestCases) {
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits, BlockType> ia, ib, iresult, iref;

		int64_t i64a, i64b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.setbits(i);
			i64a = int64_t(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.setbits(j);
				i64b = int64_t(ib);
				iref = i64a * i64b;
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
				try {
					iresult = ia * ib;
				}
				catch (...) {
					if (iref > max_int<nbits, BlockType>() || iref < min_int<nbits, BlockType>()) {
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
					if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "*", ia, ib, iref, iresult);
				}
				else {
					if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "*", ia, ib, iref, iresult);
				}
				if (nrOfFailedTests > 100) return nrOfFailedTests;
			}
			if (i % 1024 == 0) std::cout << '.';
		}
		std::cout << std::endl;
		return nrOfFailedTests;
	}

	// enumerate all division cases for an integer<nbits, BlockType> configuration
	template<size_t nbits, typename BlockType>
	int VerifyDivision(bool bReportIndividualTestCases) {
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits, BlockType> ia, ib, iresult, iref;

		int64_t i64a, i64b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.setbits(i);
			i64a = int64_t(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.setbits(j);
				i64b = int64_t(ib);
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
				try {
					iresult = ia / ib;
				}
				catch (const integer_divide_by_zero& e) {
					if (ib == integer<nbits, BlockType>(0)) {
						// correctly caught the exception
						continue;
					}
					else {
						std::cerr << "unexpected : " << e.what() << std::endl;
						nrOfFailedTests++;
					}
				}
				catch (const integer_overflow& e) {
					std::cerr << e.what() << std::endl;
					// TODO: how do you validate the overflow?
				}
				catch (...) {
					std::cerr << "unexpected exception" << std::endl;
					nrOfFailedTests++;
				}
#else
				iresult = ia / ib;
#endif
				if (j == 0) {
					iref = 0; // or maxneg?
				}
				else {
					iref = i64a / i64b;
				}
				if (iresult != iref) {
					nrOfFailedTests++;
					if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "/", ia, ib, iref, iresult);
				}
				else {
					//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "/", ia, ib, iref, iresult);
				}
				if (nrOfFailedTests > 100) return nrOfFailedTests;
			}
			if (i % 1024 == 0) std::cout << '.';
		}
		std::cout << std::endl;
		return nrOfFailedTests;
	}

	// enumerate all remainder cases for an integer<nbits, BlockType> configuration
	template<size_t nbits, typename BlockType>
	int VerifyRemainder(bool bReportIndividualTestCases) {
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits, BlockType> ia, ib, iresult, iref;

		int64_t i64a, i64b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.setbits(i);
			i64a = int64_t(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.setbits(j);
				i64b = int64_t(ib);
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
				try {
					iresult = ia % ib;
				}
				catch (...) {
					if (ib == integer<nbits, BlockType>(0)) {
						// correctly caught the exception
						continue;
					}
					else {
						nrOfFailedTests++;
					}
				}

#else
				iresult = ia % ib;
#endif
				iref = i64a % i64b;
				if (iresult != iref) {
					nrOfFailedTests++;
					if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "%", ia, ib, iref, iresult);
				}
				else {
					//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "%", ia, ib, iref, iresult);
				}
				if (nrOfFailedTests > 100) return nrOfFailedTests;
			}
			if (i % 1024 == 0) std::cout << '.';
		}
		std::cout << std::endl;
		return nrOfFailedTests;
	}

} // namespace sw::universal
