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
	int Compare(SrcType input, const TestType& testValue, const TestType& reference, bool bReportIndividualTestCases) {
		int fail = 0;
		if (testValue != reference) {
			fail++;
			if (bReportIndividualTestCases)	CfloatReportConversionError("FAIL", "=", input, reference, testValue);
		}
		else {
			// if (bReportIndividualTestCases) CfloatReportConversionSuccess("PASS", "=", input, reference, testValue);
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
	/// <param name="bReportIndividualTestCases">if true print results of each test case. Default is false.</param>
	/// <returns>number of failed test cases</returns>
	template<typename TestType, typename SrcType = double>
	int VerifyCfloatConversion(bool bReportIndividualTestCases) {
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
					nrOfFailedTests += Compare(testValue, nut, golden, bReportIndividualTestCases);

					// this rounds up 
					testValue = SrcType(da + oneULP);  // the test value between 0 and minpos
					nut = testValue;
					next.setbits(i + 1);
					golden = double(next);
					nrOfFailedTests += Compare(testValue, nut, golden, bReportIndividualTestCases);
				}
				else if (i == HALF - 3) { // project to +inf
					golden.setinf(false);

					// project to inf
					testValue = SrcType(da - oneULP);
					nut = testValue;
					nrOfFailedTests += Compare(testValue, nut, golden, bReportIndividualTestCases);

					testValue = SrcType(da);
					nut = testValue;
					nrOfFailedTests += Compare(testValue, nut, golden, bReportIndividualTestCases);

					// project to inf
					testValue = SrcType(da + oneULP);
					nut = testValue;
					nrOfFailedTests += Compare(testValue, nut, golden, bReportIndividualTestCases);
					std::cout << i << "  " << nrOfFailedTests - old << " : " << testValue << " : " << nut << " : " << to_binary(nut) << '\n';

				}
				else if (i == HALF - 1) { // encoding of qNaN
					golden.setnan(NAN_TYPE_QUIET);
					testValue = SrcType(da);
					nut = testValue;
					std::cout << i << "  " << nrOfFailedTests - old << " : " << testValue << " : " << nut << " : " << to_binary(nut) << '\n';

					nrOfFailedTests += Compare(testValue, nut, golden, bReportIndividualTestCases);
					std::cout << "quiet      NAN : " << to_binary(testValue) << std::endl;
					std::cout << "quiet NaN mask : " << to_binary(ieee754_parameter<SrcType>::qnanmask, sizeof(testValue)*8) << std::endl;
				}
				else if (i == HALF + 1) {
					// special case of projecting to -0
					testValue = SrcType(da - oneULP);
					nut = testValue;
					golden = 0.0f; golden = -golden;
					nrOfFailedTests += Compare(testValue, nut, golden, bReportIndividualTestCases);
				}
				else if (i == NR_TEST_CASES - 3) { // project to -inf
					golden.setinf(true);

					// project to -inf
					testValue = SrcType(da - oneULP);
					nut = testValue;
					nrOfFailedTests += Compare(testValue, nut, golden, bReportIndividualTestCases);
					
					testValue = SrcType(da);
					nut = testValue;
					nrOfFailedTests += Compare(testValue, nut, golden, bReportIndividualTestCases);

					// project to -inf
					testValue = SrcType(da + oneULP);
					nut = testValue;
					nrOfFailedTests += Compare(testValue, nut, golden, bReportIndividualTestCases);
				}
				else if (i == NR_TEST_CASES - 1) { // encoding of SIGNALLING NAN
					golden.setnan(NAN_TYPE_SIGNALLING);
					testValue = SrcType(da);
					nut = testValue;
					nrOfFailedTests += Compare(testValue, nut, golden, bReportIndividualTestCases);
					std::cout << "signalling NAN : " << to_binary(testValue) << std::endl;
					std::cout << "signalNaN mask : " << to_binary(ieee754_parameter<SrcType>::snanmask, sizeof(testValue)*8) << std::endl;
				}
				else {
					// for odd values of i, we are between sample values of the NUT
					// create the round-up and round-down cases
					// round-down
					testValue = SrcType(da - oneULP);
					nut = testValue;
					prev.setbits(i - 1);
					golden = double(prev);
					nrOfFailedTests += Compare(testValue, nut, golden, bReportIndividualTestCases);
					
					// round-up
					testValue = SrcType(da + oneULP);
					nut = testValue;
					next.setbits(i + 1);
					golden = double(next);
					nrOfFailedTests += Compare(testValue, nut, golden, bReportIndividualTestCases);
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
					//nrOfFailedTests += Compare(testValue, nut, golden, bReportIndividualTestCases);
					if (!nut.iszero()) {
						std::cout << "number under test is not zero: " << to_binary(nut) << '\n';
						++nrOfFailedTests;
					}

					// half of next rounds down to 0
					testValue = SrcType(dminpos / 2.0);
					nut = testValue;
					// special handling as optimizer can destroy the sign on 0
					// nrOfFailedTests += Compare(testValue, nut, golden, bReportIndividualTestCases);
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
					// nrOfFailedTests += Compare(testValue, nut, golden, bReportIndividualTestCases);
					if (!nut.iszero()) {
						std::cout << "number under test is not zero: " << to_binary(nut) << '\n';
						++nrOfFailedTests;
					}

					// half of next rounds down to -0
					testValue = SrcType(-dminpos / 2.0);
					nut = testValue;
					golden.setzero(); golden.setsign(); // make certain we are -0
					// nrOfFailedTests += Compare(testValue, nut, golden, bReportIndividualTestCases);
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
					nrOfFailedTests += Compare(testValue, nut, golden, bReportIndividualTestCases);

					testValue = SrcType(da + oneULP);
					nut = testValue;
					nrOfFailedTests += Compare(testValue, nut, golden, bReportIndividualTestCases);
				}
				else if (i == HALF - 2) { // encoding of INF
					golden.setinf(false);
					testValue = SrcType(da);
					nut = testValue;
					nrOfFailedTests += Compare(testValue, nut, golden, bReportIndividualTestCases);
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
					nrOfFailedTests += Compare(testValue, nut, golden, bReportIndividualTestCases);

					testValue = SrcType(da + oneULP);
					nut = testValue;
					nrOfFailedTests += Compare(testValue, nut, golden, bReportIndividualTestCases);
				}
				else if (i == NR_TEST_CASES - 2) { // encoding of -INF
					golden.setinf(true);
					testValue = SrcType(da);
					nut = testValue;
					nrOfFailedTests += Compare(testValue, nut, golden, bReportIndividualTestCases);
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
				std::cout << "test case [" << i << "]\n";
				std::cout << "oneULP        : " << to_binary(oneULP, true) << " : " << oneULP << '\n';
				std::cout << "da - oneULP   : " << to_binary(da - oneULP, true) << " : " << da - oneULP << '\n';
				std::cout << "da            : " << to_binary(da, true) << " : " << da << '\n';
				std::cout << "da + oneULP   : " << to_binary(da + oneULP, true) << " : " << da + oneULP << '\n';
			}
		}
		return nrOfFailedTests;
	}

	// generate random test cases to test conversion from an IEEE-754 float to a cfloat
	template<typename TestType>
	int VerifyFloat2CfloatConversionRnd(bool bReportIndividualTestCases, size_t nrOfRandoms = 10000) {
		constexpr size_t nbits = TestType::nbits;
		constexpr size_t es = TestType::es;
		using BlockType = typename TestType::BlockType;
		constexpr bool hasSubnormals = TestType::hasSubnormals;
		constexpr bool hasSupernormals = TestType::hasSupernormals;
		constexpr bool isSaturating = TestType::isSaturating;

		std::cerr << '\n' << typeid(TestType).name() << '\n';
		// this is too verbose, so I turned it off
		// std::cerr << "                                                     ignoring subnormals for the moment\n";

		int nrOfFailedTests = 0;
		cfloat<32, 8, uint32_t> ref; // this is a superset of IEEE-754 float with supernormals
		cfloat<nbits, es, BlockType, hasSubnormals, hasSupernormals, isSaturating> nut;
		float refValue{ 0.0f };
		float testValue{ 0.0f };
		// run randoms
		std::random_device rd;     // get a random seed from the OS entropy device
		std::mt19937_64 eng(rd()); // use the 64-bit Mersenne Twister 19937 generator and seed it with entropy.
		// define the distribution, by default it goes from 0 to MAX(unsigned long long)
		std::uniform_int_distribution<uint32_t> distr;
		for (unsigned i = 1; i < nrOfRandoms; i++) {
			uint32_t rawBits = distr(eng);
			ref.setbits(rawBits);
			refValue = float(ref);
			nut = refValue;
			testValue = float(nut);
			if (isdenorm(refValue)) {
//				std::cerr << "rhs is subnormal: " << to_binary(refValue) << " ignoring for the moment\n";
				continue;
			}
			nrOfFailedTests += Compare(refValue, testValue, refValue, bReportIndividualTestCases);
#ifdef CUSTOM_FEEDBACK
			if (testValue != refValue) {
				std::cout << to_binary(nut) << '\n' << to_binary(ref) << std::endl;
			}
#endif
			if (nrOfFailedTests > 24) {
				std::cerr << "Too many failures, exiting...\n";
				break;
			}
		}
		return nrOfFailedTests;
	}

