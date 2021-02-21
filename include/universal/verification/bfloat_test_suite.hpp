#pragma once
//  real_test_helpers.hpp : arbitrary real verification functions
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <vector>
#include <iostream>
#include <typeinfo>
#include <random>
#include <limits>

namespace sw::universal {

	template<typename SrcType, typename TestType>
	void ReportConversionError(const std::string& test_case, const std::string& op, SrcType input, const TestType& reference, const TestType& result) {
		// constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
		auto old_precision = std::cerr.precision();
		std::cerr << test_case
			<< " " << op << " "
			<< std::setw(NUMBER_COLUMN_WIDTH) << input
			<< " did not convert to "
			<< std::setw(NUMBER_COLUMN_WIDTH) << reference << " instead it yielded  "
			<< std::setw(NUMBER_COLUMN_WIDTH) << result
			<< "  raw " << to_binary(result)
			<< std::setprecision(old_precision)
			<< std::endl;
	}

	template<typename SrcType, typename TestType>
	void ReportConversionSuccess(const std::string& test_case, const std::string& op, SrcType input, const TestType& reference, const TestType& result) {
		constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
		std::cerr << test_case
			<< " " << op << " "
			<< std::setw(NUMBER_COLUMN_WIDTH) << input
			<< " success            "
			<< std::setw(NUMBER_COLUMN_WIDTH) << result << " golden reference is "
			<< std::setw(NUMBER_COLUMN_WIDTH) << reference
			<< "  raw " << std::setw(nbits) << to_binary(result)
			<< std::endl;
	}

	template<typename SrcType, typename TestType>
	int Compare(SrcType input, const TestType& testValue, const TestType& reference, bool bReportIndividualTestCases) {
		int fail = 0;
		if (testValue != reference) {
			fail++;
			if (bReportIndividualTestCases)	ReportConversionError("FAIL", "=", input, reference, testValue);
		}
		else {
			if (bReportIndividualTestCases) ReportConversionSuccess("PASS", "=", input, reference, testValue);
		}
		return fail;
	}

	/////////////////////////////// VERIFICATION TEST SUITES ////////////////////////////////

