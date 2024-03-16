#pragma once
// test_suite_arithmetic.hpp : generic arithmetic test suite for arbitrary universal number systems
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <typeinfo>

// CALLING ENVIRONMENT PREREQUISITE!!!!!
// We want the test suite to be used with different configurations of number systems
// so the calling environment needs to set the configuration
// This usually entails setting environment variables, such as THROW_ARITHMETIC_EXCEPTIONS
// as a function of the configured state of the number system.
// If it is not set, default is to turn it on.
#ifndef THROW_ARITHMETIC_EXCEPTION
#define THROW_ARITHMETIC_EXCEPTION 1
#endif

#include <universal/number/shared/specific_value_encoding.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_reporters.hpp>  // error/success reporting

namespace sw { namespace universal {


/////////////////////////////// VERIFICATION TEST SUITES ////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////
///                             ARITHMETIC TEST SUITES                              ///
///////////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// enumerate all negation cases for a number system configuration
/// </summary>
/// <param name="reportTestCases"></param>
/// <returns>number of failed test cases</returns>
template<typename TestType>
int VerifyNegation(bool reportTestCases) {
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (size_t(1) << nbits);
	int nrOfFailedTests = 0;
	TestType a(0), negated(0), ref(0);

	for (size_t i = 1; i < NR_TEST_CASES; i++) {
		a.setbits(i);
		negated = -a;
		// generate reference
		double da = double(a);
		ref = -da;
		if (negated != ref) {
			nrOfFailedTests++;
			if (reportTestCases)	ReportUnaryArithmeticError("FAIL", "-", a, negated, ref);
		}
		else {
			//if (reportTestCases) ReportUnaryArithmeticSuccess("PASS", "-", a, negated, ref);
		}
	}
	return nrOfFailedTests;
}

/// <summary>
/// Enumerate all addition cases for a number system configuration.
/// Uses doubles to create a reference to compare to.
/// </summary>
/// <typeparam name="TestType">the number system type to verify</typeparam>
/// <param name="reportTestCases">if yes, report on individual test failures</param>
/// <returns>number of failed test cases</returns>
template<typename TestType>
int VerifyAddition(bool reportTestCases) {
	constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
	constexpr size_t NR_VALUES = (size_t(1) << nbits);
	int nrOfFailedTests = 0;

	// set the saturation clamps
	TestType maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);

	double da, db, ref;  // make certain that IEEE doubles are sufficient as reference
	TestType a, b, c, cref;
	for (size_t i = 0; i < NR_VALUES; i++) {
		a.setbits(i); // number system concept requires a member function setbits()
		da = double(a);
		for (size_t j = 0; j < NR_VALUES; j++) {
			b.setbits(j);
			db = double(b);
			ref = da + db;
#if THROW_ARITHMETIC_EXCEPTION
			// catching overflow
			try {
				c = a + b;
			}
			catch (...) {
				if (ref < double(maxneg) || ref > double(maxpos)) {
					// correctly caught the overflow exception
					continue;
				}
				else {
					nrOfFailedTests++;
				}
			}
#else
			c = a + b;
#endif // THROW_ARITHMETIC_EXCEPTION
			cref = ref;
			if (c != cref) {
				if (ref == 0 and c.iszero()) continue; // mismatched is ignored as compiler optimizes away negative zero
				nrOfFailedTests++;
				if (reportTestCases)	ReportBinaryArithmeticError("FAIL", "+", a, b, c, ref);
			}
			else {
				//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "+", a, b, c, ref);
			}
		}
		if constexpr (NR_VALUES > 256 * 256) {
			if (i % (NR_VALUES / 25) == 0) std::cout << '.';
		}
	}
	std::cout << std::endl;
	return nrOfFailedTests;
}

/// <summary>
/// Enumerate all in-place (+=) addition cases for a number system configuration.
/// Uses doubles to create a reference to compare to.
/// </summary>
/// <typeparam name="TestType">the number system type to verify</typeparam>
/// <param name="tag">string representation of the type</param>
/// <param name="reportTestCases">if yes, report on individual test failures</param>
/// <returns></returns>
template<typename TestType>
int VerifyInPlaceAddition(bool reportTestCases) {
	constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
	constexpr size_t NR_VALUES = (size_t(1) << nbits);
	int nrOfFailedTests = 0;

	// set the saturation clamps
	TestType maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);

