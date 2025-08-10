#pragma once
//  cfloat_test_suite.hpp : verification functions for classic cfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <vector>
#include <iostream>
#include <typeinfo>
#include <random>
#include <limits>

#include <universal/verification/test_reporters.hpp>  // error/success reporting

namespace sw { namespace universal {

	static constexpr size_t COLUMN_WIDTH = 20;

	template<typename SrcType, typename TestType>
	void CfloatReportConversionError(const std::string& test_case, const std::string& op, SrcType input, const TestType& reference, const TestType& result) {
		// constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
		auto old_precision = std::cerr.precision();
		std::cerr << test_case
			<< " " << op << " "
			<< std::setw(COLUMN_WIDTH) << input
			<< " did not convert to "
			<< std::setw(COLUMN_WIDTH) << reference << " instead it yielded  "
			<< std::setw(COLUMN_WIDTH) << result
			<< "  reference " << to_binary(reference) << " vs result " << to_binary(result)
			<< std::setprecision(old_precision)
			<< std::endl;
	}

	template<typename SrcType, typename TestType>
	void CfloatReportConversionSuccess(const std::string& test_case, const std::string& op, SrcType input, const TestType& reference, const TestType& result) {
		constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
		std::cerr << test_case
			<< " " << op << " "
			<< std::setw(COLUMN_WIDTH) << input
			<< " success            "
			<< std::setw(COLUMN_WIDTH) << result << " golden reference is "
			<< std::setw(COLUMN_WIDTH) << reference
			<< "  raw " << std::setw(nbits) << to_binary(result)
			<< std::endl;
	}

	template<typename SrcType, typename TestType>
	int Compare(SrcType input, const TestType& testValue, const TestType& reference, bool reportTestCases) {
		int fail = 0;
		if (testValue != reference) {
			if (testValue.isnan() && reference.isnan()) return 0; // (s)nan != (s)nan, so the regular equivalance test fails
			fail++;
			if (reportTestCases)	CfloatReportConversionError("FAIL", "=", input, reference, testValue);
		}
		else {
			// if (reportTestCases) CfloatReportConversionSuccess("PASS", "=", input, reference, testValue);
		}
		return fail;
	}

	// compare float/double/long double values
	template<typename SrcType, typename TestType>
	int CompareIEEE(SrcType input, const TestType& testValue, const TestType& reference, bool reportTestCases) {
		int fail = 0;
		if (testValue != reference) {
			fail++;
			if (reportTestCases)	CfloatReportConversionError("FAIL", "=", input, reference, testValue);
		}
		else {
			// if (reportTestCases) CfloatReportConversionSuccess("PASS", "=", input, reference, testValue);
		}
		return fail;
	}

	////////////////////////////////  generate individual test cases //////////////////////// 

