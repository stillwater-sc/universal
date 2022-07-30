#pragma once
// test_suite_arithmetic.hpp : generic arithmetic test suite for arbitrary universal number systems
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
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
/// enumerate all negation cases for a TestType
/// </summary>
/// <param name="tag"></param>
/// <param name="reportTestCases"></param>
/// <returns></returns>
template<typename TestType>
int VerifyNegation(const std::string& tag, bool reportTestCases) {
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (size_t(1) << nbits);
	int nrOfFailedTests = 0;
	TestType a(0), neg(0), ref(0);

	for (size_t i = 1; i < NR_TEST_CASES; i++) {
		a.set_raw_bits(i);
		neg = -a;
		// generate reference
		double da = double(a);
		ref = -da;
		if (neg != ref) {
			nrOfFailedTests++;
			if (reportTestCases)	ReportUnaryArithmeticError("FAIL", "-", a, ref, neg);
		}
		else {
			//if (reportTestCases) ReportUnaryArithmeticSuccess("PASS", "-", a, ref, neg);
		}
	}
	return nrOfFailedTests;
}

/// <summary>
/// Enumerate all addition cases for a number system configuration.
/// Uses doubles to create a reference to compare to.
/// </summary>
/// <typeparam name="TestType">the number system type to verify</typeparam>
/// <param name="tag">string representation of the type</param>
/// <param name="reportTestCases">if yes, report on individual test failures</param>
/// <returns></returns>
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
/// Enumerate all subtraction cases for a number system configuration.
/// Uses doubles to create a reference to compare to.
/// </summary>
/// <typeparam name="TestType">the number system type to verify</typeparam>
/// <param name="tag">string representation of the type</param>
/// <param name="reportTestCases">if yes, report on individual test failures</param>
/// <returns></returns>
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

}} // namespace sw::universal
