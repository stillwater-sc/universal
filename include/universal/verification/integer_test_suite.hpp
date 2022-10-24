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
	template<typename BlockType>
	int VerifyShortAddition(bool reportTestCases) {
		constexpr unsigned nbits = 16; 
		constexpr unsigned NR_INTEGERS = (size_t(1) << nbits);

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
					if (reportTestCases)	ReportBinaryArithmeticError("FAIL", "+", ia, ib, iref, iresult);
				}
				else {
					//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "+", ia, ib, iref, iresult);
				}
			}
			if (i % 1024 == 0) std::cout << '.';
		}
		std::cout << std::endl;

		return nrOfFailedTests;
	}
	// enumerate all subtraction cases for an integer<16> configuration compared against native short
	template<typename BlockType>
	int VerifyShortSubtraction(bool reportTestCases) {
		constexpr unsigned nbits = 16;
		constexpr size_t NR_INTEGERS = (size_t(1) << nbits);

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
					if (reportTestCases)	ReportBinaryArithmeticError("FAIL", "-", ia, ib, iref, iresult);
				}
				else {
					//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "-", ia, ib, iref, iresult);
				}
			}
			if (i % 1024 == 0) std::cout << '.';
		}
		std::cout << std::endl;

		return nrOfFailedTests;
	}
	// enumerate all multiplication cases for an integer<16> configuration compared against native short
	template<typename BlockType, unsigned testBits = 12>
	int VerifyShortMultiplication(bool reportTestCases) {
		constexpr unsigned nbits = 16;
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
					if (reportTestCases) ReportBinaryArithmeticError("FAIL", "*", ia, ib, iref, iresult);
				}
				else {
					//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "*", ia, ib, iref, iresult);
				}
			}
			if (reportTestCases) if (i % 1024 == 0) std::cout << '.';
		}
		if (reportTestCases) std::cout << std::endl;

		return nrOfFailedTests;
	}
	// enumerate all division cases for an integer<16> configuration compared against native short
	template<typename BlockType, unsigned testBits = 10>
	int VerifyShortDivision(bool reportTestCases) {
		constexpr unsigned nbits = 16;
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
						++nrOfFailedTests;
					}
				}
				if (j > 0) iref = i16a / i16b;
				Integer max_int(SpecificValue::maxpos), min_int(SpecificValue::maxneg);
				if (iref < min_int || iref > max_int) {
					try {
						iresult = ia / ib;
					}
					catch (const integer_overflow& err) {
						// correctly caught overflow
						continue;
					}
					catch (...) {
						++nrOfFailedTests;
					}
				}
				else {
					iresult = ia / ib;
				}
#else
				if (j == 0) continue;
				iresult = ia / ib;
				iref = i16a / i16b;