	// Generate a conversion test given the scale of the number and raw bits of the fraction
	template<typename Cfloat, sw::universal::BlockTripleOperator op>
	void GenerateConversionTest(int scale, uint64_t rawBits) {
		using namespace sw::universal;
		Cfloat nut{}, ref{};
//		std::cout << type_tag(nut) << '\n';
		constexpr size_t fbits = Cfloat::fbits;
		using bt = typename Cfloat::BlockType;
		blocktriple<fbits, op, bt> b;
		// set the scale and fraction bits of the blocktriple
		b.setscale(scale);
		b.setbits(rawBits);
		convert(b, nut);
		float v = float(b);
		ref = v; // set the reference through a conversion value
		std::cout << "blocktriple: " << to_binary(b) << " : " << float(b) << '\n';
		std::cout << "cfloat     : " << to_binary(nut) << " : " << nut << '\n';
		std::cout << "cfloat ref : " << to_binary(ref) << " : " << ref << '\n';

		// range of possible values
		//GenerateTable<Cfloat>(std::cout);
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

  VerifyConversion algorithm: enumerate cfloat<nbits+1, es> and create minus and plus deltas that you a priori know which way they round
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
	/// <param name="reportTestCases">if true print results of each test case. Default is false.</param>
	/// <returns>number of failed test cases</returns>
	template<typename TestType, typename SrcType = double>
	int VerifyCfloatConversion(bool reportTestCases) {
		// we are going to generate a test set that consists of all configs and their midpoints
		// we do this by enumerating a configuration that is 1-bit larger than the test configuration
		// with the extra bit allocated to the fraction.
		// 
		// The sample values of the  larger configuration will be at the mid-point between the smaller 
		// configuration sample values thus creating a full cover test set for value conversions.
		// The precondition for this type of test is that the value conversion, that is,
		// how to go from cfloat bits to IEEE-754 double values, is verified.
		// 
		// To test the rounding logic of the conversion we are going to 
		// generate the three test cases per sample:
		// 1- we'll enumerate the exact value, 
		// 2- a perturbation slightly smaller from the midpoint that will round down, and
		// 3- a perturbation slightly larger that will round up
		// 
		constexpr size_t nbits = TestType::nbits;
		constexpr size_t es = TestType::es;
		using BlockType = typename TestType::BlockType;
		constexpr bool hasSubnormals = TestType::hasSubnormals;
		constexpr bool hasSupernormals = TestType::hasSupernormals;
		constexpr bool isSaturating = TestType::isSaturating;
		using RefType = cfloat<nbits + 1, es, BlockType, hasSubnormals, hasSupernormals, isSaturating>;
		constexpr size_t NR_TEST_CASES = (size_t(1) << (nbits + 1));
		constexpr size_t HALF = (size_t(1) << nbits);

		// For example: 
		// TestType: fixpnt<nbits,rbits,Saturating,uint8_t> needs RefType fixpnt<nbits+1, rbits+1, Saturating,uint8_t>
		// TestType: cfloat<nbits, es, uint8_t> needs RefType cfloat<nbits + 1, es, uint8_t>
		// TestType: posit<nbits, es, uint8_t> needs RefType posit<nbits + 1, es, uint8_t>

		const unsigned max = nbits > 20 ? 20 : nbits + 1;
		size_t max_tests = (size_t(1) << max);
		if (max_tests < NR_TEST_CASES) {
			std::cout << "VerifyConversion " << typeid(TestType).name() << ": NR_TEST_CASES = " << NR_TEST_CASES << " clipped by " << max_tests << std::endl;
		}

		// execute the test
		int nrOfFailedTests = 0;
		RefType refminpos{};
		refminpos.minpos();
		double dminpos = double(refminpos);

		// NUT: number under test
		TestType nut{}, golden{};
		for (size_t i = 0; i < NR_TEST_CASES && i < max_tests; ++i) {
			RefType ref{}, prev{}, next{};
			SrcType testValue{ 0.0 };
			ref.setbits(i);
			SrcType da = SrcType(ref);
			int old = nrOfFailedTests;
			SrcType oneULP = ulp(da);
			if (i % 2) {
				if (i == 1)	{
					// special case of a tie that needs to round to even -> 0
					testValue = da;
					nut = testValue;
					golden = 0.0f;
					nrOfFailedTests += Compare(testValue, nut, golden, reportTestCases);

					// this rounds up 
					testValue = SrcType(da + oneULP);  // the test value between 0 and minpos
					nut = testValue;
					next.setbits(i + 1);
					golden = double(next);
					nrOfFailedTests += Compare(testValue, nut, golden, reportTestCases);
				}
				else if (i == HALF - 3) { // project to +inf
					golden.setinf(false);

					// project to inf
					testValue = SrcType(da - oneULP);
					nut = testValue;
					nrOfFailedTests += Compare(testValue, nut, golden, reportTestCases);

					testValue = SrcType(da);
					nut = testValue;
					nrOfFailedTests += Compare(testValue, nut, golden, reportTestCases);

					// project to inf
					testValue = SrcType(da + oneULP);
					nut = testValue;
					nrOfFailedTests += Compare(testValue, nut, golden, reportTestCases);
#ifdef CHECK_SPECIAL_ENCODING
					std::cout << i << "  " << nrOfFailedTests - old << " : " << testValue << " : " << nut << " : " << to_binary(nut) << '\n';
#endif
				}
				else if (i == HALF - 1) { // encoding of qNaN
					golden.setnan(NAN_TYPE_QUIET);
					testValue = SrcType(da);
					nut = testValue;
					nrOfFailedTests += Compare(testValue, nut, golden, reportTestCases);
#ifdef CHECK_SPECIAL_ENCODING
					std::cout << i << "  " << nrOfFailedTests - old << " : " << testValue << " : " << nut << " : " << to_binary(nut) << '\n';
					std::cout << "quiet      NAN : " << to_binary(testValue) << std::endl;
					std::cout << "quiet NaN mask : " << to_binary(ieee754_parameter<SrcType>::qnanmask, sizeof(testValue)*8) << std::endl;
#endif
				}
				else if (i == HALF + 1) {
					// special case of projecting to -0
					testValue = SrcType(da - oneULP);
					nut = testValue;
					golden = 0.0f; golden = -golden;
					nrOfFailedTests += Compare(testValue, nut, golden, reportTestCases);
				}
				else if (i == NR_TEST_CASES - 3) { // project to -inf
					golden.setinf(true);

					// project to -inf
					testValue = SrcType(da - oneULP);
					nut = testValue;
					nrOfFailedTests += Compare(testValue, nut, golden, reportTestCases);
					
					testValue = SrcType(da);
					nut = testValue;
					nrOfFailedTests += Compare(testValue, nut, golden, reportTestCases);

					// project to -inf
					testValue = SrcType(da + oneULP);
					nut = testValue;
					nrOfFailedTests += Compare(testValue, nut, golden, reportTestCases);
				}
				else if (i == NR_TEST_CASES - 1) { // encoding of SIGNALLING NAN
					golden.setnan(NAN_TYPE_SIGNALLING);
					testValue = SrcType(da);
					nut = testValue;
					nrOfFailedTests += Compare(testValue, nut, golden, reportTestCases);
#ifdef CHECK_SPECIAL_ENCODING
					std::cout << "signalling NAN : " << to_binary(testValue) << std::endl;
					std::cout << "signalNaN mask : " << to_binary(ieee754_parameter<SrcType>::snanmask, sizeof(testValue)*8) << std::endl;
#endif
				}
				else {
					// for odd values of i, we are between sample values of the NUT
					// create the round-up and round-down cases
					// round-down
					testValue = SrcType(da - oneULP);
					nut = testValue;
					prev.setbits(i - 1);
					golden = double(prev);
					nrOfFailedTests += Compare(testValue, nut, golden, reportTestCases);
					
					// round-up
					testValue = SrcType(da + oneULP);
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
					golden.setzero(); // make certain we are +0
					//nrOfFailedTests += Compare(testValue, nut, golden, reportTestCases);
					if (!nut.iszero()) {
						std::cout << "number under test is not zero: " << to_binary(nut) << '\n';
						++nrOfFailedTests;
					}

					// half of next rounds down to 0
					testValue = SrcType(dminpos / 2.0);
					nut = testValue;
					// special handling as optimizer can destroy the sign on 0
					// nrOfFailedTests += Compare(testValue, nut, golden, reportTestCases);
					if (!nut.iszero()) {
						std::cout << "number under test is not zero: " << to_binary(nut) << '\n';
						++nrOfFailedTests;
					}
					
				}
				else if (i == HALF) {
					// ref = -0
					// 0                -> value = 0
					// half of next     -> value = 0
					// special case of assigning to 0

					testValue = da;
					nut = testValue;
					golden.setzero(); golden.setsign(); // make certain we are -0
					// special handling as optimizer can destroy the -0
					// nrOfFailedTests += Compare(testValue, nut, golden, reportTestCases);
					if (!nut.iszero()) {
						std::cout << "number under test is not zero: " << to_binary(nut) << '\n';
						++nrOfFailedTests;
					}

					// half of next rounds down to -0
					testValue = SrcType(-dminpos / 2.0);
					nut = testValue;
					golden.setzero(); golden.setsign(); // make certain we are -0
					// nrOfFailedTests += Compare(testValue, nut, golden, reportTestCases);
					if (!nut.iszero()) {
						std::cout << "number under test is not zero: " << to_binary(nut) << '\n';
						++nrOfFailedTests;
					}
				}
				else if (i == HALF - 4) { // project to inf or saturate to maxpos
					if constexpr (isSaturating) {
						golden.maxpos();
					}
					else {
						golden.setinf(false);
					}

					testValue = SrcType(da - oneULP);
					nut = testValue;
					nrOfFailedTests += Compare(testValue, nut, golden, reportTestCases);

					testValue = SrcType(da + oneULP);
					nut = testValue;
					nrOfFailedTests += Compare(testValue, nut, golden, reportTestCases);
				}
				else if (i == HALF - 2) { // encoding of INF
					golden.setinf(false);
					testValue = SrcType(da);
					nut = testValue;
					nrOfFailedTests += Compare(testValue, nut, golden, reportTestCases);
				}
				else if (i == NR_TEST_CASES - 4) { // project to -inf or saturation to maxneg
					if constexpr (isSaturating) {
						golden.maxneg();
					}
					else {
						golden.setinf(true);
					}

					testValue = SrcType(da - oneULP);
					nut = testValue;
					nrOfFailedTests += Compare(testValue, nut, golden, reportTestCases);

					testValue = SrcType(da + oneULP);
					nut = testValue;
					nrOfFailedTests += Compare(testValue, nut, golden, reportTestCases);
				}
				else if (i == NR_TEST_CASES - 2) { // encoding of -INF
					golden.setinf(true);
					testValue = SrcType(da);
					nut = testValue;
					nrOfFailedTests += Compare(testValue, nut, golden, reportTestCases);
				}
				else {
					// for even values, we are on actual representable values, so we create the round-up and round-down cases
					// round-up
					testValue = SrcType(da - oneULP);
					nut = testValue;
					golden = da;
					nrOfFailedTests += Compare(testValue, nut, golden, reportTestCases);
					
					// round-down
					testValue = SrcType(da + oneULP);
					nut = testValue;
					nrOfFailedTests += Compare(testValue, nut, golden, reportTestCases);
				}
			}
			if (reportTestCases && nrOfFailedTests > old) {
				std::cout << "test case [" << i << "]\n";
				std::cout << "oneULP        : " << to_binary(oneULP, true) << " : " << oneULP << '\n';
				std::cout << "da - oneULP   : " << to_binary(da - oneULP, true) << " : " << da - oneULP << '\n';
				std::cout << "da            : " << to_binary(da, true) << " : " << da << '\n';
				std::cout << "da + oneULP   : " << to_binary(da + oneULP, true) << " : " << da + oneULP << '\n';
			}
		}
		return nrOfFailedTests;
	}

#define CUSTOM_FEEDBACK
	
	// generate random test cases to test conversion from an IEEE-754 float to a cfloat
	template<typename TestType>
	int VerifyFloat2CfloatConversionRnd(bool reportTestCases, size_t nrOfRandoms = 10000) {
		constexpr size_t nbits = TestType::nbits;
		constexpr size_t es = TestType::es;
		using BlockType = typename TestType::BlockType;
		constexpr bool hasSubnormals = TestType::hasSubnormals;
		constexpr bool hasSupernormals = TestType::hasSupernormals;
		constexpr bool isSaturating = TestType::isSaturating;
		using SrcType = float;

//		cfloat<32, 8, uint32_t, true, false, false> ref; // this is an IEEE-754 float
		cfloat<32, 8, uint32_t, true, true, false> ref{};
		                                                // this is a superset of an IEEE-754 float with gradual overflow
		cfloat<nbits, es, BlockType, hasSubnormals, hasSupernormals, isSaturating> nut{};

		if (reportTestCases) { std::cerr << type_tag(nut) << '\n'; }
		// this is too verbose, so I turned it off
		// std::cerr << "                                                     ignoring subnormals for the moment\n";

		// run randoms
		int nrOfFailedTests = 0;
		std::random_device rd;     // get a random seed from the OS entropy device
		std::mt19937_64 eng(rd()); // use the 64-bit Mersenne Twister 19937 generator and seed it with entropy.
		// define the distribution, by default it goes from 0 to MAX(unsigned long long)
		std::uniform_int_distribution<uint32_t> distr;
		for (unsigned i = 1; i < nrOfRandoms; i++) {
			uint32_t rawBits = distr(eng);
			ref.setbits(rawBits);
			SrcType refValue = SrcType(ref);
			nut = refValue;
			SrcType testValue = SrcType(nut);
			if (isdenorm(refValue)) {
				std::cerr << "synthesized a subnormal : " << to_binary(refValue) << " ignoring for the moment\n";
				continue;
			}
			nrOfFailedTests += CompareIEEE(refValue, testValue, refValue, reportTestCases);
#ifdef CUSTOM_FEEDBACK
			if (ref.isnan()) {
				std::cerr << "synthesized a NaN       : " << to_binary(ref) << '\n';
				std::cerr << "nut : " << to_binary(nut) << "\nref : " << to_binary(ref) << '\n';
				std::cerr << "test: " << to_binary(testValue) << "\nref : " << to_binary(refValue) << '\n';
			}
			if (testValue != refValue) { // IEEE rules: this test yields true if both are NaN
				std::cerr << "nut : " << to_binary(nut) << "\nref : " << to_binary(ref) << '\n';
			}
#endif
			if (nrOfFailedTests > 24) {
				std::cerr << "Too many failures, exiting...\n";
				break;
			}
		}
		return nrOfFailedTests;
	}


	// generate random test cases to test conversion from an IEEE-754 double to a cfloat
	template<typename TestType>
	int VerifyDouble2CfloatConversionRnd(bool reportTestCases, size_t nrOfRandoms = 10000) {
		constexpr size_t nbits = TestType::nbits;
		constexpr size_t es = TestType::es;
		using BlockType = typename TestType::BlockType;
		constexpr bool hasSubnormals = TestType::hasSubnormals;
		constexpr bool hasSupernormals = TestType::hasSupernormals;
		constexpr bool isSaturating = TestType::isSaturating;
 
		cfloat<64, 11, uint64_t, true, false, false> ref{};  // this is an IEEE-754 double
//		cfloat<64, 11, uint64_t, true, false, false> ref;  // this is a superset of an IEEE-754 double with gradual overflow
		cfloat<nbits, es, BlockType, hasSubnormals, hasSupernormals, isSaturating> nut{};

		if (reportTestCases) { std::cerr << type_tag(nut) << '\n'; }
		// this is too verbose, so I turned it off
		// std::cerr << "                                                     ignoring subnormals for the moment\n";


		// run randoms
		int nrOfFailedTests = 0;
		std::random_device rd;     // get a random seed from the OS entropy device
		std::mt19937_64 eng(rd()); // use the 64-bit Mersenne Twister 19937 generator and seed it with entropy.
		// define the distribution, by default it goes from 0 to MAX(unsigned long long)
		std::uniform_int_distribution<uint64_t> distr;
		for (unsigned i = 1; i < nrOfRandoms; i++) {
			uint64_t rawBits = distr(eng);
			ref.setbits(rawBits);
			double refValue = double(ref);
			nut = refValue;
			double testValue = double(nut);
			if (isdenorm(refValue)) {
				std::cerr << "rhs is subnormal: " << to_binary(refValue) << " ignoring for the moment\n";
				continue;
			}
			nrOfFailedTests += CompareIEEE(refValue, testValue, refValue, reportTestCases);
#ifdef CUSTOM_FEEDBACK
			if (ref.isnan()) {
				std::cerr << "synthesized a NaN       : " << to_binary(ref) << '\n';
				std::cerr << "nut : " << to_binary(nut) << "\nref : " << to_binary(ref) << '\n';
				std::cerr << "test: " << to_binary(testValue) << "\nref : " << to_binary(refValue) << '\n';
			}
			if (testValue != refValue) { // IEEE rules: this test yields true if both are NaN
				std::cout << "nut : " << to_binary(nut) << "\nref : " << to_binary(ref) << std::endl;
			}
#endif
			if (nrOfFailedTests > 24) {
				std::cerr << "Too many failures, exiting...\n";
				break;
			}
		}
		return nrOfFailedTests;
	}

	// generate IEEE-754 single precision subnormal values
	template<typename BlockType>
	int VerifyIeee754FloatSubnormals(bool reportTestCases) {
		using namespace std;
		using namespace sw::universal;
		constexpr size_t nbits = 32;
		constexpr size_t es = 8;
		constexpr bool hasSubnormals = true;
		constexpr bool hasSupernormals = true;
		constexpr bool isSaturating = false;
		cfloat<nbits, es, BlockType, hasSubnormals, hasSupernormals, isSaturating> nut{}, result{};
		float f{ 0.0f };
		int nrOfFailedTests{ 0 };

		// verify the subnormals
		nut = 0;
		++nut;
		for (size_t i = 0; i < ieee754_parameter<float>::fbits; ++i) {
			f = float(nut);
			result = f;
			if (result != nut) {
				nrOfFailedTests += Compare(f, result, nut, reportTestCases);
			}
			uint64_t fraction = nut.fraction_ull();
			fraction <<= 1;
			nut.setfraction(fraction);
		}
		return nrOfFailedTests;
	}

	// generate IEEE-754 double precision subnormal values
	template<typename BlockType>
	int VerifyIeee754DoubleSubnormals(bool reportTestCases) {
		using namespace std;
		using namespace sw::universal;
		constexpr size_t nbits = 64;
		constexpr size_t es = 11;
		constexpr bool hasSubnormals = true;
		constexpr bool hasSupernormals = true;
		constexpr bool isSaturating = false;
		cfloat<nbits, es, BlockType, hasSubnormals, hasSupernormals, isSaturating> nut{}, result{};
		double d{ 0.0f };
		int nrOfFailedTests{ 0 };

		// verify the subnormals
		nut = 0;
		++nut;
		for (size_t i = 0; i < ieee754_parameter<double>::fbits; ++i) {
			d = double(nut);
			result = d;
			if (result != nut) {
				nrOfFailedTests += Compare(d, result, nut, reportTestCases);
			}
			uint64_t fraction = nut.fraction_ull();
			fraction <<= 1;
			nut.setfraction(fraction);
		}
		return nrOfFailedTests;
	}

#if LONG_DOUBLE_SUPPORT
	// generate IEEE-754 long double precision subnormal values
	template<typename BlockType>
	int VerifyIeee754LongDoubleSubnormals(bool reportTestCases) {
		using namespace std;
		using namespace sw::universal;
		constexpr size_t nbits = 80;
		constexpr size_t es = 15;
		constexpr bool hasSubnormals = true;
		constexpr bool hasSupernormals = true;
		constexpr bool isSaturating = false;
		cfloat<nbits, es, BlockType, hasSubnormals, hasSupernormals, isSaturating> nut{}, result{};
		double d{ 0.0f };
		int nrOfFailedTests{ 0 };

		// long double support tends to be just extended precision support (that implies afbits = 64)
		constexpr size_t fbits = 64;
		if constexpr (ieee754_parameter<long double>::fbits == fbits) {
			// verify the subnormals
			nut = 0;
			++nut;
			for (size_t i = 0; i < fbits; ++i) {
				d = double(nut);
				result = d;
				if (result != nut) {
					nrOfFailedTests += Compare(d, result, nut, reportTestCases);
				}
				blockbinary<fbits, BlockType> fraction{ 0 };
				nut.fraction(fraction);
				fraction <<= 1;
				nut.setfraction(fraction);
			}
		}
		else {
			std::cerr << "long double for this compiler environment is not extended precision\n";
		}

		return nrOfFailedTests;
	}
#endif

	////////////////    cfloat <-> blocktriple

// include the PASS side for reporting
#undef VERBOSE_POSITIVITY

	/// <summary>
	/// verify convertion of a blocktriple into a cfloat
	/// </summary>
	/// <typeparam name="CfloatConfiguration"></typeparam>
	/// <param name="reportTestCases"></param>
	/// <returns></returns>
	template<typename CfloatConfiguration, BlockTripleOperator op>
	int VerifyCfloatFromBlocktripleConversion(bool reportTestCases) {
		using namespace sw::universal;
		constexpr size_t nbits         = CfloatConfiguration::nbits;
		constexpr size_t es            = CfloatConfiguration::es;
		using bt                       = typename CfloatConfiguration::BlockType;
		constexpr bool hasSubnormals   = CfloatConfiguration::hasSubnormals;
		constexpr bool hasSupernormals = CfloatConfiguration::hasSupernormals;
		constexpr bool isSaturating    = CfloatConfiguration::isSaturating;
		constexpr size_t fbits         = CfloatConfiguration::fbits;

		int nrOfTestFailures{ 0 };

		cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> a{}, nut{};
//		std::cout << dynamic_range(a) << '\n';
		int minposScale = minpos_scale(a);
		int maxposScale = maxpos_scale(a);

		/// blocktriple addition and subtraction is done in a 2's complement format 0ii.fffff.
		/// blocktriple multiplication is done in a 1's complement format of ii.fffff
		/// blocktriple division is done in a ?'s complement format of ???????
		/// 
		/// blocktriples can be in overflow configuration, but not in denormalized form
		/// 
		/// BlockTripleOperator::ADD  blocktriple type that comes out of an addition or subtraction operation
		/// BlockTripleOperator::MUL  blocktriple type that comes out of a multiplication operation
		/// BlockTripleOperator::DIV  blocktriple type that comes out of a division operation

		using BlockTripleConfiguration = blocktriple<fbits, op, bt>;
		constexpr size_t rbits = BlockTripleConfiguration::rbits;
		constexpr size_t abits = BlockTripleConfiguration::abits;
		BlockTripleConfiguration b;
		if (reportTestCases) std::cout << "\n+-----\n" << type_tag(b) << "  radix point at " << BlockTripleConfiguration::radix << ", smallest scale = " << minposScale << ", largest scale = " << maxposScale << '\n';
		// test the special cases first
		b.setbits(0x0ull); // propagate the proper radix position to the blocktriple significant
		// the quiet and signalling nan
		for (int sign = 0; sign < 2; ++sign) {
			b.setnan(sign == 1);
			convert(b, nut);
			a = double(b);
			if (a != nut) {
				if (a.isnan() && nut.isnan()) continue; // (s)nan != (s)nan, so the regular equivalance test fails
				++nrOfTestFailures;
				if (reportTestCases) std::cout << "FAIL: " << to_triple(b) << " : " << std::setw(15) << b << " -> " << to_binary(nut) << " != ref " << to_binary(a) << " or " << nut << " != " << a << '\n';
			}
			else {
#ifdef VERBOSE_POSITIVITY
				if (reportTestCases) std::cout << "PASS: " << to_triple(b) << " : " << std::setw(15) << b << " -> " << to_binary(nut) << " == ref " << to_binary(a) << " or " << nut << " == " << a << '\n';
#endif
			}
		}
		// plus and minus infinity
		for (int sign = 0; sign < 2; ++sign) {
			b.setinf(sign == 1);
			convert(b, nut);
			a = double(b);
			if (a != nut) {
				++nrOfTestFailures;
				if (reportTestCases) std::cout << "FAIL: " << to_triple(b) << " : " << std::setw(15) << b << " -> " << to_binary(nut) << " != ref " << to_binary(a) << " or " << nut << " != " << a << '\n';
			}
			else {
#ifdef VERBOSE_POSITIVITY
				if (reportTestCases) std::cout << "PASS: " << to_triple(b) << " : " << std::setw(15) << b << " -> " << to_binary(nut) << " == ref " << to_binary(a) << " or " << nut << " == " << a << '\n';
#endif
			}
		}
		// plus and minus zero
		for (int sign = 0; sign < 2; ++sign) {
			b.setzero(sign == 1);
			convert(b, nut);
			a = double(b); // optimizing compiler does NOT honor sign on 0
			if (a != nut) {
				if (a.iszero() && nut.iszero()) continue;
				++nrOfTestFailures;
				if (reportTestCases) std::cout << "FAIL: " << to_triple(b) << " : " << std::setw(15) << b << " -> " << to_binary(nut) << " != ref " << to_binary(a) << " or " << nut << " != " << a << '\n';
			}
			else {
#ifdef VERBOSE_POSITIVITY
				if (reportTestCases) std::cout << "PASS: " << to_triple(b) << " : " << std::setw(15) << b << " -> " << to_binary(nut) << " == ref " << to_binary(a) << " or " << nut << " == " << a << '\n';
#endif
			}
		}

		// non-special cases of values that need to be mapped to encodings
		b.setnan(false);
		b.setinf(false);
		b.setzero(false);
		for (int sign = 0; sign < 2; ++sign) {
			b.setsign(sign == 1);
			for (int scale = minposScale; scale <= maxposScale; ++scale) {
				// if ADD, pattern is  0ii.fffff, without 000.fffff     // convert does not expect negative 2's complement numbers
				// if MUL, patterns is  ii.fffff, without  00.fffff
				// blocktriples are normal or overflown, so we need to enumerate 2^2 * 2^fbits cases
				size_t fractionBits{ 0 };
				size_t integerSet{ 0 };
				if constexpr (op == BlockTripleOperator::ADD) {
					fractionBits = fbits; // make it explicit for ease of understanding
					integerSet = 4;
				}
				if constexpr (op == BlockTripleOperator::MUL) {
					fractionBits = 2 * fbits;
					integerSet = 4;
				}
				size_t NR_ENCODINGS = (1ull << fractionBits);
				b.setscale(scale);
				for (size_t i = 1; i < integerSet; ++i) {  // 01, 10, 11.fffff: state 00 is not part of the encoding as that would represent a denormal
					size_t integerBits = (i << abits);
					for (size_t f = 0; f < NR_ENCODINGS; ++f) {
						size_t btbits = integerBits | (f << rbits);
						b.setbits(btbits);
//						b.setbits(integerBits + f);

//						std::cout << "blocktriple: " << to_binary(b) << " : " << b << '\n';

						convert(b, nut);

						// get the reference by marshalling the blocktriple value through a double value and assigning it to the cfloat
						a = double(b);
						if (a != nut) {
							//						std::cout << "blocktriple: " << to_binary(b) << " : " << b << " vs " << to_binary(nut) << " : " << nut << '\n';

							if (a.isnan() && b.isnan()) continue;
							if (a.isinf() && b.isinf()) continue;
							if (a.iszero() && b.iszero()) continue; // optimizer adds a sign to 0

							++nrOfTestFailures;
							if (reportTestCases) std::cout << "FAIL: " << to_triple(b) << " : " << std::setw(15) << b << " -> " << to_binary(nut) << " != ref " << to_binary(a) << " or " << nut << " != " << a << '\n';
						}
						else {
#ifdef VERBOSE_POSITIVITY
							if (reportTestCases) std::cout << "PASS: " << to_triple(b) << " : " << std::setw(15) << b << " -> " << to_binary(nut) << " == ref " << to_binary(a) << " or " << nut << " == " << a << '\n';
#endif
						}
					}
				}
			}

		}
		return nrOfTestFailures;
	}

	/// <summary>
	/// verify convertion of a blocktriple into a cfloat
	/// </summary>
	/// <typeparam name="CfloatConfiguration"></typeparam>
	/// <param name="reportTestCases"></param>
	/// <returns></returns>
	template<typename CfloatConfiguration, BlockTripleOperator op>
	int VerifyBigCfloatFromBlocktripleConversion(bool reportTestCases) {
		using namespace sw::universal;
		constexpr size_t nbits         = CfloatConfiguration::nbits;
		constexpr size_t es            = CfloatConfiguration::es;
		constexpr size_t fbits         = CfloatConfiguration::fbits;
		using bt                       = typename CfloatConfiguration::BlockType;
		constexpr bool hasSubnormals   = CfloatConfiguration::hasSubnormals;
		constexpr bool hasSupernormals = CfloatConfiguration::hasSupernormals;
		constexpr bool isSaturating    = CfloatConfiguration::isSaturating;

		using BlockTripleConfiguration = blocktriple<fbits, op, bt>;
//		constexpr size_t bfbits        = BlockTripleConfiguration::bfbits;
//		constexpr size_t rbits         = BlockTripleConfiguration::rbits;
//		constexpr size_t abits         = BlockTripleConfiguration::abits;

		int nrOfTestFailures{ 0 };

		cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> a{}, nut{};
		//		std::cout << dynamic_range(a) << '\n';
		int minposScale = minpos_scale(a);
		int maxposScale = maxpos_scale(a);

		/// blocktriple addition and subtraction is done in a 2's complement format 0ii.fffff.
		/// blocktriple multiplication is done in a 1's complement format of ii.fffff
		/// blocktriple division is done in a ?'s complement format of ???????
		/// 
		/// blocktriples can be in overflow configuration, but not in denormalized form
		/// 
		/// BlockTripleOperator::ADD  blocktriple type that comes out of an addition or subtraction operation
		/// BlockTripleOperator::MUL  blocktriple type that comes out of a multiplication operation
		/// BlockTripleOperator::DIV  blocktriple type that comes out of a division operation
		/// significant blocks are organized like this:
		///   ADD        iii.ffffrrrrrrrrr          3 integer bits, f fraction bits, and 2*fhbits rounding bits
		///   MUL         ii.ffff'ffff              2 integer bits, 2*f fraction bits
		///   DIV         ii.ffff'ffff'ffff'rrrr    2 integer bits, 3*f fraction bits, and r rounding bits

		BlockTripleConfiguration b;
		if (reportTestCases) std::cout << "\n+-----\n" << type_tag(b) << "  radix point at " << BlockTripleConfiguration::radix << ", smallest scale = " << minposScale << ", largest scale = " << maxposScale << '\n';
	
		if constexpr (op == BlockTripleOperator::ADD) {
			// create a specific test value
			BlockTripleConfiguration b;
			b.setsign(false);
			b.setscale(0);

		}
		if constexpr (op == BlockTripleOperator::MUL) {
		}
		if constexpr (op == BlockTripleOperator::DIV) {
		}
		return nrOfTestFailures;
	}

	/// <summary>
/// testing of normalization for different blocktriple operators (ADD, MUL, DIV, SQRT)
/// </summary>
/// <typeparam name="CfloatConfiguration"></typeparam>
/// <param name="reportTestCases"></param>
/// <returns></returns>
	template<typename CfloatConfiguration, BlockTripleOperator op>
	int VerifyCfloatToBlocktripleConversion(bool reportTestCases) {
		using namespace sw::universal;
		constexpr size_t nbits = CfloatConfiguration::nbits;
		constexpr size_t es = CfloatConfiguration::es;
		using bt = typename CfloatConfiguration::BlockType;
		constexpr bool hasSubnormals = CfloatConfiguration::hasSubnormals;
		constexpr bool hasSupernormals = CfloatConfiguration::hasSupernormals;
		constexpr bool isSaturating = CfloatConfiguration::isSaturating;

		int nrOfTestFailures{ 0 };
		constexpr size_t NR_ENCODINGS = (1u << nbits);
		cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> a{};

		// ADD
		if constexpr (op == BlockTripleOperator::ADD) {
			//constexpr size_t abits = CfloatConfiguration::abits;
			constexpr size_t fbits = CfloatConfiguration::fbits;
			blocktriple<fbits, op, bt> b;   // the size of the blocktriple is configured by the number of fraction bits of the source number system
			for (size_t i = 0; i < NR_ENCODINGS; ++i) {
				a.setbits(i);
				a.normalizeAddition(b);
				if (double(a) != double(b)) {
					if (a.isnan() && b.isnan()) continue;
					if (a.isinf() && b.isinf()) continue;

					++nrOfTestFailures;
					if (reportTestCases) std::cout << "FAIL: " << to_binary(a) << " : " << a << " != " << to_triple(b) << " : " << b << '\n';
				}
#ifdef VERBOSE_POSITIVITY
				else {
					if (reportTestCases) std::cout << "PASS: " << to_binary(a) << " : " << a << " == " << to_triple(b) << " : " << b << '\n';
				}
#endif
			}
		}

		// MUL
		if constexpr (op == BlockTripleOperator::MUL) {
			constexpr size_t fbits = CfloatConfiguration::fbits;
			blocktriple<fbits, op, bt> b;   // the size of the blocktriple is configured by the number of fraction bits of the source number system
			blocktriple<2 * fbits, BlockTripleOperator::REP, bt> ref;
			for (size_t i = 0; i < NR_ENCODINGS; ++i) {
				a.setbits(i);
				a.normalizeMultiplication(b);
				ref = double(b);
				if (double(ref) != double(b)) {
//					std::cout << "ref  : " << to_triple(ref) << " : " << ref << '\n';
//					std::cout << "norm : " << to_triple(b) << " : " << b << '\n';
					if (a.isnan() && b.isnan()) continue;
					if (a.isinf() && b.isinf()) continue;
					++nrOfTestFailures;
					if (reportTestCases) std::cout << "FAIL: " << to_binary(a) << " : " << a << " != " << to_triple(b) << " : " << b << '\n';
				}
#ifdef VERBOSE_POSITIVITY
				else {
					if (reportTestCases) std::cout << "PASS: " << to_binary(a) << " : " << a << " == " << to_triple(b) << " : " << b << '\n';
				}
#endif
			}
		}

		// DIV
		if constexpr (op == BlockTripleOperator::DIV) {
			constexpr size_t fbits = CfloatConfiguration::fbits;
			blocktriple<fbits, op, bt> b;   // the size of the blocktriple is configured by the number of fraction bits of the source number system
			blocktriple<2 * fbits, BlockTripleOperator::REP, bt> ref;
			for (size_t i = 0; i < NR_ENCODINGS; ++i) {
				a.setbits(i);
				a.normalizeDivision(b);
				ref = double(b);
				if (double(ref) != double(b)) {
//					std::cout << "ref  : " << to_triple(ref) << " : " << ref << '\n';
//					std::cout << "norm : " << to_triple(b) << " : " << b << '\n';
					if (a.isnan() && b.isnan()) continue;
					if (a.isinf() && b.isinf()) continue;
					++nrOfTestFailures;
					if (reportTestCases) std::cout << "FAIL: " << to_binary(a) << " : " << a << " != " << to_triple(b) << " : " << b << '\n';
				}
#ifdef VERBOSE_POSITIVITY
				else {
					if (reportTestCases) std::cout << "PASS: " << to_binary(a) << " : " << a << " == " << to_triple(b) << " : " << b << '\n';
				}
#endif
			}
		}
		return nrOfTestFailures;
	}

	// Generate ordered set in ascending order from [-NaN, -inf, -maxpos, ..., +maxpos, +inf, +NaN] for a particular cfloat config <nbits, es>
	template<typename TestType>
	void GenerateOrderedCfloatSet(std::vector<TestType>& set) {
		constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
		constexpr size_t es = TestType::es;
		using BlockType = typename TestType::BlockType;
		constexpr bool hasSubnormals = TestType::hasSubnormals;
		constexpr bool hasSupernormals = TestType::hasSupernormals;
		constexpr bool isSaturating = TestType::isSaturating;
		using Cfloat = cfloat<nbits, es, BlockType, hasSubnormals, hasSupernormals, isSaturating>;

		constexpr size_t NR_OF_ENCODINGS = (unsigned(1) << nbits);		// don't do this for state spaces larger than 4G

		// generate a set in the order we want increment and decrement to progress
		// 1.11.111   snan
		// 1.11.110   -inf
		// 1.11.101   -maxpos == maxneg
		// ...
		// 1.01.001
		// 1.01.000
		// 1.00.111   <--- subnormals, which we need to remove if the config doesn't have them
		// ...
		// 1.00.001   minneg
		// 1.00.000   -0      ]
		// 0.00.000   +0      ] we are collapsing -0/+0 as next values from 0 are minpos/minneg
		// 0.00.001   mindenorm, minpos if subnormals
		// ...
		// 0.00.111   <-- subnormals
		// 0.01.000   minpos if no subnormals
		// 0.11.101   maxpos   <--- is maxpos for
		// 0.11.110   inf
		// 0.11.111   nan

		std::vector< Cfloat > s;
		Cfloat c{}; // == TestType but marshalled
		constexpr size_t NEGATIVE_ZERO = (1ull << (nbits - 1)); // pattern 1.00.000
		constexpr size_t QUIET_NAN = (~0ull >> (64 - nbits + 1)); // pattern 0.11.111
		for (size_t pattern = NR_OF_ENCODINGS - 1; pattern > NEGATIVE_ZERO ; --pattern) {  // remove negative zero from the set
			c.setbits(pattern);
//			std::cout << to_binary(pattern, nbits, true) << " : " << to_binary(c, true) << '\n';
			if constexpr (hasSubnormals) {
				// s[i++] = c;
				s.push_back(c);
			}
			else {
				if (!c.isdenormal()) {
					// s[i++] = c;
					s.push_back(c);
				}
				// continue to the next pattern
			}
//			for (auto v : s) std::cout << v << ' '; std::cout << '\n';
		}
		for (size_t pattern = 0; pattern <= QUIET_NAN; ++pattern) {
			c.setbits(pattern);
//			std::cout << to_binary(pattern, nbits, true) << " : " << to_binary(c, true) << '\n';
			if constexpr (hasSubnormals) {
				// s[i++] = c;
				s.push_back(c);
			}
			else {
				if (!c.isdenormal()) {
					// s[i++] = c;
					s.push_back(c);
				}
				// continue to the next pattern
			}
//			for (auto v : s) std::cout << v << ' '; std::cout << '\n';
		}
		set = s;
	}

	// test just the special cases of increment operator: operator++()
	template<typename TestType>
	int VerifyCfloatIncrementSpecialCases(bool reportTestCases) {
		constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
		constexpr size_t es = TestType::es;
		using BlockType = typename TestType::BlockType;
		constexpr bool hasSubnormals = TestType::hasSubnormals;
		constexpr bool hasSupernormals = TestType::hasSupernormals;
		constexpr bool isSaturating = TestType::isSaturating;
		using Cfloat = cfloat<nbits, es, BlockType, hasSubnormals, hasSupernormals, isSaturating>;

		constexpr Cfloat minneg(SpecificValue::minneg);
		constexpr Cfloat minpos(SpecificValue::minpos);

		int nrOfFailedTestCases = 0;

		// special cases are transitions to different regimes and special encodings
		if constexpr (hasSubnormals) {
			TestType a(minneg);
			++a;  // we are going from minneg to be 0
			if (!a.iszero()) {
				if (reportTestCases) std::cout << " FAIL " << to_binary(a) << " : " << a << " !=  0\n";
				++nrOfFailedTestCases;
			}
			++a; // going from 0 to minpos
			if (a != minpos) {
				if (reportTestCases) std::cout << " FAIL " << to_binary(a) << " : " << a << " != " << minpos << std::endl;
				++nrOfFailedTestCases;
			}
		}
		else {  // the logic is exactly the same, but the values are very different
			TestType a(minneg);
			if (++a != 0) {
				if (reportTestCases) std::cout << " FAIL " << to_binary(a) << " : " << a << " != 0\n";
				++nrOfFailedTestCases;
			}
			a = 0;
			if (++a != minpos) {
				if (reportTestCases) std::cout << " FAIL " << to_binary(a) << " : " << a << " != " << minpos << std::endl;
				++nrOfFailedTestCases;
			}
		}
		
		// TODO: implement special cases for supernormals
		if constexpr (hasSupernormals) {
		}
		else {
		}
		
		// TODO: special case of saturing arithmetic: sequences will terminate at maxneg and maxpos
		if constexpr (isSaturating) {

			// Cfloat maxneg(SpecificValue::maxneg);
			// Cfloat maxpos(SpecificValue::maxpos);

		}
		return nrOfFailedTestCases;
	}

	// validate the increment operator++
	template<typename TestType>
	int VerifyCfloatIncrement(bool reportTestCases)
	{
		constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
		constexpr size_t es = TestType::es;
		using BlockType = typename TestType::BlockType;
		constexpr bool hasSubnormals = TestType::hasSubnormals;
		constexpr bool hasSupernormals = TestType::hasSupernormals;
		constexpr bool isSaturating = TestType::isSaturating;
		using Cfloat = cfloat<nbits, es, BlockType, hasSubnormals, hasSupernormals, isSaturating>;

		std::vector< Cfloat > set;
		GenerateOrderedCfloatSet(set); // [snan, -inf, maxneg, ..., {-0 +0}, ..., maxpos, +inf, nan]

		int nrOfFailedTestCases = 0;

		Cfloat c{}, ref{}; // == TestType but marshalled
		// starting from SNaN iterating from -inf, -maxpos to maxpos, +inf, +nan
		for (typename std::vector < Cfloat >::iterator it = set.begin(); it != set.end() - 1; ++it) {
			c = *it;
			c++; // this will test both postfix and prefix operators
			ref = *(it + 1);
//			std::cout << to_binary(*it) << " < " << to_binary(*(it + 1)) << " increment " << to_binary(c) << " : " << c << '\n';
			if (c != ref) {
				if (c.isnan() && ref.isnan()) continue; // nan != nan, so the regular equivalance test fails
				std::cout << to_binary(*it) << " < " << to_binary(*(it + 1)) << " incremented value " << to_binary(c) << '\n';
				if (reportTestCases) std::cout << " FAIL " << c << " != " << ref << std::endl;
				nrOfFailedTestCases++;
			}
		}

		return nrOfFailedTestCases;
	}

	// test just the special cases of decrement operator: operator--() TODO
	template<typename TestType>
	int VerifyCfloatDecrementSpecialCases(bool reportTestCases) {
		constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
		constexpr size_t es = TestType::es;
		using BlockType = typename TestType::BlockType;
		constexpr bool hasSubnormals = TestType::hasSubnormals;
		constexpr bool hasSupernormals = TestType::hasSupernormals;
		constexpr bool isSaturating = TestType::isSaturating;
		using Cfloat = cfloat<nbits, es, BlockType, hasSubnormals, hasSupernormals, isSaturating>;

		Cfloat minneg(SpecificValue::minneg);
		Cfloat minpos(SpecificValue::minpos);

		int nrOfFailedTestCases = 0;

		// special cases are transitions to different regimes and special encodings
		if constexpr (hasSubnormals) {
			TestType a(minpos);
			--a;  // we are going minpos to 0
			if (!a.iszero()) {
				if (reportTestCases) std::cout << " FAIL " << to_binary(a) << " : " << a << " != 0\n";
				++nrOfFailedTestCases;
			}
			// going from 0 to minneg
			if (--a != minneg) {
				if (reportTestCases) std::cout << " FAIL " << a << " != " << minneg << std::endl;
				++nrOfFailedTestCases;
			}
		}
		else {  // the logic is exactly the same, but the values are very different
			TestType a(minpos);
			if (--a != 0) {
				if (reportTestCases) std::cout << " FAIL " << a << " != 0\n";
				++nrOfFailedTestCases;
			}
			if (--a != minneg) {
				if (reportTestCases) std::cout << " FAIL " << a << " != " << minneg << std::endl;
				++nrOfFailedTestCases;
			}
		}

		if constexpr (hasSupernormals) {
		}
		else {
		}

		// special case of saturing arithmetic: sequences will terminate at maxneg and maxpos
		if constexpr (isSaturating) {

			// Cfloat maxneg(SpecificValue::maxneg);
			// Cfloat maxpos(SpecificValue::maxpos);

		}
		return nrOfFailedTestCases;
	}

	// validate the decrement operator--
	template<typename TestType>
	int VerifyCfloatDecrement(bool reportTestCases)	{
		constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
		constexpr size_t es = TestType::es;
		using BlockType = typename TestType::BlockType;
		constexpr bool hasSubnormals = TestType::hasSubnormals;
		constexpr bool hasSupernormals = TestType::hasSupernormals;
		constexpr bool isSaturating = TestType::isSaturating;
		using Cfloat = sw::universal::cfloat<nbits, es, BlockType, hasSubnormals, hasSupernormals, isSaturating>;
		
		std::vector< Cfloat > set;
		GenerateOrderedCfloatSet(set); // [snan, -inf, maxneg, ..., minneg, +0, minpos, ..., maxpos, +inf, qnan]

		/*
		std::cout << "Ordered set of cfloat values\n";
		for (auto v : set) std::cout << v << ' '; std::cout << std::endl;
		*/

		int nrOfFailedTestCases = 0;

		Cfloat c{}, ref{};
		// starting from +nan, +inf, maxpos, ..., +0, minneg, ..., maxneg, -inf, -nan
		for (typename std::vector < Cfloat >::reverse_iterator it = set.rbegin(); it != set.rend() - 1; ++it) {
			c = *it;
			c--;  // this will test both postfix and prefix operators
			ref = *(it + 1);
//			std::cout << to_binary(*it) << " > " << to_binary(ref) << " decrement " << to_binary(c) << " : " << c << '\n';
			if (c != ref) {
				// in the no supernormal case, we are decrementing the pattern, but
				// any supernormal evaluates to nan, and that lands us inside the != check
				// We check explicity below to filter out all these nan cases.
				// To see that pattern decrements, uncomment the following line
				// std::cout << to_binary(*it) << " > " << to_binary(*(it - 1)) << " decremented value " << to_binary(c) << '\n';
				if (c.isnan() && ref.isnan()) continue; // nan != nan, so the regular equivalance test fails
				std::cout << to_binary(*it) << " > " << to_binary(*(it + 1)) << " decremented value " << to_binary(c) << '\n';
				if (reportTestCases) std::cout << " FAIL " << c << " != " << ref << std::endl;
				nrOfFailedTestCases++;
			}
		}

		return nrOfFailedTestCases;
	}

	/// <summary>
	/// Enumerate all addition cases for a number system configuration.
	/// Uses doubles to create a reference to compare to.
	/// </summary>
	/// <typeparam name="TestType">the number system type to verify</typeparam>
	/// <param name="reportTestCases">if yes, report on individual test failures</param>
	/// <returns>nr of failed test cases</returns>
	template<typename TestType>
	int VerifyCfloatAddition(bool reportTestCases) {
		constexpr size_t nbits         = TestType::nbits;  // number system concept requires a static member indicating its size in bits
		constexpr size_t es            = TestType::es;
		using BlockType                = typename TestType::BlockType;
		constexpr bool hasSubnormals   = TestType::hasSubnormals;
		constexpr bool hasSupernormals = TestType::hasSupernormals;
		constexpr bool isSaturating    = TestType::isSaturating;
		using Cfloat = sw::universal::cfloat<nbits, es, BlockType, hasSubnormals, hasSupernormals, isSaturating>;

		constexpr size_t NR_ENCODINGS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;

		// set the saturation clamps
		// Cfloat maxpos(sw::universal::SpecificValue::maxpos), maxneg(sw::universal::SpecificValue::maxneg);

		double da, db, ref;  // make certain that IEEE doubles are sufficient as reference
		Cfloat a{}, b{}, nut{}, cref{};
		for (size_t i = 0; i < NR_ENCODINGS; ++i) {
			a.setbits(i); // number system concept requires a member function setbits()
			if constexpr (hasSubnormals == false) if (a.isdenormal()) continue; // ignore subnormal encodings
			da = double(a);
			for (size_t j = 0; j < NR_ENCODINGS; ++j) {
				b.setbits(j);
				if constexpr (hasSubnormals == false) if (b.isdenormal()) continue; // ignore subnormal encodings
				db = double(b);
				ref = da + db;
#if CFLOAT_THROW_ARITHMETIC_EXCEPTION
				// catching overflow
				try {
					result = a + b;
				}
				catch (...) {
					if (!nut.inrange(ref)) {
						// correctly caught the overflow exception
						continue;
					}
					else {
						nrOfFailedTests++;
					}
				}
#else
				nut = a + b;
				if (a.isnan() || b.isnan()) {
					// nan-type propagates
					// if both are nan then signalling nan wins
					// a        b   =   ref
					// qnan    qnan = qnan
					// qnan     #   = qnan
					// #       qnan = qnan
					// snan     #   = snan
					// #       snan = snan
					// snan    snan = snan
					// snan    qnan = snan
					// qnan    snan = snan
					if (a.isnan(NAN_TYPE_SIGNALLING) || b.isnan(NAN_TYPE_SIGNALLING)) {
						cref.setnan(NAN_TYPE_SIGNALLING);
					}
					else {
						cref.setnan(NAN_TYPE_QUIET);
					}
				}
				else if (a.isinf() || b.isinf()) {
					// a      b  =  ref
					// +inf +inf = +inf
					// +inf -inf = snan
					// -inf +inf = snan
					// -inf -inf = -inf
					if (a.isinf()) {
						if (b.isinf()) {
							if (a.sign() == b.sign()) {
								cref.setinf(a.sign());
							}
							else {
								cref.setnan(NAN_TYPE_SIGNALLING);
							}
						}
						else {
							cref.setinf(a.sign());
						}
					}
					else {
						cref.setinf(b.sign());
					}
				}
				else {
					if (!nut.inrange(ref)) {
						// the result of the addition is outside of the range
						// of the NUT (number system under test)
						if constexpr (isSaturating) {
							if (ref > 0) cref.maxpos(); else cref.maxneg();
						}
						else {
							cref.setinf(ref < 0);
						}
					}
					else {
						cref = ref;
					}
				}

#endif // THROW_ARITHMETIC_EXCEPTION

				if (nut != cref) {
					if (nut.isnan() && cref.isnan()) continue; // (s)nan != (s)nan, so the regular equivalance test fails
					if (ref == 0 && nut.iszero()) continue;    // mismatched is ignored as compiler optimizes away negative zero
					nrOfFailedTests++;
					if (reportTestCases)	ReportBinaryArithmeticError("FAIL", "+", a, b, nut, cref);
#ifdef TRACE_ROUNDING
					blocktriple<TestType::abits, BlockType> bta, btb, btsum;
					// transform the inputs into (sign,scale,significant) 
					// triples of the correct width
					a.normalizeAddition(bta);
					b.normalizeAddition(btb);
					btsum.add(bta, btb); 
					auto oldPrecision = std::cout.precision(15);
					std::cout << i << ',' << j << '\n';
					std::cout 
						<< "a    " << to_binary(a) << ' ' << std::setw(20) << a << ' ' << to_binary(float(a)) << ' ' << to_triple(bta) << '\n'
						<< "b    " << to_binary(b) << ' ' << std::setw(20) << b << ' ' << to_binary(float(b)) << ' ' << to_triple(btb) << '\n'
						<< "nut  " << to_binary(nut) << ' ' << std::setw(20) << nut << ' ' << to_binary(float(nut)) << ' ' << to_triple(btsum) << '\n'
						<< "cref " << to_binary(cref) << ' ' << std::setw(20) << cref << ' ' << to_binary(float(cref)) << ' ' << to_triple(cref) << '\n';
					std::cout.precision(oldPrecision);

					if (nrOfFailedTests > 9) return nrOfFailedTests;
#endif
				}
#ifdef VERBOSE_POSITIVITY
				else {
					if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "+", a, b, nut, cref);
				}
#endif
			}
			if constexpr (NR_ENCODINGS > 256 * 256) {
				if (i % (NR_ENCODINGS / 25) == 0) std::cout << '.';
			}
		}
//		std::cout << std::endl;
		return nrOfFailedTests;
	}