	/*
   0:            b0000       0      -1             b00              b0                             0        4.2x0x0r
   1:            b0001       0      -1             b00              b1                           0.5        4.2x0x1r
   2:            b0010       0       0             b01              b0                             1        4.2x0x2r
   3:            b0011       0       0             b01              b1                           1.5        4.2x0x3r
   4:            b0100       0       1             b10              b0                             2        4.2x0x4r
   5:            b0101       0       1             b10              b1                             3        4.2x0x5r
   6:            b0110       0       2             b11              b0                           inf        4.2x0x6r
   7:            b0111       0       2             b11              b1                           nan        4.2x0x7r
   8:            b1000       1      -1             b00              b0                             0        4.2x0x8r

   0:           b00000       0      -2              b0            b000                             0       5.1x0x00r
   1:           b00001       0      -2              b0            b001                          0.25       5.1x0x01r
   2:           b00010       0      -1              b0            b010                           0.5       5.1x0x02r
   3:           b00011       0      -1              b0            b011                          0.75       5.1x0x03r
   4:           b00100       0       0              b0            b100                             1       5.1x0x04r
   5:           b00101       0       0              b0            b101                          1.25       5.1x0x05r
   6:           b00110       0       0              b0            b110                           1.5       5.1x0x06r
   7:           b00111       0       0              b0            b111                          1.75       5.1x0x07r
   8:           b01000       0       1              b1            b000                             2       5.1x0x08r
   9:           b01001       0       1              b1            b001                          2.25       5.1x0x09r
  10:           b01010       0       1              b1            b010                           2.5       5.1x0x0Ar
  11:           b01011       0       1              b1            b011                          2.75       5.1x0x0Br
  12:           b01100       0       1              b1            b100                             3       5.1x0x0Cr
  13:           b01101       0       1              b1            b101                          3.25       5.1x0x0Dr
  14:           b01110       0       1              b1            b110                           inf       5.1x0x0Er
  15:           b01111       0       1              b1            b111                           nan       5.1x0x0Fr
  16:           b10000       1      -2              b0            b000                             0       5.1x0x10r
	
   0:           b0000-0       0      -2              b0            b000                             0       5.1x0x00r  <---- 4.1x0x0r
   1:           b0000-1       0      -2              b0            b001                          0.25       5.1x0x01r
   2:           b0001-0       0      -1              b0            b010                           0.5       5.1x0x02r  <---- 4.1x0x1r
   3:           b0001-1       0      -1              b0            b011                          0.75       5.1x0x03r
   4:           b0010-0       0       0              b0            b100                             1       5.1x0x04r  <---- 4.1x0x2r
   5:           b0010-1       0       0              b0            b101                          1.25       5.1x0x05r
   6:           b0011-0       0       0              b0            b110                           1.5       5.1x0x06r  <---- 4.1x0x3r
   7:           b0011-1       0       0              b0            b111                          1.75       5.1x0x07r
   8:           b0100-0       0       1              b1            b000                             2       5.1x0x08r  <---- 4.1x0x4r
   9:           b0100-1       0       1              b1            b001                          2.25       5.1x0x09r
  10:           b0101-0       0       1              b1            b010                           2.5       5.1x0x0Ar  <---- 4.1x0x4r
  11:           b0101-1       0       1              b1            b011                          2.75       5.1x0x0Br
  12:           b0110-0       0       1              b1            b100                             3       5.1x0x0Cr  <---- 4.1x0x4r
  13:           b0110-1       0       1              b1            b101                          3.25       5.1x0x0Dr
  14:           b0111-0       0       1              b1            b110                           inf       5.1x0x0Er  <---- 4.1x0x7r
  15:           b0111-1       0       1              b1            b111                           nan       5.1x0x0Fr
  16:           b1000-0       1      -2              b0            b000                             0       5.1x0x10r  <---- 4.1x0x7r

  VerifyConversion algorithm: enumerate bfloat<nbits+1, es> and create minus and plus deltas that you a priori know which way they round
																								 1.00 - delta       round up
   4:           b0010-0       0       0              b0            b100                             1       5.1x0x04r  <---- 4.1x0x2r
                                                                                                 1.00 + delta       round down

																								 1.25 - delta       round up
   5:           b0010-1       0       0              b0            b101                          1.25       5.1x0x05r
																								 1.25 + delta       round down

																								 1.50 - delta       round up
   6:           b0011-0       0       0              b0            b110                           1.5       5.1x0x06r  <---- 4.1x0x3r
																								 1.50 + delta       round down

																								 1.75 - delta       round up
   7:           b0011-1       0       0              b0            b111                          1.75       5.1x0x07r
																								 1.75 + delta       round down


   8:           b0100-0       0       1              b1            b000                             2       5.1x0x08r  <---- 4.1x0x4r

	*/

/// <summary>
/// enumerate all conversion cases for a TestType
/// </summary>
/// <typeparam name="TestType">the test configuration</typeparam>
/// <typeparam name="RefType">the reference configuration</typeparam>
/// <param name="tag">string to indicate what is being tested</param>
/// <param name="bReportIndividualTestCases">if true print results of each test case. Default is false.</param>
/// <returns>number of failed test cases</returns>
	template<typename TestType, typename SrcType = double>
	int VerifyBfloatConversion(bool bReportIndividualTestCases) {
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
		constexpr size_t es = TestType::es;
		using BlockType = typename TestType::BlockType;
		using RefType = bfloat<nbits + 1, es, BlockType>;
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

		// NUT: number under test
		TestType nut, golden;
		for (size_t i = 0; i < NR_TEST_CASES && i < max_tests; ++i) {
			RefType ref, prev, next;
			SrcType testValue{ 0.0 };
			ref.set_raw_bits(i);
			SrcType da = SrcType(ref);
			int old = nrOfFailedTests;
			SrcType oneULP = (da > 0 ? ulp(da) : -ulp(da));

			if (i % 2) {
				if (i == 1)	{
					// special case of a tie that needs to round to even -> 0
					testValue = da;
					nut = testValue;
					golden = 0.0f;
					nrOfFailedTests += Compare(testValue, nut, golden, bReportIndividualTestCases);

					// this rounds up 
					testValue = SrcType(da + oneULP);  // the test value between 0 and minpos
					nut = testValue;
					next.set_raw_bits(i + 1);
					golden = double(next);
					nrOfFailedTests += Compare(testValue, nut, golden, bReportIndividualTestCases);
				}
				else if (i == HALF - 3) { // encoding of maxpos
					// ignore
					if (bReportIndividualTestCases) std::cout << i << " : >" << da << " ignored\n";
				}
				else if (i == HALF - 1) { // encoding of qNaN
					// ignore
					if (bReportIndividualTestCases) std::cout << i << " : >" << da << " ignored\n";
				}
				else if (i == HALF + 1) {
					// special case of projecting to -0
					testValue = SrcType(da - oneULP);
					nut = testValue;
					golden = 0.0f; golden = -golden;
					nrOfFailedTests += Compare(testValue, nut, golden, bReportIndividualTestCases);
				}
				else if (i == NR_TEST_CASES - 3) { // encoding of maxneg
					// ignore
					if (bReportIndividualTestCases) std::cout << i << " : < " << da << " ignored\n";
				}
				else if (i == NR_TEST_CASES - 1) { // encoding of SIGNALLING NAN
					// ignore
					if (bReportIndividualTestCases) std::cout << i << " : < " << da << " ignored\n";
				}
				else {
					// for odd values of i, we are between sample values of the NUT
					// create the round-up and round-down cases
					// round-down
					testValue = SrcType(da - oneULP);
					nut = testValue;
					prev.set_raw_bits(i - 1);
					golden = double(prev);
					nrOfFailedTests += Compare(testValue, nut, golden, bReportIndividualTestCases);
					
					// round-up
					if (i == HALF - 5 || i == NR_TEST_CASES - 5) {
						if (bReportIndividualTestCases) std::cout << i << " : >" << da << " ignored\n";
					}
					else {
						testValue = SrcType(da + oneULP);
						nut = testValue;
						next.set_raw_bits(i + 1);
						golden = double(next);
						nrOfFailedTests += Compare(testValue, nut, golden, bReportIndividualTestCases);
					}
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
					golden.setzero(); // make certain we are +0
					nrOfFailedTests += Compare(testValue, nut, golden, bReportIndividualTestCases);

					// half of next rounds down to 0
					testValue = SrcType(dminpos / 2.0);
					nut = testValue;
					nrOfFailedTests += Compare(testValue, nut, golden, bReportIndividualTestCases);
				}
				else if (i == HALF) {
					// ref = -0
					// 0                -> value = 0
					// half of next     -> value = 0
					// special case of assigning to 0
					testValue = da;
					nut = testValue;
					golden.setzero(); golden = -golden; // make certain we are -0
					nrOfFailedTests += Compare(testValue, nut, golden, bReportIndividualTestCases);

					// half of next rounds down to -0
					testValue = SrcType(-dminpos / 2.0);
					nut = testValue;
					nrOfFailedTests += Compare(testValue, nut, golden, bReportIndividualTestCases);
				}
				else if (i == HALF - 4) {
					if (bReportIndividualTestCases) std::cout << i << " : > " << da << " ignored\n";
				}
				else if (i == HALF - 2) { // encoding of INF
					// ignore
					if (bReportIndividualTestCases) std::cout << i << " : " << da << " ignored\n";
				}
				else if (i == NR_TEST_CASES - 4) { // encoding of maxneg
					// ignore
					if (bReportIndividualTestCases) std::cout << i << " : < " << da << " ignored\n";
				}
				else if (i == NR_TEST_CASES - 2) { // encoding of -INF
					// ignore
					if (bReportIndividualTestCases) std::cout << i << " : " << da << " ignored\n";
				}
				else {
					// for even values, we are on actual representable values, so we create the round-up and round-down cases
					// round-up
					testValue = SrcType(da - oneULP);
					nut = testValue;
					golden = da;
					nrOfFailedTests += Compare(testValue, nut, golden, bReportIndividualTestCases);
					
					// round-down
					testValue = SrcType(da + oneULP);
					nut = testValue;
					nrOfFailedTests += Compare(testValue, nut, golden, bReportIndividualTestCases);
				}
			}
			if (bReportIndividualTestCases && nrOfFailedTests > old) {
				std::cout << to_binary(oneULP, true) << " : " << oneULP << '\n';
				std::cout << to_binary(da - oneULP, true) << " : " << da - oneULP << '\n';
				std::cout << to_binary(da, true) << " : " << da << '\n';
				std::cout << to_binary(da + oneULP, true) << " : " << da + oneULP << '\n';
				std::cout << "[" << i << "]\n";
			}
		}
		return nrOfFailedTests;
	}