	double da, db, ref;  // make certain that IEEE doubles are sufficient as reference
	TestType a, b, c, cref;
	for (size_t i = 0; i < NR_VALUES; i++) {
		a.setbits(i); // number system concept requires a member function setbits()
		da = double(a);
		for (size_t j = 0; j < NR_VALUES; j++) {
			b.setbits(j);
			db = double(b);
			ref = da + db;
#if THROW_ARITHMETIC_EXCEPTION
			// catching overflow
			try {
				c = a;
				c += b;
			}
			catch (...) {
				if (ref < double(maxneg) || ref > double(maxpos)) {
					// correctly caught the overflow exception
					continue;
				}
				else {
					nrOfFailedTests++;
				}
			}
#else
			c = a;
			c += b;
#endif // THROW_ARITHMETIC_EXCEPTION
			cref = ref;
			if (c != cref) {
				if (ref == 0 and c.iszero()) continue; // mismatched is ignored as compiler optimizes away negative zero
				nrOfFailedTests++;
				if (reportTestCases)	ReportBinaryArithmeticError("FAIL", "+", a, b, c, ref);
			}
			else {
				//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "+", a, b, c, ref);
			}
		}
		if constexpr (NR_VALUES > 256 * 256) {
			if (i % (NR_VALUES / 25) == 0) std::cout << '.';
		}
	}
	std::cout << std::endl;
	return nrOfFailedTests;
}

/// <summary>
/// Enumerate all subtraction cases for a number system configuration.
/// Uses doubles to create a reference to compare to.
/// </summary>
/// <typeparam name="TestType">the number system type to verify</typeparam>
/// <param name="reportTestCases">if yes, report on individual test failures</param>
/// <returns>number of failed test cases</returns>
template<typename TestType>
int VerifySubtraction(bool reportTestCases) {
	constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
	constexpr size_t NR_VALUES = (size_t(1) << nbits);
	int nrOfFailedTests = 0;

	// set the saturation clamps
	TestType maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);

	double da, db, ref;  // make certain that IEEE doubles are sufficient as reference
	TestType a, b, c, cref;
	for (size_t i = 0; i < NR_VALUES; i++) {
		a.setbits(i); // number system concept requires a member function setbits()
		da = double(a);
		for (size_t j = 0; j < NR_VALUES; j++) {
			b.setbits(j);
			db = double(b);
			ref = da - db;
#if THROW_ARITHMETIC_EXCEPTION
			// catching overflow
			try {
				c = a - b;
			}
			catch (...) {
				if (ref < double(maxneg) || ref > double(maxpos)) {
					// correctly caught the overflow exception
					continue;
				}
				else {
					nrOfFailedTests++;
				}
			}
#else
			c = a - b;
#endif // THROW_ARITHMETIC_EXCEPTION
			cref = ref;
			if (c != cref) {
				if (ref == 0 and c.iszero()) continue; // mismatched is ignored as compiler optimizes away negative zero
				nrOfFailedTests++;
				if (reportTestCases)	ReportBinaryArithmeticError("FAIL", "-", a, b, c, ref);
			}
			else {
				//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "-", a, b, c, ref);
			}
			if (nrOfFailedTests > 9) return nrOfFailedTests;
		}
		if constexpr (NR_VALUES > 256 * 256) {
			if (i % (NR_VALUES / 25) == 0) std::cout << '.';
		}
	}
	std::cout << std::endl;
	return nrOfFailedTests;
}

/// <summary>
/// Enumerate all in-place (-=) subtraction cases for a number system configuration.
/// Uses doubles to create a reference to compare to.
/// </summary>
/// <typeparam name="TestType">the number system type to verify</typeparam>
/// <param name="reportTestCases">if yes, report on individual test failures</param>
/// <returns>number of failed test cases</returns>
template<typename TestType>
int VerifyInPlaceSubtraction(bool reportTestCases) {
	constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
	constexpr size_t NR_VALUES = (size_t(1) << nbits);
	int nrOfFailedTests = 0;

	// set the saturation clamps
	TestType maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);

	double da, db, ref;  // make certain that IEEE doubles are sufficient as reference
	TestType a, b, c, cref;
	for (size_t i = 0; i < NR_VALUES; i++) {
		a.setbits(i); // number system concept requires a member function setbits()
		da = double(a);
		for (size_t j = 0; j < NR_VALUES; j++) {
			b.setbits(j);
			db = double(b);
			ref = da - db;
#if THROW_ARITHMETIC_EXCEPTION
			// catching overflow
			try {
				c = a;
				c -= b;
			}
			catch (...) {
				if (ref < double(maxneg) || ref > double(maxpos)) {
					// correctly caught the overflow exception
					continue;
				}
				else {
					nrOfFailedTests++;
				}
			}
#else
			c = a;
			c -= b;
#endif // THROW_ARITHMETIC_EXCEPTION
			cref = ref;
			if (c != cref) {
				if (ref == 0 and c.iszero()) continue; // mismatched is ignored as compiler optimizes away negative zero
				nrOfFailedTests++;
				if (reportTestCases)	ReportBinaryArithmeticError("FAIL", "-", a, b, c, ref);
			}
			else {
				//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "-", a, b, c, ref);
			}
			if (nrOfFailedTests > 9) return nrOfFailedTests;
		}
		if constexpr (NR_VALUES > 256 * 256) {
			if (i % (NR_VALUES / 25) == 0) std::cout << '.';
		}
	}
	std::cout << std::endl;
	return nrOfFailedTests;
}