	/// <summary>
	/// Enumerate all subtraction cases for a number system configuration.
	/// Uses doubles to create a reference to compare to.
	/// </summary>
	/// <typeparam name="TestType">the number system type to verify</typeparam>
	/// <param name="reportTestCases">if yes, report on individual test failures</param>
	/// <returns>nr of failed test cases</returns>
	template<typename TestType>
	int VerifyCfloatSubtraction(bool reportTestCases) {
		constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
		constexpr size_t es = TestType::es;
		using BlockType = typename TestType::BlockType;
		constexpr bool hasSubnormals = TestType::hasSubnormals;
		constexpr bool hasSupernormals = TestType::hasSupernormals;
		constexpr bool isSaturating = TestType::isSaturating;
		using Cfloat = sw::universal::cfloat<nbits, es, BlockType, hasSubnormals, hasSupernormals, isSaturating>;

		constexpr size_t NR_ENCODINGS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;

		// set the saturation clamps
		// Cfloat maxpos(sw::universal::SpecificValue::maxpos), maxneg(sw::universal::SpecificValue::maxneg);

		double da, db, ref;  // make certain that IEEE doubles are sufficient as reference
		Cfloat a{}, b{}, nut{}, cref{};
		for (size_t i = 0; i < NR_ENCODINGS; ++i) {
			a.setbits(i); // number system concept requires a member function setbits()
			da = double(a);
			for (size_t j = 0; j < NR_ENCODINGS; ++j) {
				b.setbits(j);
				db = double(b);
				ref = da - db;
#if CFLOAT_THROW_ARITHMETIC_EXCEPTION
				// catching overflow
				try {
					result = a - b;
				}
				catch (...) {
					if (!nut.inrange(ref)) {
						// correctly caught the overflow exception
						continue;
					}
					else {
						nrOfFailedTests++;
					}
				}

#else
				nut = a - b;
				if (a.isnan() || b.isnan()) {
					// nan-type propagates
					// if both are nan then signalling nan wins
					// a        b   =   ref
					// qnan    qnan = qnan
					// qnan     #   = qnan
					// #       qnan = qnan
					// snan     #   = snan
					// #       snan = snan
					// snan    snan = snan
					// snan    qnan = snan
					// qnan    snan = snan
					if (a.isnan(NAN_TYPE_SIGNALLING) || b.isnan(NAN_TYPE_SIGNALLING)) {
						cref.setnan(NAN_TYPE_SIGNALLING);
					}
					else {
						cref.setnan(NAN_TYPE_QUIET);
					}
				}
				else if (a.isinf() || b.isinf()) {
					// a      b  =  ref
					// +inf +inf = snan
					// +inf -inf = +inf
					// -inf +inf = -inf
					// -inf -inf = snan
					if (a.isinf()) {
						if (b.isinf()) {
							if (a.sign() != b.sign()) {
								cref.setinf(a.sign());
							}
							else {
								cref.setnan(NAN_TYPE_SIGNALLING);
							}
						}
						else {
							cref.setinf(a.sign());
						}
					}
					else {
						cref.setinf(!b.sign());
					}
				}
				else {
					if (!nut.inrange(ref)) {
						// the result of the subtraction is outside of the range
						// of the NUT (number system under test)
						if constexpr (isSaturating) {
							if (ref > 0) cref.maxpos(); else cref.maxneg();
						}
						else {
							cref.setinf(ref < 0);
						}
					}
					else {
						cref = ref;
					}
				}

#endif // THROW_ARITHMETIC_EXCEPTION

				if (nut != cref) {
					if (nut.isnan() && cref.isnan()) continue; // (s)nan != (s)nan, so the regular equivalance test fails
					if (ref == 0 and nut.iszero()) continue; // mismatched is ignored as compiler optimizes away negative zero
					nrOfFailedTests++;
					if (reportTestCases)	ReportBinaryArithmeticError("FAIL", "-", a, b, nut, cref);
#ifdef TRACE_ROUNDING
					blocktriple<TestType::abits, BlockType> bta, btb, btsum;
					// transform the inputs into (sign,scale,significant) 
					// triples of the correct width
					a.normalizeAddition(bta);
					b.normalizeAddition(btb);
					btsum.add(bta, btb);
					auto oldPrecision = std::cout.precision(15);
					std::cout << i << ',' << j << '\n';
					std::cout
						<< "a    " << to_binary(a) << ' ' << std::setw(20) << a << ' ' << to_binary(float(a)) << ' ' << to_triple(bta) << '\n'
						<< "b    " << to_binary(b) << ' ' << std::setw(20) << b << ' ' << to_binary(float(b)) << ' ' << to_triple(btb) << '\n'
						<< "nut  " << to_binary(nut) << ' ' << std::setw(20) << nut << ' ' << to_binary(float(nut)) << ' ' << to_triple(btsum) << '\n'
						<< "cref " << to_binary(cref) << ' ' << std::setw(20) << cref << ' ' << to_binary(float(cref)) << ' ' << to_triple(cref) << '\n';
					std::cout.precision(oldPrecision);

					if (nrOfFailedTests > 9) return nrOfFailedTests;
#endif
				}
#ifdef VERBOSE_POSITIVITY
				else {
					if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "-", a, b, nut, cref);
				}
#endif
			}
			if constexpr (NR_ENCODINGS > 256 * 256) {
				if (i % (NR_ENCODINGS / 25) == 0) std::cout << '.';
			}
		}
		//		std::cout << std::endl;
		return nrOfFailedTests;
	}

