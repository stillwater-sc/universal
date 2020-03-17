//  integer_test_helpers.hpp : arithmetic test suite for abitrary precision integers
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
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

namespace sw {
namespace unum {


#define INTEGER_TABLE_WIDTH 20
	template<size_t nbits, typename BlockType>
	void ReportBinaryArithmeticError(std::string test_case, std::string op, const integer<nbits, BlockType>& lhs, const integer<nbits, BlockType>& rhs, const integer<nbits, BlockType>& pref, const integer<nbits, BlockType>& presult) {
		auto old_precision = std::cerr.precision(); 
		std::cerr << test_case << " "
			<< std::setprecision(20)
			<< std::setw(INTEGER_TABLE_WIDTH) << lhs
			<< " " << op << " "
			<< std::setw(INTEGER_TABLE_WIDTH) << rhs
			<< " != "
			<< std::setw(INTEGER_TABLE_WIDTH) << pref << " instead it yielded "
			<< std::setw(INTEGER_TABLE_WIDTH) << presult
			<< " " << to_binary(pref) << " vs " << to_binary(presult)
			<< std::setprecision(old_precision)
			<< std::endl;
	}

	template<size_t nbits, typename BlockType>
	void ReportBinaryArithmeticSuccess(std::string test_case, std::string op, const integer<nbits, BlockType>& lhs, const integer<nbits, BlockType>& rhs, const integer<nbits, BlockType>& pref, const integer<nbits, BlockType>& presult) {
		auto old_precision = std::cerr.precision();
		std::cerr << test_case << " "
			<< std::setprecision(20)
			<< std::setw(INTEGER_TABLE_WIDTH) << lhs
			<< " " << op << " "
			<< std::setw(INTEGER_TABLE_WIDTH) << rhs
			<< " == "
			<< std::setw(INTEGER_TABLE_WIDTH) << pref << " matches reference "
			<< std::setw(INTEGER_TABLE_WIDTH) << presult
			<< " " << to_binary(pref) << " vs " << to_binary(presult)
			<< std::setprecision(old_precision)
			<< std::endl;
	}

	template<size_t nbits, typename BlockType>
	void ReportUnaryArithmeticError(std::string test_case, std::string op, const integer<nbits, BlockType>& argument, const integer<nbits, BlockType>& ref, const integer<nbits, BlockType>& result) {
		auto old_precision = std::cerr.precision();
		std::cerr << test_case << " "
			<< " " << op << " "
			<< std::setprecision(20)
			<< std::setw(INTEGER_TABLE_WIDTH) << argument
			<< " != "
			<< std::setw(INTEGER_TABLE_WIDTH) << ref << " instead it yielded "
			<< std::setw(INTEGER_TABLE_WIDTH) << result
			<< " " << to_binary(ref) << " vs " << to_binary(result)
			<< std::setprecision(old_precision)
			<< std::endl;
	}

	// enumerate all addition cases for an integer<16> configuration compared against native short
	template<typename BlockType>
	int VerifyShortAddition(std::string tag, bool bReportIndividualTestCases) {
		constexpr size_t nbits = 16;

		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits, BlockType> ia, ib, iresult, iref;

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
	int VerifyShortSubtraction(std::string tag, bool bReportIndividualTestCases) {
		constexpr size_t nbits = 16;

		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits, BlockType> ia, ib, iresult, iref;

		short i16a, i16b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.set_raw_bits(i);
			i16a = short(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.set_raw_bits(j);
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
	int VerifyShortMultiplication(std::string tag, bool bReportIndividualTestCases) {
		constexpr size_t nbits = 16;

		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits, BlockType> ia, ib, iresult, iref;

		short i16a, i16b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.set_raw_bits(i);
			i16a = short(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.set_raw_bits(j);
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
	int VerifyShortDivision(std::string tag, bool bReportIndividualTestCases) {
		constexpr size_t nbits = 16;

		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits, BlockType> ia, ib, iresult, iref;

		short i16a, i16b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.set_raw_bits(i);
			i16a = short(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.set_raw_bits(j);
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
	int VerifyShortRemainder(std::string tag, bool bReportIndividualTestCases) {
		constexpr size_t nbits = 16;

		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits, BlockType> ia, ib, iresult, iref;

		short i16a, i16b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.set_raw_bits(i);
			i16a = short(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.set_raw_bits(j);
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
	int VerifyAddition(std::string tag, bool bReportIndividualTestCases) {
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits, BlockType> ia, ib, iresult, iref;

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
	int VerifySubtraction(std::string tag, bool bReportIndividualTestCases) {
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits, BlockType> ia, ib, iresult, iref;

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
	int VerifyMultiplication(std::string tag, bool bReportIndividualTestCases) {
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits, BlockType> ia, ib, iresult, iref;

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
	int VerifyDivision(std::string tag, bool bReportIndividualTestCases) {
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits, BlockType> ia, ib, iresult, iref;

		int64_t i64a, i64b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.set_raw_bits(i);
			i64a = int64_t(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.set_raw_bits(j);
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
	int VerifyRemainder(std::string tag, bool bReportIndividualTestCases) {
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		integer<nbits, BlockType> ia, ib, iresult, iref;

		int64_t i64a, i64b;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.set_raw_bits(i);
			i64a = int64_t(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.set_raw_bits(j);
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

} // namespace unum
} // namespace sw