	// validate the increment operator++
	template<size_t nbits, size_t es>
	int VerifyIncrement(bool bReportIndividualTestCases)
	{
		std::vector< bfloat<nbits, es> > set;
		//	GenerateOrderedPositSet(set); // [NaR, -maxpos, ..., -minpos, 0, minpos, ..., maxpos]

		int nrOfFailedTestCases = 0;

		bfloat<nbits, es> p, ref;
		// starting from NaR iterating from -maxpos to maxpos through zero
		for (typename std::vector < bfloat<nbits, es> >::iterator it = set.begin(); it != set.end() - 1; ++it) {
			p = *it;
			p++;
			ref = *(it + 1);
			if (p != ref) {
				if (bReportIndividualTestCases) std::cout << " FAIL " << p << " != " << ref << std::endl;
				nrOfFailedTestCases++;
			}
		}

		return nrOfFailedTestCases;
	}

	// validate the decrement operator--
	template<size_t nbits, size_t es>
	int VerifyDecrement(bool bReportIndividualTestCases)
	{
		std::vector< bfloat<nbits, es> > set;
		//	GenerateOrderedPositSet(set); // [NaR, -maxpos, ..., -minpos, 0, minpos, ..., maxpos]

		int nrOfFailedTestCases = 0;

		bfloat<nbits, es> p, ref;
		// starting from maxpos iterating to -maxpos, and finally NaR via zero
		for (typename std::vector < bfloat<nbits, es> >::iterator it = set.end() - 1; it != set.begin(); --it) {
			p = *it;
			p--;
			ref = *(it - 1);
			if (p != ref) {
				if (bReportIndividualTestCases) std::cout << " FAIL " << p << " != " << ref << std::endl;
				nrOfFailedTestCases++;
			}
		}

		return nrOfFailedTestCases;
	}

} // namespace sw::universal