	/// <summary>
	/// Enumerate all multiplication cases for a number system configuration.
	/// Uses doubles to create a reference to compare to.
	/// </summary>
	/// <typeparam name="TestType">the number system type to verify</typeparam>
	/// <param name="reportTestCases">if yes, report on individual test failures</param>
	/// <returns>nr of failed test cases</returns>
	template<typename TestType>
	int VerifyCfloatMultiplication(bool reportTestCases) {
		constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
		constexpr size_t es = TestType::es;
		using BlockType = typename TestType::BlockType;
		constexpr bool hasSubnormals = TestType::hasSubnormals;
		constexpr bool hasSupernormals = TestType::hasSupernormals;
		constexpr bool isSaturating = TestType::isSaturating;
		using Cfloat = sw::universal::cfloat<nbits, es, BlockType, hasSubnormals, hasSupernormals, isSaturating>;

		constexpr size_t NR_ENCODINGS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;

		// set the saturation clamps
		// Cfloat maxpos(sw::universal::SpecificValue::maxpos), maxneg(sw::universal::SpecificValue::maxneg);

		double da, db, ref;  // make certain that IEEE doubles are sufficient as reference
		Cfloat a{}, b{}, nut{}, cref{};
		for (size_t i = 0; i < NR_ENCODINGS; ++i) {
			a.setbits(i); // number system concept requires a member function setbits()
			da = double(a);
			for (size_t j = 0; j < NR_ENCODINGS; ++j) {
				b.setbits(j);
				db = double(b);
				ref = da * db;
#if CFLOAT_THROW_ARITHMETIC_EXCEPTION
				// catching overflow
				try {
					result = a * b;
				}
				catch (...) {
					if (!nut.inrange(ref)) {
						// correctly caught the overflow exception
						continue;
					}
					else {
						nrOfFailedTests++;
					}
				}

#else
				nut = a * b;
				if (a.isnan() || b.isnan()) {
					// nan-type propagates
					if (a.isnan(NAN_TYPE_SIGNALLING) || b.isnan(NAN_TYPE_SIGNALLING)) {
						cref.setnan(NAN_TYPE_SIGNALLING);
					}
					else {
						cref.setnan(NAN_TYPE_QUIET);
					}
				}
				else if (a.isinf() || b.isinf()) {
					// a      b  =  ref
					// +inf +inf = +inf
					// +inf -inf = -inf
					// -inf +inf = -inf
					// -inf -inf = +inf
					//  0   +inf = snan
					// +inf  0   = snan
					if (a.isinf()) {
						if (b.iszero()) {
							cref.setnan(NAN_TYPE_QUIET);
						}
						else {
							cref.setinf(a.sign() != b.sign());
						}
					}
					else {
						if (a.iszero()) {
							cref.setnan(NAN_TYPE_QUIET);
						}
						else {
							cref.setinf(a.sign() != b.sign());
						}
					}
				}
				else {
					if (!nut.inrange(ref)) {
						// the result of the multiplication is outside of the range
						// of the NUT (number system under test)
						if constexpr (isSaturating) {
							if (ref > 0) cref.maxpos(); else cref.maxneg();
						}
						else {
							cref.setinf(ref < 0);
						}
					}
					else {
						cref = ref;
					}
				}

#endif // THROW_ARITHMETIC_EXCEPTION

				if (nut != cref) {
					if (nut.isnan() && cref.isnan()) continue; // (s)nan != (s)nan, so the regular equivalance test fails
					if (ref == 0 and nut.iszero()) continue; // mismatched is ignored as compiler optimizes away negative zero
					nrOfFailedTests++;
					if (reportTestCases)	ReportBinaryArithmeticError("FAIL", "*", a, b, nut, cref);
#ifdef TRACE_ROUNDING
					blocktriple<TestType::abits, BlockType> bta, btb, btprod;
					// transform the inputs into (sign,scale,significant) 
					// triples of the correct width
					a.normalizeAddition(bta);
					b.normalizeAddition(btb);
					btprod.mul(bta, btb);
					auto oldPrecision = std::cout.precision(15);
					std::cout << i << ',' << j << '\n';
					std::cout
						<< "a    " << to_binary(a) << ' ' << std::setw(20) << a << ' ' << to_binary(float(a)) << ' ' << to_triple(bta) << '\n'
						<< "b    " << to_binary(b) << ' ' << std::setw(20) << b << ' ' << to_binary(float(b)) << ' ' << to_triple(btb) << '\n'
						<< "nut  " << to_binary(nut) << ' ' << std::setw(20) << nut << ' ' << to_binary(float(nut)) << ' ' << to_triple(btprod) << '\n'
						<< "cref " << to_binary(cref) << ' ' << std::setw(20) << cref << ' ' << to_binary(float(cref)) << ' ' << to_triple(cref) << '\n';
					std::cout.precision(oldPrecision);

					if (nrOfFailedTests > 9) return nrOfFailedTests;
#endif
				}
#ifdef VERBOSE_POSITIVITY
				else {
					if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "*", a, b, nut, cref);
				}
#endif
			}
			if constexpr (NR_ENCODINGS > 256 * 256) {
				if (i % (NR_ENCODINGS / 25) == 0) std::cout << '.';
			}
		}
		//		std::cout << std::endl;
		return nrOfFailedTests;
	}

