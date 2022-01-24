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

namespace sw { namespace universal {

	// enumerate all addition cases for an integer<16> configuration compared against native short
	template<typename BlockType, size_t testBits = 12>
	int VerifyShortAddition(bool bReportIndividualTestCases) {
		constexpr size_t nbits = 16; 
		constexpr size_t NR_INTEGERS = (size_t(1) << testBits);

		using Integer = integer<nbits, BlockType>;
		Integer ia, ib, iresult, iref;

		int nrOfFailedTests = 0;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.setbits(i);
			short i64a = short(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.setbits(j);
				short i64b = short(ib);
				iref = i64a + i64b;
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
				try {
					iresult = ia + ib;
				}
				catch (...) {
					Integer max_int(SpecificValue::maxpos), min_int(SpecificValue::maxneg);
					if (iref > max_int || iref < min_int) {
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
	template<typename BlockType, size_t testBits = 12>
	int VerifyShortSubtraction(bool bReportIndividualTestCases) {
		constexpr size_t nbits = 16;
		constexpr size_t NR_INTEGERS = (size_t(1) << testBits);

		using Integer = integer<nbits, BlockType>;
		Integer ia, ib, iresult, iref;

		int nrOfFailedTests = 0;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.setbits(i);
			short i16a = short(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.setbits(j);
				short i16b = short(ib);
				iref = i16a - i16b;
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
				try {
					iresult = ia - ib;
				}
				catch (...) {
					Integer max_int(SpecificValue::maxpos), min_int(SpecificValue::maxneg);
					if (iref > max_int || iref < min_int) {
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
	template<typename BlockType, size_t testBits = 12>
	int VerifyShortMultiplication(bool bReportIndividualTestCases) {
		constexpr size_t nbits = 16;
		constexpr size_t NR_INTEGERS = (size_t(1) << testBits);

		using Integer = integer<nbits, BlockType>;
		Integer ia, ib, iresult, iref;

		int nrOfFailedTests = 0;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.setbits(i);
			short i16a = short(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.setbits(j);
				short i16b = short(ib);
				iref = i16a * i16b;
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
				try {
					iresult = ia * ib;
				}
				catch (...) {
					Integer max_int(SpecificValue::maxpos), min_int(SpecificValue::maxneg);
					if (iref > max_int || iref < min_int) {
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
	template<typename BlockType, size_t testBits = 10>
	int VerifyShortDivision(bool bReportIndividualTestCases) {
		constexpr size_t nbits = 16;
		constexpr size_t NR_INTEGERS = (size_t(1) << testBits);

		using Integer = integer<nbits, BlockType>;
		Integer ia, ib, iresult, iref;

		int nrOfFailedTests = 0;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.setbits(i);
			short i16a = short(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.setbits(j);
				short i16b = short(ib);
				if (j > 0) iref = i16a / i16b;
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
				Integer max_int(SpecificValue::maxpos), min_int(SpecificValue::maxneg);
				if (iref > max_int || iref < min_int) {
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
	template<typename BlockType, size_t testBits = 10>
	int VerifyShortRemainder(bool bReportIndividualTestCases) {
		constexpr size_t nbits = 16;
		constexpr size_t NR_INTEGERS = (size_t(1) << testBits);

		using Integer = integer<nbits, BlockType>;
		Integer ia, ib, iresult, iref;

		int nrOfFailedTests = 0;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.setbits(i);
			short i16a = short(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.setbits(j);
				short i16b = short(ib);
				if (j > 0) iref = i16a % i16b;
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
				try {
					iresult = ia % ib;
				}
				catch (...) {
					Integer max_int(SpecificValue::maxpos), min_int(SpecificValue::maxneg);
					if (iref > max_int || iref < min_int) {
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
		using Integer = integer<nbits, BlockType>;
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);

		Integer ia, ib, iresult, iref;

		int nrOfFailedTests = 0;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.setbits(i);
			int64_t i64a = int64_t(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.setbits(j);
				int64_t i64b = int64_t(ib);
				iref = i64a + i64b;
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
				try {
					iresult = ia + ib;
				}
				catch (...) {
					Integer max_int(SpecificValue::maxpos), min_int(SpecificValue::maxneg);
					if (iref > max_int || iref < min_int) {
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
		using Integer = integer<nbits, BlockType>;
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);

		Integer ia, ib, iresult, iref;

		int nrOfFailedTests = 0;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.setbits(i);
			int64_t i64a = int64_t(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.setbits(j);
				int64_t i64b = int64_t(ib);
				iref = i64a - i64b;
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
				try {
					iresult = ia - ib;
				}
				catch (...) {
					Integer max_int(SpecificValue::maxpos), min_int(SpecificValue::maxneg);
					if (iref > max_int || iref < min_int) {
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
		using Integer = integer<nbits, BlockType>;
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);

		Integer ia, ib, iresult, iref;

		int nrOfFailedTests = 0;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.setbits(i);
			int64_t i64a = int64_t(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.setbits(j);
				int64_t i64b = int64_t(ib);
				iref = i64a * i64b;
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
				try {
					iresult = ia * ib;
				}
				catch (...) {
					Integer max_int(SpecificValue::maxpos), min_int(SpecificValue::maxneg);
					if (iref > max_int || iref < min_int) {
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
		using Integer = integer<nbits, BlockType>;
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);

		Integer ia, ib, iresult, iref;

		int nrOfFailedTests = 0;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.setbits(i);
			int64_t i64a = int64_t(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.setbits(j);
				int64_t i64b = int64_t(ib);
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
		using Integer = integer<nbits, BlockType>;
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);

		Integer ia, ib, iresult, iref;

		int nrOfFailedTests = 0;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.setbits(i);
			int64_t i64a = int64_t(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.setbits(j);
				int64_t i64b = int64_t(ib);
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

}} // namespace sw::universal