#endif

				if (iresult != iref) {
					nrOfFailedTests++;
					if (reportTestCases)	ReportBinaryArithmeticError("FAIL", "/", ia, ib, iref, iresult);
				}
				else {
					//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "/", ia, ib, iref, iresult);
				}
			}
			if (reportTestCases) if (i % 1024 == 0) std::cout << '.';
		}
		if (reportTestCases) std::cout << std::endl;

		return nrOfFailedTests;
	}
	// enumerate all remainder cases for an integer<16> configuration compared against native short
	template<typename BlockType, unsigned testBits = 10>
	int VerifyShortRemainder(bool reportTestCases) {
		constexpr unsigned nbits = 16;
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
					if (reportTestCases)	ReportBinaryArithmeticError("FAIL", "%", ia, ib, iref, iresult);
				}
				else {
					//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "%", ia, ib, iref, iresult);
				}
			}
			if (i % 1024 == 0) std::cout << '.';
		}
		std::cout << std::endl;

		return nrOfFailedTests;
	}

	// enumerate all addition cases for an integer<nbits, BlockType> configuration
	template<unsigned nbits, typename BlockType>
	int VerifyAddition(bool reportTestCases) {
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
					if (reportTestCases) ReportBinaryArithmeticError("FAIL", "+", ia, ib, iref, iresult);
				}
				else {
					//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "+", ia, ib, iref, iresult);
				}
				if (nrOfFailedTests > 100) return nrOfFailedTests;
			}
			if (reportTestCases) if (i % 1024 == 0) std::cout << '.';
		}
		if (reportTestCases) std::cout << std::endl;
		return nrOfFailedTests;
	}
	// enumerate all subtraction cases for an integer<nbits, BlockType> configuration
	template<unsigned nbits, typename BlockType>
	int VerifySubtraction(bool reportTestCases) {
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
					if (reportTestCases) ReportBinaryArithmeticError("FAIL", "-", ia, ib, iref, iresult);
				}
				else {
					//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "-", ia, ib, iref, iresult);
				}
				if (nrOfFailedTests > 100) return nrOfFailedTests;
			}
			if (reportTestCases) if (i % 1024 == 0) std::cout << '.';
		}
		if (reportTestCases) std::cout << std::endl;
		return nrOfFailedTests;
	}

	// enumerate all multiplication cases for an integer<nbits, BlockType> configuration
	template<unsigned nbits, typename BlockType>
	int VerifyMultiplication(bool reportTestCases) {
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
					if (reportTestCases) ReportBinaryArithmeticError("FAIL", "*", ia, ib, iref, iresult);
				}
				else {
					// if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "*", ia, ib, iref, iresult);
				}
				if (nrOfFailedTests > 100) return nrOfFailedTests;
			}
			if (reportTestCases) if (i % 1024 == 0) std::cout << '.';
		}
		if (reportTestCases) std::cout << std::endl;
		return nrOfFailedTests;
	}

	// default is an unsigned reference type
	template<IntegerNumberType NumberType>
	struct ReferenceTypeForInteger {
		typedef std::uint64_t reference_type;
	};
	// specialized for IntegerNumber to yield a signed reference type
	template<>
	struct ReferenceTypeForInteger< IntegerNumberType::IntegerNumber>{
		typedef std::int64_t reference_type;
	};

	// enumerate all division cases for an integer<nbits, BlockType, NumberType> configuration
	template<unsigned nbits, typename BlockType, IntegerNumberType NumberType>
	int VerifyDivision(bool reportTestCases) {
		using Integer = integer<nbits, BlockType>;
		using ReferenceType = typename ReferenceTypeForInteger<NumberType>::reference_type;

		constexpr size_t NR_INTEGERS = (1ull << nbits);

		Integer ia, ib, ic, iref;
		ReferenceType ra, rb, rc;  // reference values

		int nrOfFailedTests = 0;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.setbits(i);
			ra = ReferenceType(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.setbits(j);
				rb = ReferenceType(ib);
#if INTEGER_THROW_ARITHMETIC_EXCEPTION
				try {
					ic = ia / ib;
				}
				catch (const integer_divide_by_zero& e) {
					if (ib.iszero()) {
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
				ic = ia / ib;
#endif
				if (j == 0) {
					iref = 0; // or maxneg?
				}
				else {
					rc = ra / rb;
					iref = rc;
				}
				if (ic != iref) {
					nrOfFailedTests++;
					if (reportTestCases) ReportBinaryArithmeticError("FAIL", "/", ia, ib, iref, ic);
				}
				else {
					//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "/", ia, ib, iref, ic);
				}
				if (nrOfFailedTests > 100) return nrOfFailedTests;
			}
			if (reportTestCases) if (i % 1024 == 0) std::cout << '.';
		}
		if (reportTestCases) std::cout << std::endl;
		return nrOfFailedTests;
	}

	// enumerate all remainder cases for an integer<nbits, BlockType> configuration
	template<unsigned nbits, typename BlockType>
	int VerifyRemainder(bool reportTestCases) {
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
					if (reportTestCases) ReportBinaryArithmeticError("FAIL", "%", ia, ib, iref, iresult);
				}
				else {
					//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "%", ia, ib, iref, iresult);
				}
				if (nrOfFailedTests > 100) return nrOfFailedTests;
			}
			if (reportTestCases) if (i % 1024 == 0) std::cout << '.';
		}
		if (reportTestCases) std::cout << std::endl;
		return nrOfFailedTests;
	}

}} // namespace sw::universal
