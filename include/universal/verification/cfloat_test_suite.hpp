#pragma once
//  cfloat_test_suite.hpp : verification functions for classic cfloats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <vector>
#include <iostream>
#include <typeinfo>
#include <random>
#include <limits>

#include <universal/math/stub/classify.hpp>
#include <universal/verification/test_reporters.hpp>  // error/success reporting

namespace sw::universal {

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
			fail++;
			if (reportTestCases)	CfloatReportConversionError("FAIL", "=", input, reference, testValue);
		}
		else {
			// if (reportTestCases) CfloatReportConversionSuccess("PASS", "=", input, reference, testValue);
		}
		return fail;
	}


	////////////////////////////////  generate individual test cases //////////////////////// 

	// Generate a conversion test given raw bits and a scale
	template<typename Cfloat, sw::universal::BlockTripleOperator op>
	void GenerateConversionTest(uint64_t rawBits, int scale) {
		using namespace sw::universal;
		Cfloat nut, ref;
		std::cout << type_tag(nut) << '\n';
		constexpr size_t fbits = Cfloat::fbits;
		using bt = typename Cfloat::BlockType;
		blocktriple<fbits, op, bt> b;
		// set the bits and scale
		b.setbits(rawBits);
		b.setscale(scale);
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
		RefType refminpos;
		refminpos.minpos();
		double dminpos = double(refminpos);

		// NUT: number under test
		TestType nut, golden;
		for (size_t i = 0; i < NR_TEST_CASES && i < max_tests; ++i) {
			RefType ref, prev, next;
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
		cfloat<32, 8, uint32_t, true, true, false> ref; // this is a superset of an IEEE-754 float with gradual overflow
		cfloat<nbits, es, BlockType, hasSubnormals, hasSupernormals, isSaturating> nut;

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
			nrOfFailedTests += Compare(refValue, testValue, refValue, reportTestCases);
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
 
		cfloat<64, 11, uint64_t, true, false, false> ref;  // this is an IEEE-754 double
//		cfloat<64, 11, uint64_t, true, false, false> ref;  // this is a superset of an IEEE-754 double with gradual overflow
		cfloat<nbits, es, BlockType, hasSubnormals, hasSupernormals, isSaturating> nut;

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
			nrOfFailedTests += Compare(refValue, testValue, refValue, reportTestCases);
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
		cfloat<nbits, es, BlockType, hasSubnormals, hasSupernormals, isSaturating> nut, result;
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
		cfloat<nbits, es, BlockType, hasSubnormals, hasSupernormals, isSaturating> nut, result;
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
		cfloat<nbits, es, BlockType, hasSubnormals, hasSupernormals, isSaturating> nut, result;
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
	

	/// <summary>
	/// convert a blocktriple to a cfloat
	/// </summary>
	/// <typeparam name="CfloatConfiguration"></typeparam>
	/// <param name="reportTestCases"></param>
	/// <returns></returns>
	template<typename CfloatConfiguration, BlockTripleOperator op>
	int VerifyCfloatFromBlocktripleConversion(bool reportTestCases) {
		using namespace sw::universal;
		constexpr size_t nbits = CfloatConfiguration::nbits;
		constexpr size_t es = CfloatConfiguration::es;
		using bt = typename CfloatConfiguration::BlockType;
		constexpr bool hasSubnormals = CfloatConfiguration::hasSubnormals;
		constexpr bool hasSupernormals = CfloatConfiguration::hasSupernormals;
		constexpr bool isSaturating = CfloatConfiguration::isSaturating;
		constexpr size_t fbits = CfloatConfiguration::fbits;

		int nrOfTestFailures{ 0 };

		cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> a, nut;
		std::cout << dynamic_range(a) << '\n';
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
		BlockTripleConfiguration b;
		std::cout << "\n+-----\n" << type_tag(b) << "  radix point at " << BlockTripleConfiguration::radix << ", smallest scale = " << minposScale << ", largest scale = " << maxposScale << '\n';
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
			size_t NR_VALUES = (1ull << fractionBits);
			b.setscale(scale);
			for (size_t i = 1; i < integerSet; ++i) {  // 01, 10, 11.fffff: state 00 is not part of the encoding as that would represent a denormal
				size_t integerBits = i * NR_VALUES;
				for (size_t f = 0; f < NR_VALUES; ++f) {
					b.setbits(integerBits + f);

					//					std::cout << "blocktriple: " << to_binary(b) << " : " << b << '\n';

					convert(b, nut);

					// get the reference by marshalling the blocktriple value through a double value and assigning it to the cfloat
					a = double(b);
					if (a != nut) {
						//						std::cout << "blocktriple: " << to_binary(b) << " : " << b << " vs " << to_binary(nut) << " : " << nut << '\n';

						if (a.isnan() && b.isnan()) continue;
						if (a.isinf() && b.isinf()) continue;

						++nrOfTestFailures;
						if (reportTestCases) std::cout << "FAIL: " << to_triple(b) << " : " << std::setw(15) << b << " -> " << to_binary(nut) << " != ref " << to_binary(a) << " or " << nut << " != " << a << '\n';
					}
					else {
#ifndef VERBOSE_POSITIVITY
						if (reportTestCases) std::cout << "PASS: " << to_triple(b) << " : " << std::setw(15) << b << " -> " << to_binary(nut) << " == ref " << to_binary(a) << " or " << nut << " == " << a << '\n';
#endif
					}
				}
			}
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
		constexpr size_t NR_VALUES = (1u << nbits);
		cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> a;

		// ADD
		if constexpr (op == BlockTripleOperator::ADD) {
			//constexpr size_t abits = CfloatConfiguration::abits;
			constexpr size_t fbits = CfloatConfiguration::fbits;
			blocktriple<fbits, op, bt> b;   // the size of the blocktriple is configured by the number of fraction bits of the source number system
			for (size_t i = 0; i < NR_VALUES; ++i) {
				a.setbits(i);
				a.normalizeAddition(b);
				if (double(a) != double(b)) {
					if (a.isnan() && b.isnan()) continue;
					if (a.isinf() && b.isinf()) continue;

					++nrOfTestFailures;
					if (reportTestCases) std::cout << "FAIL: " << to_binary(a) << " : " << a << " != " << to_triple(b) << " : " << b << '\n';
				}
				else {
					if (reportTestCases) std::cout << "PASS: " << to_binary(a) << " : " << a << " == " << to_triple(b) << " : " << b << '\n';
				}
			}
		}

		// MUL
		if constexpr (op == BlockTripleOperator::MUL) {
			constexpr size_t fbits = CfloatConfiguration::fbits;
			blocktriple<fbits, op, bt> b;   // the size of the blocktriple is configured by the number of fraction bits of the source number system
			blocktriple<2 * fbits, BlockTripleOperator::REPRESENTATION, bt> ref;
			for (size_t i = 0; i < NR_VALUES; ++i) {
				a.setbits(i);
				a.normalizeMultiplication(b);
				ref = double(b);
				if (double(ref) != double(b)) {
					if (a.isnan() && b.isnan()) continue;
					if (a.isinf() && b.isinf()) continue;

					++nrOfTestFailures;
					if (reportTestCases) std::cout << "FAIL: " << to_binary(a) << " : " << a << " != " << to_triple(b) << " : " << b << '\n';
				}
			}
		}
		return nrOfTestFailures;
	}

	// Generate ordered set in ascending order from [-NaN, -inf, -maxpos, ..., +maxpos, +inf, +NaN] for a particular posit config <nbits, es>
	template<typename TestType>
	void GenerateOrderedCfloatSet(std::vector<TestType>& set) {
		constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
		constexpr size_t es = TestType::es;
		using BlockType = typename TestType::BlockType;
		constexpr bool hasSubnormals = TestType::hasSubnormals;
		constexpr bool hasSupernormals = TestType::hasSupernormals;
		constexpr bool isSaturating = TestType::isSaturating;
		using Cfloat = cfloat<nbits, es, BlockType, hasSubnormals, hasSupernormals, isSaturating>;

		constexpr size_t NR_OF_REALS = (unsigned(1) << nbits);		// don't do this for state spaces larger than 4G

		// generate a set in the order we want increment and decrement to progress
		// 1.11.111   snan
		// 1.11.110   -inf
		// 1.11.101   -maxpos == maxneg
		// ...
		// 1.00.001
		// 1.00.000   -0
		// 0.00.000   +0
		// 0.00.001
		// ...
		// 0.11.101   maxpos
		// 0.11.110   inf
		// 0.11.111   nan

		std::vector< Cfloat > s(NR_OF_REALS);
		Cfloat c; // == TestType but marshalled
		constexpr size_t NEGATIVE_ZERO = (1ull << (nbits - 1)); // pattern 1.00.000
		constexpr size_t MAX_POS = (~0ull >> (64 - nbits + 1)); // pattern 0.11.111
		size_t i = 0;
		for (size_t pattern = NR_OF_REALS - 1; pattern >= NEGATIVE_ZERO ; --pattern) {
			c.setbits(pattern);
			s[i++] = c;
		}
		for (size_t pattern = 0; pattern <= MAX_POS; ++pattern) {
			c.setbits(pattern);
			s[i++] = c;
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

		Cfloat minneg(SpecificValue::minneg);
		Cfloat minpos(SpecificValue::minpos);

		int nrOfFailedTestCases = 0;

		// special cases are transitions to different regimes and special encodings
		if constexpr (hasSubnormals) {
			TestType a(minneg);
			++a;  // we are going to be -0
			if (!a.iszero() && a.isneg()) {
				if (reportTestCases) std::cout << " FAIL " << a << " != -0\n";
				++nrOfFailedTestCases;
			}
			++a; // going from -0 to +0
			if (!a.iszero() && a.ispos()) {
				if (reportTestCases) std::cout << " FAIL " << a << " != +0\n";
				++nrOfFailedTestCases;
			}
			if (++a != minpos) {
				if (reportTestCases) std::cout << " FAIL " << a << " != " << minpos << std::endl;
				++nrOfFailedTestCases;
			}
		}
		else {  // the logic is exactly the same, but the values are very different
			TestType a(minneg);
			if (++a != 0) {
				if (reportTestCases) std::cout << " FAIL " << a << " != 0\n";
				++nrOfFailedTestCases;
			}
			a = 0;
			if (++a != minpos) {
				if (reportTestCases) std::cout << " FAIL " << a << " != " << minpos << std::endl;
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
		GenerateOrderedCfloatSet(set); // [snan, -inf, maxneg, ..., -0, +0, ..., maxpos, +inf, nan]

		int nrOfFailedTestCases = 0;

		Cfloat c, ref; // == TestType but marshalled
		// starting from SNaN iterating from -inf, -maxpos to maxpos, +inf, +nan
		for (typename std::vector < Cfloat >::iterator it = set.begin(); it != set.end() - 1; ++it) {
			c = *it;
			c++; // this will test both postfix and prefix operators
			ref = *(it + 1);
			if (c != ref) {
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
			TestType a(0);
			--a;  // we are going to be -0
			if (!a.iszero() && a.isneg()) {
				if (reportTestCases) std::cout << " FAIL " << a << " != -0\n";
				++nrOfFailedTestCases;
			}
			++a; // going from -0 to +0
			if (!a.iszero() && a.ispos()) {
				if (reportTestCases) std::cout << " FAIL " << a << " != +0\n";
				++nrOfFailedTestCases;
			}
			if (++a != minpos) {
				if (reportTestCases) std::cout << " FAIL " << a << " != " << minpos << std::endl;
				++nrOfFailedTestCases;
			}
		}
		else {  // the logic is exactly the same, but the values are very different
			TestType a(minneg);
			if (++a != 0) {
				if (reportTestCases) std::cout << " FAIL " << a << " != 0\n";
				++nrOfFailedTestCases;
			}
			a = 0;
			if (++a != minpos) {
				if (reportTestCases) std::cout << " FAIL " << a << " != " << minpos << std::endl;
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
	int VerifyCfloatDecrement(bool reportTestCases)
	{
		constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
		constexpr size_t es = TestType::es;
		using BlockType = typename TestType::BlockType;
		constexpr bool hasSubnormals = TestType::hasSubnormals;
		constexpr bool hasSupernormals = TestType::hasSupernormals;
		constexpr bool isSaturating = TestType::isSaturating;
		using Cfloat = sw::universal::cfloat<nbits, es, BlockType, hasSubnormals, hasSupernormals, isSaturating>;
		std::vector< Cfloat > set;
		GenerateOrderedCfloatSet(set); // [snan, -inf, maxneg, ..., -0, +0, ..., maxpos, +inf, nan]

		int nrOfFailedTestCases = 0;

		Cfloat c, ref;
		// starting from +nan, +inf, maxpos, ..., +0, -0, ..., maxneg, -inf, -nan
		for (typename std::vector < Cfloat >::iterator it = set.end() - 1; it != set.begin(); --it) {
			c = *it;
			c--;  // this will test both postfix and prefix operators
			ref = *(it - 1);

			if (c != ref) {
				// std::cout << to_binary(*it) << " : " << to_binary(*(it - 1)) << " : " << to_binary(c) << '\n';
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
		constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
		constexpr size_t es = TestType::es;
		using BlockType = typename TestType::BlockType;
		constexpr bool hasSubnormals = TestType::hasSubnormals;
		constexpr bool hasSupernormals = TestType::hasSupernormals;
		constexpr bool isSaturating = TestType::isSaturating;
		using Cfloat = sw::universal::cfloat<nbits, es, BlockType, hasSubnormals, hasSupernormals, isSaturating>;

		constexpr size_t NR_VALUES = (size_t(1) << nbits);
		int nrOfFailedTests = 0;

		// set the saturation clamps
		// Cfloat maxpos(sw::universal::SpecificValue::maxpos), maxneg(sw::universal::SpecificValue::maxneg);

		double da, db, ref;  // make certain that IEEE doubles are sufficient as reference
		Cfloat a, b, nut, cref;
		for (size_t i = 0; i < NR_VALUES; ++i) {
			a.setbits(i); // number system concept requires a member function setbits()
			da = double(a);
			for (size_t j = 0; j < NR_VALUES; ++j) {
				b.setbits(j);
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
					if (ref == 0 and nut.iszero()) continue; // mismatched is ignored as compiler optimizes away negative zero
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
				else {
					//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "+", a, b, nut, cref);
				}
			}
			if constexpr (NR_VALUES > 256 * 256) {
				if (i % (NR_VALUES / 25) == 0) std::cout << '.';
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

		constexpr size_t NR_VALUES = (size_t(1) << nbits);
		int nrOfFailedTests = 0;

		// set the saturation clamps
		// Cfloat maxpos(sw::universal::SpecificValue::maxpos), maxneg(sw::universal::SpecificValue::maxneg);

		double da, db, ref;  // make certain that IEEE doubles are sufficient as reference
		Cfloat a, b, nut, cref;
		for (size_t i = 0; i < NR_VALUES; ++i) {
			a.setbits(i); // number system concept requires a member function setbits()
			da = double(a);
			for (size_t j = 0; j < NR_VALUES; ++j) {
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
				else {
					//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "-", a, b, nut, cref);
				}
			}
			if constexpr (NR_VALUES > 256 * 256) {
				if (i % (NR_VALUES / 25) == 0) std::cout << '.';
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

		constexpr size_t NR_VALUES = (size_t(1) << nbits);
		int nrOfFailedTests = 0;

		// set the saturation clamps
		// Cfloat maxpos(sw::universal::SpecificValue::maxpos), maxneg(sw::universal::SpecificValue::maxneg);

		double da, db, ref;  // make certain that IEEE doubles are sufficient as reference
		Cfloat a, b, nut, cref;
		for (size_t i = 0; i < NR_VALUES; ++i) {
			a.setbits(i); // number system concept requires a member function setbits()
			da = double(a);
			for (size_t j = 0; j < NR_VALUES; ++j) {
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
					if (a.isinf()) {
						if (b.isinf()) {
							cref.setinf(a.sign() != b.sign());
						}
						else {
							cref.setnan(NAN_TYPE_SIGNALLING);
						}
					}
					else {
						cref.setnan(NAN_TYPE_SIGNALLING);
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
				else {
					if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "*", a, b, nut, cref);
				}
			}
			if constexpr (NR_VALUES > 256 * 256) {
				if (i % (NR_VALUES / 25) == 0) std::cout << '.';
			}
		}
		//		std::cout << std::endl;
		return nrOfFailedTests;
	}
} // namespace sw::universal

