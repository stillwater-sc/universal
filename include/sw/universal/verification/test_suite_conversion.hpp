#pragma once
// test_suite_conversion.hpp : conversion test suite for arbitrary universal number systems
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
// This usually entails setting environment variables, such as
// #define POSIT_THOW_ARITHMETIC_EXCEPTIONS 1

#include <universal/verification/test_status.hpp>
#include <universal/verification/test_reporters.hpp>  // error/success reporting

namespace sw { namespace universal {

/////////////////////////////// VERIFICATION TEST SUITES ////////////////////////////////

template<typename TestType>
int CompareAgainstDouble(double input, const TestType& testValue, double reference, bool reportTestCases) {
	int fail = 0;
	double result = double(testValue);
	if (std::fabs(result - reference) > 0.000000001) {
		fail++;
		if (reportTestCases)	ReportConversionError("FAIL", "=", input, reference, testValue);
	}
	else {
		// if (reportTestCases) ReportConversionSuccess("PASS", "=", input, reference, testValue);
	}
	return fail;
}

template<typename TestType, typename RefType, typename SrcType>
int Compare(SrcType input, const TestType& nut, const RefType& reference, bool reportTestCases) {
	int fail = 0;
	if (nut != reference) {
		fail++;
		if (reportTestCases)	ReportConversionError("FAIL", "=", double(input), nut, double(reference));
	}
	else {
		//if (reportTestCases) ReportConversionSuccess("PASS", "=", double(input), nut, double(reference));
	}
	return fail;
}

///////////////////////////////////////////////////////////////////////////////////////
///                        ASSIGNMENT/CONVERSION TEST SUITES                        ///
///////////////////////////////////////////////////////////////////////////////////////

template<typename TestType, typename RefType>
int VerifyAssignment(bool reportTestCases, bool verbose = false) {
	// algorithm: TestType raw -> to value in RefType -> assign back to TestType -> compare resulting TestTypes
	TestType number, assigned;
	constexpr size_t nbits = number.nbits;  // number system concept requires a static member indicating its size in bits
	constexpr size_t NR_NUMBERS = (size_t(1) << nbits);
	int nrOfFailedTestCases = 0;

	for (size_t i = 0; i < NR_NUMBERS; i++) {
		number.setbits(i); 
		if (verbose) std::cout << to_binary(number) << std::endl;
		RefType value = (RefType)(number);
		assigned = value;
		if (verbose) std::cout << number << " " << value << " " << assigned << std::endl;
		if (number != assigned) {
			nrOfFailedTestCases++;
			if (reportTestCases) ReportAssignmentError("FAIL", "=", number, assigned, value);
		}
		else {
			if (verbose && reportTestCases) ReportAssignmentSuccess("PASS", "=", number, assigned, value);
		}
	}
	return nrOfFailedTestCases;
}

// enumerate all conversion cases for integers
template<typename TestType>
int VerifyIntegerConversion(bool reportTestCases) {
	// we generate numbers from 1 to NaN to -1 and the special case of 0
	constexpr size_t nbits = TestType::nbits; 
	constexpr size_t NR_OF_TESTS = (size_t(1) << (nbits - 1)) + 1;
	int nrOfFailedTestCases = 0;

	TestType a(0);
	if (!a.iszero()) nrOfFailedTestCases++;

	a = 1;
//	if (!a.isone()) nrOfFailedTestCases++;
	for (size_t i = 0; i < NR_OF_TESTS; ++i) {
		a.setbits(i);
		if (!a.isnan()) {             // <---- we need to support an isnan() for posits to unify the API
			long long ref = (long long)a;
			TestType result = ref;
			if (result != ref) {
				if (reportTestCases) std::cout << " FAIL " << a << " != " << ref << std::endl;
			}
			else {
				// if (reportTestCases) std::cout << " PASS " << a << " == " << ref << std::endl;
			}
		}
		++a;  // assumes that the number system has an encoding enumerator operator++()
	}
	return nrOfFailedTestCases;
}

// enumerate all conversion cases for a number system configuration that uses 'round-to-even'

/// <summary>
/// enumerate all conversion cases for a TestType
/// </summary>
/// <typeparam name="TestType">the test configuration</typeparam>
/// <typeparam name="RefType">the reference configuration</typeparam>
/// <param name="tag">string to indicate what is being tested</param>
/// <param name="reportTestCases">if true print results of each test case. Default is false.</param>
/// <returns>number of failed test cases</returns>
template<typename TestType, typename RefType, typename SrcType = double>
int VerifyConversion(bool reportTestCases) {
	// we are going to generate a test set that consists of all configs and their midpoints
	// we do this by enumerating a configuration that is 1-bit larger than the test configuration
	// with the extra bit allocated to the fraction.
	// The sample values of the  larger configuration will be at the mid-point between the smaller 
	// configuration sample values thus creating a full cover test set for value conversions.
	// The precondition for this type of test is that the value conversion is verified.
	// To generate the three test cases, we'll enumerate the exact value, and a perturbation slightly
	// smaller from the midpoint that will round down, and one slightly larger that will round up,
	// to test the rounding logic of the conversion.
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (size_t(1) << (nbits + 1));
	constexpr size_t HALF = (size_t(1) << nbits);

	// For example: 
	// TestType: fixpnt<nbits,rbits,Saturating,uint8_t> needs RefType fixpnt<nbits+1, rbits+1, Saturating,uint8_t>
	// TestType: bfloat<nbits, es, uint8_t> needs RefType bfloat<nbits + 1, es, uint8_t>
	// TestType: posit<nbits, es, uint8_t> needs RefType posit<nbits + 1, es, uint8_t>

	const unsigned max = nbits > 20 ? 20 : nbits + 1;
	size_t max_tests = (size_t(1) << max);
	if (max_tests < NR_TEST_CASES) {
		std::cout << "VerifyConversion " << typeid(TestType).name() << ": NR_TEST_CASES = " << NR_TEST_CASES << " clipped by " << max_tests << std::endl;
	}

	// execute the test
	int nrOfFailedTests = 0;
	RefType positive_minimum;
	double dminpos = double(minpos(positive_minimum));
	RefType negative_maximum;
	double dmaxneg = double(maxneg(negative_maximum));

	// NUT: number under test
	TestType nut, golden;
	double eps = dminpos / 2.0;  // the test value between 0 and minpos
	for (size_t i = 0; i < NR_TEST_CASES && i < max_tests; ++i) {
		RefType ref, prev, next;
		SrcType testValue{ 0.0 };
		ref.setbits(i);
		SrcType da = SrcType(ref);
		if (i > 0) {
			eps = 1.0e-6; // da > 0 ? da * 1.0e-6 : da * -1.0e-6;
		}
		if (i % 2) {
			if (i == 1)
			{
				// special case of a tie that needs to round to even -> 0
				testValue = da;
				nut = testValue;
				golden = 0.0f;
				nrOfFailedTests += Compare(testValue, nut, golden, reportTestCases);

				// this rounds up
				testValue = da + eps;
				nut = testValue;
				next.setbits(i + 1);
				golden = double(next);
				nrOfFailedTests += Compare(testValue, nut, golden, reportTestCases);

			}
			else if (i == HALF - 1) {
				// special case of projecting to maxpos
				testValue = da - eps;
				nut = testValue;
				prev.setbits(HALF - 2);
				golden = double(prev);
				nrOfFailedTests += Compare(testValue, nut, golden, reportTestCases);
			}
			else if (i == HALF + 1) {
				// special case of projecting to maxneg
				testValue = da - eps;
				nut = testValue;
				golden = dmaxneg;
				nrOfFailedTests += Compare(testValue, nut, golden, reportTestCases);
			}
			else if (i == NR_TEST_CASES - 1) {
				// special case of projecting to minneg
				testValue = da - eps;
				nut = testValue;
				prev.setbits(i - 1);
				golden = double(prev);
				nrOfFailedTests += Compare(testValue, nut, golden, reportTestCases);

				// but the +delta goes to 0
				testValue = da + eps;
				nut = testValue;
				//				nrOfFailedTests += Compare(testValue, nut, (double)prev, reportTestCases);
				golden = 0.0f;
				nrOfFailedTests += Compare(testValue, nut, golden, reportTestCases);
			}
			else {
				// for odd values, we are between fixed point values, so we create the round-up and round-down cases
				// round-down
				testValue = da - eps;
				nut = testValue;
				prev.setbits(i - 1);
				golden = double(prev);
				nrOfFailedTests += Compare(testValue, nut, golden, reportTestCases);
				// round-up
				testValue = da + eps;
				nut = testValue;
				next.setbits(i + 1);
				golden = double(next);
				nrOfFailedTests += Compare(testValue, nut, golden, reportTestCases);
			}
		}
		else {
			// for the even values, we generate the round-to-actual cases
			if (i == 0) {
				// ref = 0
				// 0                -> value = 0
				// half of next     -> value = 0
				// special case of assigning to 0
				testValue = da;
				nut = testValue;
				golden = 0.0f;
				nrOfFailedTests += Compare(testValue, nut, golden, reportTestCases);

				testValue = da + eps;
				nut = testValue;
				nrOfFailedTests += Compare(testValue, nut, golden, reportTestCases);
			}
			else if (i == NR_TEST_CASES - 2) {
				// special case of projecting to minneg
				testValue = da - eps;
				nut = testValue;
				prev.setbits(NR_TEST_CASES - 2);
				golden = double(prev);
				nrOfFailedTests += Compare(testValue, nut, golden, reportTestCases);
			}
			else {
				// for even values, we are on actual representable values, so we create the round-up and round-down cases
				// round-up
				testValue = da - eps;
				nut = testValue;
				golden = da;
				nrOfFailedTests += Compare(testValue, nut, golden, reportTestCases);
				// round-down
				testValue = da + eps;
				nut = testValue;
				nrOfFailedTests += Compare(testValue, nut, golden, reportTestCases);
			}
		}
	}
	return nrOfFailedTests;
}

}} // namespace sw::universal