// #define CUSTOM_FEEDBACK
	// generate random test cases to test conversion from an IEEE-754 double to a cfloat
	template<typename TestType>
	int VerifyDouble2CfloatConversionRnd(bool bReportIndividualTestCases, size_t nrOfRandoms = 10000) {
		constexpr size_t nbits = TestType::nbits;
		constexpr size_t es = TestType::es;
		using BlockType = typename TestType::BlockType;

		std::cerr << "                                                     ignoring subnormals for the moment\n";

		int nrOfFailedTests = 0;
		cfloat<64, 11, uint64_t> ref;
		cfloat<nbits, es, BlockType> nut;
		double refValue{ 0.0 };
		double testValue{ 0.0 };
		// run randoms
		std::random_device rd;     // get a random seed from the OS entropy device
		std::mt19937_64 eng(rd()); // use the 64-bit Mersenne Twister 19937 generator and seed it with entropy.
		// define the distribution, by default it goes from 0 to MAX(unsigned long long)
		std::uniform_int_distribution<uint64_t> distr;
		for (unsigned i = 1; i < nrOfRandoms; i++) {
			uint64_t rawBits = distr(eng);
			ref.setbits(rawBits);
			refValue = double(ref);
			nut = refValue;
			testValue = double(nut);
			if (isdenorm(refValue)) {
//				std::cerr << "rhs is subnormal: " << to_binary(refValue) << " ignoring for the moment\n";
				continue;
			}
			nrOfFailedTests += Compare(refValue, testValue, refValue, bReportIndividualTestCases);
#ifdef CUSTOM_FEEDBACK
			if (testValue != refValue) {
				std::cout << "nut : " << to_binary(nut) << '\n' << "ref : " << to_binary(ref) << std::endl;
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
	int VerifyFloatSubnormals(bool bReportIndividualTestCases) {
		using namespace std;
		using namespace sw::universal;
		constexpr size_t nbits = 32;
		constexpr size_t es = 8;
		int nrOfFailedTests = 0;
		cfloat<nbits, es, BlockType> nut, result;
		float f{ 0.0f };
		// verify the subnormals
		nut = 0;
		++nut;
		for (size_t i = 0; i < ieee754_parameter<float>::fbits; ++i) {
			f = float(nut);
			result = f;
			if (result != nut) {
				nrOfFailedTests += Compare(f, result, nut, bReportIndividualTestCases);
			}
			uint64_t fraction = nut.fraction_ull();
			fraction <<= 1;
			nut.setfraction(fraction);
		}
		return nrOfFailedTests;
	}

	// generate IEEE-754 double precision subnormal values
	template<typename BlockType>
	int VerifyDoubleSubnormals(bool bReportIndividualTestCases) {
		using namespace std;
		using namespace sw::universal;
		constexpr size_t nbits = 64;
		constexpr size_t es = 11;
		int nrOfFailedTests = 0;
		cfloat<nbits, es, BlockType> nut, result;
		double d{ 0.0f };
		// verify the subnormals
		nut = 0;
		++nut;
		for (size_t i = 0; i < ieee754_parameter<float>::fbits; ++i) {
			d = double(nut);
			result = d;
			if (result != nut) {
				nrOfFailedTests += Compare(d, result, nut, bReportIndividualTestCases);
			}
			uint64_t fraction = nut.fraction_ull();
			fraction <<= 1;
			nut.setfraction(fraction);
		}
		return nrOfFailedTests;
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

		std::vector< cfloat<nbits, es> > s(NR_OF_REALS);
		TestType c;
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

	// validate the increment operator++
	template<typename TestType>
	int VerifyCfloatIncrement(bool bReportIndividualTestCases)
	{
		constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
		constexpr size_t es = TestType::es;
		using BlockType = typename TestType::BlockType;
		constexpr bool hasSubnormals = TestType::hasSubnormals;
		constexpr bool hasSupernormals = TestType::hasSupernormals;
		constexpr bool isSaturating = TestType::isSaturating;

		std::vector< cfloat<nbits, es, BlockType, hasSubnormals, hasSupernormals, isSaturating> > set;
		GenerateOrderedCfloatSet(set); // [snan, -inf, maxneg, ..., -0, +0, ..., maxpos, +inf, nan]

		int nrOfFailedTestCases = 0;

		TestType c, ref;
		// starting from SNaN iterating from -inf, -maxpos to maxpos, +inf, +nan
		for (typename std::vector < cfloat<nbits, es> >::iterator it = set.begin(); it != set.end() - 1; ++it) {
			c = *it;
			c++; // this will test both postfix and prefix operators
			ref = *(it + 1);
			if (c != ref) {
				if (bReportIndividualTestCases) std::cout << " FAIL " << c << " != " << ref << std::endl;
				nrOfFailedTestCases++;
			}
		}

		return nrOfFailedTestCases;
	}

	// validate the decrement operator--
	template<typename TestType>
	int VerifyCfloatDecrement(bool bReportIndividualTestCases)
	{
		constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
		constexpr size_t es = TestType::es;
		using BlockType = typename TestType::BlockType;
		constexpr bool hasSubnormals = TestType::hasSubnormals;
		constexpr bool hasSupernormals = TestType::hasSupernormals;
		constexpr bool isSaturating = TestType::isSaturating;

		std::vector< cfloat<nbits, es> > set;
		GenerateOrderedCfloatSet(set); // [snan, -inf, maxneg, ..., -0, +0, ..., maxpos, +inf, nan]

		int nrOfFailedTestCases = 0;

		cfloat<nbits, es> c, ref;
		// starting from +nan, +inf, maxpos, ..., +0, -0, ..., maxneg, -inf, -nan
		for (typename std::vector < cfloat<nbits, es> >::iterator it = set.end() - 1; it != set.begin(); --it) {
			c = *it;
			c--;  // this will test both postfix and prefix operators
			ref = *(it - 1);

			if (c != ref) {
				if (bReportIndividualTestCases) std::cout << " FAIL " << c << " != " << ref << std::endl;
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
/// <param name="tag">string representation of the type</param>
/// <param name="bReportIndividualTestCases">if yes, report on individual test failures</param>
/// <returns></returns>
	template<typename TestType>
	int VerifyCfloatAddition(bool bReportIndividualTestCases) {
		constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
		constexpr size_t es = TestType::es;
		using BlockType = typename TestType::BlockType;
		constexpr bool hasSubnormals = TestType::hasSubnormals;
		constexpr bool hasSupernormals = TestType::hasSupernormals;
		constexpr bool isSaturating = TestType::isSaturating;
		constexpr size_t NR_VALUES = (size_t(1) << nbits);
		int nrOfFailedTests = 0;

		// set the saturation clamps
		TestType maxpos(sw::universal::SpecificValue::maxpos), maxneg(sw::universal::SpecificValue::maxneg);

		double da, db, ref;  // make certain that IEEE doubles are sufficient as reference
		TestType a, b, nut, cref;
		for (size_t i = 0; i < NR_VALUES; i++) {
			a.setbits(i); // number system concept requires a member function setbits()
			da = double(a);
			for (size_t j = 0; j < NR_VALUES; j++) {
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
#endif // THROW_ARITHMETIC_EXCEPTION

				if (nut != cref) {
					if (ref == 0 and nut.iszero()) continue; // mismatched is ignored as compiler optimizes away negative zero
					nrOfFailedTests++;
					if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "+", a, b, cref, nut);
				}
				else {
					//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "+", a, b, cref, nut);
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
} // namespace sw::universal

