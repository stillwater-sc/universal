#pragma once
// test_suite_arithmetic.hpp : arithmetic test suite for arbitrary universal number systems
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <typeinfo>

// CALLING ENVIRONMENT PREREQUISITE!!!!!
// We want the test suite to be used with different configurations of number systems
// so the calling environment needs to set the configuration
// This usually entails setting environment variables, such as
// #default POSIT_THOW_ARITHMETIC_EXCEPTIONS 1

#include <universal/verification/test_status.hpp>
#include <universal/verification/test_reporters.hpp>  // error/success reporting

namespace sw::universal {


/////////////////////////////// VERIFICATION TEST SUITES ////////////////////////////////

template<typename TestType>
int Compare(double input, const TestType& testValue, double reference, bool bReportIndividualTestCases) {
	int fail = 0;
	double result = double(testValue);
	if (std::fabs(result - reference) > 0.000000001) {
		fail++;
		if (bReportIndividualTestCases)	ReportConversionError("FAIL", "=", input, reference, testValue);
	}
	else {
		// if (bReportIndividualTestCases) ReportConversionSuccess("PASS", "=", input, reference, testValue);
	}
	return fail;
}

///////////////////////////////////////////////////////////////////////////////////////
///                        ASSIGNMENT/CONVERSION TEST SUITES                        ///
///////////////////////////////////////////////////////////////////////////////////////

template<typename TestType, typename RefType>
int VerifyAssignment(bool bReportIndividualTestCases, bool verbose = false) {
	// algorithm: TestType raw -> to value in RefType -> assign back to TestType -> compare resulting TestTypes
	TestType number, assigned;
	constexpr size_t nbits = number.nbits;  // number system concept requires a static member indicating its size in bits
	constexpr size_t NR_NUMBERS = (size_t(1) << nbits);
	int nrOfFailedTestCases = 0;

	for (size_t i = 0; i < NR_NUMBERS; i++) {
		number.set_raw_bits(i); 
		if (verbose) std::cout << to_binary(number) << std::endl;
		RefType value = (RefType)(number);
		assigned = value;
		if (verbose) std::cout << number << " " << value << " " << assigned << std::endl;
		if (number != assigned) {
			nrOfFailedTestCases++;
			if (bReportIndividualTestCases) ReportAssignmentError("FAIL", "=", number, assigned, value);
		}
		else {
			if (verbose && bReportIndividualTestCases) ReportAssignmentSuccess("PASS", "=", number, assigned, value);
		}
	}
	return nrOfFailedTestCases;
}

// enumerate all conversion cases for a fixed-point configuration

/// <summary>
/// enumerate all conversion cases for a TestType
/// </summary>
/// <typeparam name="TestType">the test configuration</typeparam>
/// <typeparam name="RefType">the reference configuration</typeparam>
/// <param name="tag">string to indicate what is being tested</param>
/// <param name="bReportIndividualTestCases">if true print results of each test case. Default is false.</param>
/// <returns>number of failed test cases</returns>
template<typename TestType, typename RefType>
int VerifyConversion(const std::string& tag, bool bReportIndividualTestCases) {
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
	// TestType: areal<nbits,es,uint8_t> needs RefType areal<nbits + 1, es, uint8_t>
	// TestType: posit<nbits,es,uint8_t> needs RefType posit<nbits + 1, es, uint8_t>

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
	TestType nut;
	double eps = dminpos / 2.0;  // the test value between 0 and minpos
	for (size_t i = 0; i < NR_TEST_CASES && i < max_tests; ++i) {
		RefType ref, prev, next;
		double testValue{ 0.0 };
		ref.set_raw_bits(i);
		double da = double(ref);
		if (i > 0) {
			eps = 1.0e-6; // da > 0 ? da * 1.0e-6 : da * -1.0e-6;
		}
		if (i % 2) {
			if (i == 1) {
				// special case of a tie that needs to round to even -> 0
				testValue = da;
				nut = testValue;
				nrOfFailedTests += Compare(testValue, nut, 0.0, bReportIndividualTestCases);

				// this rounds up
				testValue = da + eps;
				nut = testValue;
				next.set_raw_bits(i + 1);
				nrOfFailedTests += Compare(testValue, nut, (double)next, bReportIndividualTestCases);

			}
			else if (i == HALF - 1) {
				// special case of projecting to maxpos
				testValue = da - eps;
				nut = testValue;
				prev.set_raw_bits(HALF - 2);
				nrOfFailedTests += Compare(testValue, nut, (double)prev, bReportIndividualTestCases);
			}
			else if (i == HALF + 1) {
				// special case of projecting to maxneg
				testValue = da - eps;
				nut = testValue;
				nrOfFailedTests += Compare(testValue, nut, dmaxneg, bReportIndividualTestCases);
			}
			else if (i == NR_TEST_CASES - 1) {
				// special case of projecting to minneg
				testValue = da - eps;
				nut = testValue;
				prev.set_raw_bits(i - 1);
				nrOfFailedTests += Compare(testValue, nut, (double)prev, bReportIndividualTestCases);
				// but the +delta goes to 0
				testValue = da + eps;
				nut = testValue;
				//				nrOfFailedTests += Compare(testValue, nut, (double)prev, bReportIndividualTestCases);
				nrOfFailedTests += Compare(testValue, nut, 0.0, bReportIndividualTestCases);
			}
			else {
				// for odd values, we are between fixed point values, so we create the round-up and round-down cases
				// round-down
				testValue = da - eps;
				nut = testValue;
				prev.set_raw_bits(i - 1);
				nrOfFailedTests += Compare(testValue, nut, (double)prev, bReportIndividualTestCases);
				// round-up
				testValue = da + eps;
				nut = testValue;
				next.set_raw_bits(i + 1);
				nrOfFailedTests += Compare(testValue, nut, (double)next, bReportIndividualTestCases);
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
				nrOfFailedTests += Compare(testValue, nut, da, bReportIndividualTestCases);

				testValue = da + eps;
				nut = testValue;
				nrOfFailedTests += Compare(testValue, nut, da, bReportIndividualTestCases);
			}
			else if (i == NR_TEST_CASES - 2) {
				// special case of projecting to minneg
				testValue = da - eps;
				nut = testValue;
				prev.set_raw_bits(NR_TEST_CASES - 2);
				nrOfFailedTests += Compare(testValue, nut, (double)prev, bReportIndividualTestCases);
			}
			else {
				// for even values, we are on actual fixed point values, so we create the round-up and round-down cases
				// round-up
				testValue = da - eps;
				nut = testValue;
				nrOfFailedTests += Compare(testValue, nut, da, bReportIndividualTestCases);
				// round-down
				testValue = da + eps;
				nut = testValue;
				nrOfFailedTests += Compare(testValue, nut, da, bReportIndividualTestCases);
			}
		}
	}
	return nrOfFailedTests;
}

///////////////////////////////////////////////////////////////////////////////////////
///                             ARITHMETIC TEST SUITES                              ///
///////////////////////////////////////////////////////////////////////////////////////

// enumerate all addition cases for a number system configuration
template<typename TestType>
int VerifyAddition(const std::string& tag, bool bReportIndividualTestCases) {
	TestType a{ 0 }, b{ 0 }, result, cref;
	constexpr size_t nbits = a.nbits;  // number system concept requires a static member indicating its size in bits
	constexpr size_t NR_VALUES = (size_t(1) << nbits);
	int nrOfFailedTests = 0;

	// set the saturation clamps
	TestType maxpositive{ 0 }, maxnegative{ 0 };
	maxpos(maxpositive); // depend on ADL to specialize
	maxneg(maxnegative);

	double da, db, ref;  // make certain that IEEE doubles are sufficient as reference
	for (size_t i = 0; i < NR_VALUES; i++) {
		a.set_raw_bits(i); // number system concept requires a member function set_raw_bits()
		da = double(a);
		for (size_t j = 0; j < NR_VALUES; j++) {
			b.set_raw_bits(j);
			db = double(b);
			ref = da + db;
#if THROW_ARITHMETIC_EXCEPTION
			// catching overflow
			try {
				result = a + b;
			}
			catch (...) {
				if (ref < double(maxnegitive) || ref > double(maxpositive)) {
					// correctly caught the overflow exception
					continue;
				}
				else {
					nrOfFailedTests++;
				}
			}

#else
			result = a + b;
#endif // THROW_ARITHMETIC_EXCEPTION
			cref = ref;
			if (result != cref) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "+", a, b, cref, result);
			}
			else {
				//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "+", a, b, cref, result);
			}
			if (nrOfFailedTests > 100) return nrOfFailedTests;
		}
		if (NR_VALUES > 256*256) if (i % (NR_VALUES / 25) == 0) std::cout << '.';
	}
	std::cout << std::endl;
	return nrOfFailedTests;
}



} // namespace sw::universal