/// <summary>
/// Enumerate all multiplication cases for a number system configuration.
/// Uses doubles to create a reference to compare to.
/// </summary>
/// <typeparam name="TestType">the number system type to verify</typeparam>
/// <param name="reportTestCases">if yes, report on individual test failures</param>
/// <returns>number of failed test cases</returns>
template<typename TestType>
int VerifyMultiplication(bool reportTestCases) {
	constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
	const unsigned NR_VALUES = (unsigned(1) << nbits);
	int nrOfFailedTests = 0;

	TestType a, b, c, cref;
	for (unsigned i = 0; i < NR_VALUES; i++) {
		a.setbits(i);
		double da = double(a);
		for (unsigned j = 0; j < NR_VALUES; j++) {
			b.setbits(j);
			double db = double(b);
			double ref = da * db; // make certain that IEEE doubles are sufficient as reference
#if THROW_ARITHMETIC_EXCEPTION
			try {
				c = a * b;
			}
			catch (...) {
				if (a.isnan() || b.isnan()) {
					// correctly caught the exception
					c.setnan(true); // TODO: unify quiet vs signalling propagation among real number systems
					// posits behave differently than floats, so this may need a least common denominator approach
				}
				else {
					throw;  // rethrow
				}
			}
#else
			c = a * b;
#endif
			cref = ref;
			if (c != cref) {
				if (reportTestCases) ReportBinaryArithmeticError("FAIL", "*", a, b, c, ref);
				nrOfFailedTests++;
			}
			else {
				//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "*", a, b, c, ref);
			}
		}
	}
	return nrOfFailedTests;
}

/// <summary>
/// Enumerate all in-place (*=) multiplication cases for a number system configuration.
/// Uses doubles to create a reference to compare to.
/// </summary>
/// <typeparam name="TestType">the number system type to verify</typeparam>
/// <param name="reportTestCases">if yes, report on individual test failures</param>
/// <returns>number of failed test cases</returns>
template<typename TestType>
int VerifyInPlaceMultiplication(bool reportTestCases) {
	constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
	const unsigned NR_VALUES = (unsigned(1) << nbits);
	int nrOfFailedTests = 0;

	TestType a, b, c, cref;
	for (unsigned i = 0; i < NR_VALUES; i++) {
		a.setbits(i);
		double da = double(a);
		for (unsigned j = 0; j < NR_VALUES; j++) {
			b.setbits(j);
			double db = double(b);
			double ref = da * db;  // make certain that IEEE doubles are sufficient as reference
#if THROW_ARITHMETIC_EXCEPTION
			try {
				c = a;
				c *= b;
			}
			catch (...) {
				if (a.isnan() || b.isnan()) {
					// correctly caught the exception
					c.setnan(true); // TODO: unify quiet vs signalling propagation among real number systems
					// posits behave differently than floats, so this may need a least common denominator approach
				}
				else {
					throw;  // rethrow
				}
			}
#else
			c = a;
			c *= b;
#endif
			cref = ref;
			if (c != cref) {
				if (reportTestCases) ReportBinaryArithmeticError("FAIL", "*", a, b, c, ref);
				nrOfFailedTests++;
			}
			else {
				//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "*", a, b, c, ref);
			}
		}
	}
	return nrOfFailedTests;
}