// optimizing compilers manipulate NaN(ind) and the sign of infinite on a division by zero
// when defined, this compiler guard adds a filter to the CFLOAT division test regression
// to filter out these discrepancies. In debug builds, the compiler is compliant and you
// can undefine this guard and add the test comparisons for divide by zero to catch any
// errors that the implementation might have.
#define FILTER_OUT_DIVIDE_BY_ZERO

	/// <summary>
	/// Enumerate all division cases for a cfloat configuration.
	/// Uses doubles to create a reference to compare to.
	/// </summary>
	/// <typeparam name="TestType">the number system type to verify</typeparam>
	/// <param name="reportTestCases">if yes, report on individual test failures</param>
	/// <returns>nr of failed test cases</returns>
	template<typename TestType>
	int VerifyCfloatDivision(bool reportTestCases) {
		constexpr size_t nbits         = TestType::nbits;  // number system concept requires a static member indicating its size in bits
		constexpr size_t es            = TestType::es;
		using BlockType                = typename TestType::BlockType;
		constexpr bool hasSubnormals   = TestType::hasSubnormals;
		constexpr bool hasSupernormals = TestType::hasSupernormals;
		constexpr bool isSaturating    = TestType::isSaturating;
		using Cfloat = sw::universal::cfloat<nbits, es, BlockType, hasSubnormals, hasSupernormals, isSaturating>;

		constexpr size_t NR_ENCODINGS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;

		// set the saturation clamps
		// Cfloat maxpos(sw::universal::SpecificValue::maxpos), maxneg(sw::universal::SpecificValue::maxneg);

		double da, db, ref;  // make certain that IEEE doubles are sufficient as reference
		Cfloat a{}, b{}, nut{}, cref{};
		for (size_t i = 0; i < NR_ENCODINGS; ++i) {
			a.setbits(i); // number system concept requires a member function setbits()
			da = double(a);
			for (size_t j = 0; j < NR_ENCODINGS; ++j) {
				b.setbits(j);
				db = double(b);
				ref = da / db;
#if CFLOAT_THROW_ARITHMETIC_EXCEPTION
				// catching overflow
				try {
					nut = a / b;
				}
				catch (...) {
					if (!nut.inrange(ref)) {
						// correctly caught the overflow exception
						continue;
					}
					else {
						nrOfFailedTests++;
					}
				}

#else
				nut = a / b;
				bool resultSign = a.sign() != b.sign();
				if (a.isnan() || b.isnan()) {
					// nan-type propagates
					if (a.isnan(NAN_TYPE_SIGNALLING) || b.isnan(NAN_TYPE_SIGNALLING)) {
						cref.setnan(NAN_TYPE_SIGNALLING);
					}
					else {
						cref.setnan(NAN_TYPE_QUIET);
					}
				}
				else if (a.isinf() || b.isinf()) {
					//     a /   b  =  ref
					//     0 /  inf =  0 : 0b0.00000000.00000000000000000000000
					//	   0 / -inf = -0 : 0b1.00000000.00000000000000000000000
					//	   1 /  inf =  0 : 0b0.00000000.00000000000000000000000
					//	   1 / -inf = -0 : 0b1.00000000.00000000000000000000000
					//	 inf /    0 =  inf : 0b0.11111111.00000000000000000000000
					//	 inf /   -0 = -inf : 0b1.11111111.00000000000000000000000
					//	-inf /    0 = -inf : 0b1.11111111.00000000000000000000000
					//	-inf /   -0 =  inf : 0b0.11111111.00000000000000000000000
					//	 inf /  inf = -nan(ind) : 0b1.11111111.10000000000000000000000
					//	 inf / -inf = -nan(ind) : 0b1.11111111.10000000000000000000000
					//	-inf /  inf = -nan(ind) : 0b1.11111111.10000000000000000000000
					//	-inf / -inf = -nan(ind) : 0b1.11111111.10000000000000000000000
					if (a.isinf()) {
						if (b.isinf()) {
							cref.setnan(NAN_TYPE_QUIET);
							cref.setsign(false);  // MSVC NaN/indeterminate
						}
						else {
							cref.setinf(resultSign);
						}
					}
					else {
						cref.setzero();
						cref.setsign(resultSign);
					}
				}
				else {
					if (!nut.inrange(ref)) {
						// the result of the division is outside of the range
						// of the NUT (number system under test)
						if constexpr (isSaturating) {
							if (ref > 0) cref.maxpos(); else cref.maxneg();
						}
						else {
							cref.setinf(ref < 0);
						}
					}
					else {
						cref = ref;
					}
				}

#endif // CFLOAT_THROW_ARITHMETIC_EXCEPTION

				if (nut != cref) {
					if (nut.isnan() && cref.isnan()) continue; // (s)nan != (s)nan, so the regular equivalance test fails
					if (ref == 0 and nut.iszero()) continue; // mismatched is ignored as compiler optimizes away negative zero
#ifdef FILTER_OUT_DIVIDE_BY_ZERO
					if (b.iszero()) continue; // optimization alters nan(ind) and +-inf
#endif
					nrOfFailedTests++;
					if (reportTestCases)	ReportBinaryArithmeticError("FAIL", "/", a, b, nut, cref);
#ifdef TRACE_ROUNDING
					blocktriple<TestType::abits, BlockType> bta, btb, btprod;
					// transform the inputs into (sign,scale,significant) 
					// triples of the correct width
					a.normalizeAddition(bta);
					b.normalizeAddition(btb);
					btprod.div(bta, btb);
					auto oldPrecision = std::cout.precision(15);
					std::cout << i << ',' << j << '\n';
					std::cout
						<< "a    " << to_binary(a) << ' ' << std::setw(20) << a << ' ' << to_binary(float(a)) << ' ' << to_triple(bta) << '\n'
						<< "b    " << to_binary(b) << ' ' << std::setw(20) << b << ' ' << to_binary(float(b)) << ' ' << to_triple(btb) << '\n'
						<< "nut  " << to_binary(nut) << ' ' << std::setw(20) << nut << ' ' << to_binary(float(nut)) << ' ' << to_triple(btprod) << '\n'
						<< "cref " << to_binary(cref) << ' ' << std::setw(20) << cref << ' ' << to_binary(float(cref)) << ' ' << to_triple(cref) << '\n';
					std::cout.precision(oldPrecision);

					if (nrOfFailedTests > 9) return nrOfFailedTests;
#endif
				}
#ifdef VERBOSE_POSITIVITY
				else {
					if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "/", a, b, nut, cref);
				}
#endif
			}
			if constexpr (NR_ENCODINGS > 256 * 256) {
				if (i % (NR_ENCODINGS / 25) == 0) std::cout << '.';
			}
		}
		//		std::cout << std::endl;
		return nrOfFailedTests;
	}

	/// <summary>
	/// Enumerate all square root cases for a cfloat configuration.
	/// Uses doubles to create a reference to verify against.
	/// </summary>
	/// <param name="reportTestCases"></param>
	/// <returns></returns>
	template<typename TestType>
	int VerifyCfloatSqrt(bool reportTestCases) {
		constexpr size_t nbits         = TestType::nbits;  // number system concept requires a static member indicating its size in bits
		constexpr size_t es            = TestType::es;
		using BlockType                = typename TestType::BlockType;
		constexpr bool hasSubnormals   = TestType::hasSubnormals;
		constexpr bool hasSupernormals = TestType::hasSupernormals;
		constexpr bool isSaturating    = TestType::isSaturating;
		using Cfloat = sw::universal::cfloat<nbits, es, BlockType, hasSubnormals, hasSupernormals, isSaturating>;

		constexpr unsigned NR_TEST_CASES = (unsigned(1) << (nbits-1)); // remove the negative values from the test
		int nrOfFailedTests = 0;

		for (unsigned i = 1; i < NR_TEST_CASES; i++) {
			Cfloat ca, csqrt, cref;
			ca.setbits(i);
			csqrt = sw::universal::sqrt(ca);
			// generate reference
			double da = double(ca);
			cref = std::sqrt(da);
			if (csqrt != cref) {
				if (csqrt.isnan() && cref.isnan()) continue;
				if (csqrt.iszero() && cref.iszero()) continue;
				nrOfFailedTests++;
				std::cout << csqrt << " != " << cref << std::endl;
				if (reportTestCases)	ReportUnaryArithmeticError("FAIL", "sqrt", ca, cref, csqrt);
				if (nrOfFailedTests > 24) return nrOfFailedTests;
			}
			else {
				//if (reportTestCases) ReportUnaryArithmeticSuccess("PASS", "sqrt", ca, cref, csqrt);
			}
		}
		return nrOfFailedTests;
	}

}} // namespace sw::universal