/// <summary>
/// Enumerate all division cases for a number system configuration.
/// Uses doubles to create a reference to compare to.
/// </summary>
/// <typeparam name="TestType">the number system type to verify</typeparam>
/// <param name="reportTestCases">if yes, report on individual test failures</param>
/// <returns>number of failed test cases</returns>
template<typename TestType>
int VerifyDivision(bool reportTestCases) {
	constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
	const unsigned NR_VALUES = (unsigned(1) << nbits);
	int nrOfFailedTests = 0;

	TestType a, b, c, cref;
	for (unsigned i = 0; i < NR_VALUES; i++) {
		a.setbits(i);
		double da = double(a);
		for (unsigned j = 0; j < NR_VALUES; j++) {
			b.setbits(j);
			double db = double(b);
			double ref{ 0 }; // make certain that IEEE doubles are sufficient as reference
#if THROW_ARITHMETIC_EXCEPTION
			try {
				c = a / b;
				ref = da / db;
			}
			catch (...) {
				if (b.iszero()) {
					// correctly caught the exception
					c.setnan(true); // TODO: unify quiet vs signalling propagation among real number systems
					// posits behave differently than floats, so this may need a least common denominator approach
				}
				else if (a.isnan() || b.isnan()) {
					// Universal will throw a divide_by_nar or numerator_is_nar exception for posits
					c.setnan(true); // TODO: unify quiet vs signalling propagation among real number systems
				}
				else {
					throw;  // rethrow
				}
			}
#else
			c = a / b;
			ref = da / db;
#endif
			cref = ref;
			if (c != cref) {
				if (reportTestCases) ReportBinaryArithmeticError("FAIL", "*", a, b, c, ref);
				nrOfFailedTests++;
			}
			else {
				//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "*", a, b, c, ref);
			}
		}
	}
	return nrOfFailedTests;
}

/// <summary>
/// Enumerate all in-place (/=) division cases for a number system configuration.
/// Uses doubles to create a reference to compare to.
/// </summary>
/// <typeparam name="TestType">the number system type to verify</typeparam>
/// <param name="reportTestCases">if yes, report on individual test failures</param>
/// <returns>number of failed test cases</returns>
template<typename TestType>
int VerifyInPlaceDivision(bool reportTestCases) {
	constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
	const unsigned NR_VALUES = (unsigned(1) << nbits);
	int nrOfFailedTests = 0;

	TestType a, b, c, cref;
	for (unsigned i = 0; i < NR_VALUES; i++) {
		a.setbits(i);
		double da = double(a);
		for (unsigned j = 0; j < NR_VALUES; j++) {
			b.setbits(j);
			double db = double(b);
			double ref{ 0 };  // make certain that IEEE doubles are sufficient as reference
#if THROW_ARITHMETIC_EXCEPTION
			try {
				c = a;
				c /= b;
				ref = da / db;
			}
			catch (...) {
				if (b.iszero()) {
					// correctly caught the exception
					c.setnan(true); // TODO: unify quiet vs signalling propagation among real number systems
					// posits behave differently than floats, so this may need a least common denominator approach
				}
				if (a.isnan() || b.isnan()) {
					// Universal will throw a divide_by_nar or numerator_is_nar exception for posits
					c.setnan(true); // TODO: unify quiet vs signalling propagation among real number systems
				}
				else {
					throw;  // rethrow
				}
			}
#else
			c = a;
			c /= b;
			ref = da / db;
#endif
			cref = ref;
			if (c != cref) {
				if (reportTestCases) ReportBinaryArithmeticError("FAIL", "*", a, b, c, ref);
				nrOfFailedTests++;
			}
			else {
				//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "*", a, b, c, ref);
			}
		}
	}
	return nrOfFailedTests;
}

/// <summary>
/// Enumerate all reciprocation cases for a number system configuration.
/// Uses doubles to create a reference to compare to.
/// </summary>
/// <typeparam name="TestType">the number system type to verify</typeparam>
/// <param name="reportTestCases">if yes, report on individual test failures</param>
/// <returns>number of failed test cases</returns>
template<typename TestType>
int VerifyReciprocation(bool reportTestCases) {
	constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
	const unsigned NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTests = 0;
	for (unsigned i = 0; i < NR_TEST_CASES; i++) {
		TestType a, reciprocal, ref;
		a.setbits(i);
		double da = double(a);
#if THROW_ARITHMETIC_EXCEPTION
		try {
			reciprocal = a.reciprocal();
			ref = 1.0 / da;
		}
		catch (...) {
			if (a.iszero()) {
				// correctly caught divide by zero exception
			}
		}
#else
		reciprocal = a.reciprocate();
		ref = 1.0 / da;
#endif

		if (reciprocal != ref) {
			nrOfFailedTests++;
			if (reportTestCases)	ReportUnaryArithmeticError("FAIL", "reciprocate", a, reciprocal, ref);
		}
		else {
			//if (reportTestCases) ReportUnaryArithmeticSuccess("PASS", "reciprocate", a, reciprocal, ref);
		}
	}
	return nrOfFailedTests;
}

}} // namespace sw::universal
