// from_blocktriple.cpp: test suite runner for conversion tests between blocktriple and cfloats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// minimum set of include files to reflect source code dependencies
// Configure the cfloat template environment
// first: enable general or specialized configurations
#define CFLOAT_FAST_SPECIALIZATION
// second: enable/disable arithmetic exceptions
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 0
// third: enable trace conversion
#define TRACE_CONVERSION 0

#include <universal/number/cfloat/cfloat_impl.hpp>
#include <universal/number/cfloat/manipulators.hpp>
#include <universal/number/cfloat/mathlib.hpp>
#include <universal/verification/test_suite_conversion.hpp>
#include <universal/verification/cfloat_test_suite.hpp>
#include <universal/number/cfloat/table.hpp>

/*
How do you test the conversion state space of blocktriple to cfloat.
We need to convert the blocktriple that comes out of an ADD, a MUL, and a DIV operation.
The blocktriples have bits that need to be rounded by convert.
How do you test that rounding?

Convert the blocktriple to a value.
Use the cfloat operator=() to round. That is your reference. This assumes that cfloat operator=() has been validated.
Use convert() to convert to a cfloat.
Compare the operator=() and convert() cfloat patterns to check correctness
 */

/*
Generate table for a cfloat<4, 2, unsigned char, noSubnormals, noSupernormals, notSaturating>  in TXT format
   #           Binary    sign   scale        exponent        fraction                         value      hex_format
   0:         0b0.00.0       0      -1             b00              b0                             0        4.2x0x0c
   1:         0b0.00.1       0      -1             b00              b1                             0        4.2x0x1c
   2:         0b0.01.0       0       0             b01              b0                             1        4.2x0x2c
   3:         0b0.01.1       0       0             b01              b1                           1.5        4.2x0x3c
   4:         0b0.10.0       0       1             b10              b0                             2        4.2x0x4c
   5:         0b0.10.1       0       1             b10              b1                             3        4.2x0x5c
   6:         0b0.11.0       0       2             b11              b0                           nan        4.2x0x6c
   7:         0b0.11.1       0       2             b11              b1                           nan        4.2x0x7c
   8:         0b1.00.0       1      -1             b00              b0                            -0        4.2x0x8c
   9:         0b1.00.1       1      -1             b00              b1                            -0        4.2x0x9c
  10:         0b1.01.0       1       0             b01              b0                            -1        4.2x0xAc
  11:         0b1.01.1       1       0             b01              b1                          -1.5        4.2x0xBc
  12:         0b1.10.0       1       1             b10              b0                            -2        4.2x0xCc
  13:         0b1.10.1       1       1             b10              b1                            -3        4.2x0xDc
  14:         0b1.11.0       1       2             b11              b0                     nan(snan)        4.2x0xEc
  15:         0b1.11.1       1       2             b11              b1                     nan(snan)        4.2x0xFc

Generate table for a cfloat<5, 2, unsigned char, noSubnormals, noSupernormals, notSaturating>  in TXT format
   #           Binary    sign   scale        exponent        fraction                         value      hex_format
   0:        0b0.00.00       0      -2             b00             b00                             0       5.2x0x00c
   1:        0b0.00.01       0      -2             b00             b01                             0       5.2x0x01c
   2:        0b0.00.10       0      -1             b00             b10                             0       5.2x0x02c
   3:        0b0.00.11       0      -1             b00             b11                             0       5.2x0x03c
   4:        0b0.01.00       0       0             b01             b00                             1       5.2x0x04c
   5:        0b0.01.01       0       0             b01             b01                          1.25       5.2x0x05c
   6:        0b0.01.10       0       0             b01             b10                           1.5       5.2x0x06c
   7:        0b0.01.11       0       0             b01             b11                          1.75       5.2x0x07c
   8:        0b0.10.00       0       1             b10             b00                             2       5.2x0x08c
   9:        0b0.10.01       0       1             b10             b01                           2.5       5.2x0x09c
  10:        0b0.10.10       0       1             b10             b10                             3       5.2x0x0Ac
  11:        0b0.10.11       0       1             b10             b11                           3.5       5.2x0x0Bc
  12:        0b0.11.00       0       2             b11             b00                           nan       5.2x0x0Cc
  13:        0b0.11.01       0       2             b11             b01                           nan       5.2x0x0Dc
  14:        0b0.11.10       0       2             b11             b10                           nan       5.2x0x0Ec
  15:        0b0.11.11       0       2             b11             b11                           nan       5.2x0x0Fc
  16:        0b1.00.00       1      -2             b00             b00                            -0       5.2x0x10c
  17:        0b1.00.01       1      -2             b00             b01                            -0       5.2x0x11c
  18:        0b1.00.10       1      -1             b00             b10                            -0       5.2x0x12c
  19:        0b1.00.11       1      -1             b00             b11                            -0       5.2x0x13c
  20:        0b1.01.00       1       0             b01             b00                            -1       5.2x0x14c
  21:        0b1.01.01       1       0             b01             b01                         -1.25       5.2x0x15c
  22:        0b1.01.10       1       0             b01             b10                          -1.5       5.2x0x16c
  23:        0b1.01.11       1       0             b01             b11                         -1.75       5.2x0x17c
  24:        0b1.10.00       1       1             b10             b00                            -2       5.2x0x18c
  25:        0b1.10.01       1       1             b10             b01                          -2.5       5.2x0x19c
  26:        0b1.10.10       1       1             b10             b10                            -3       5.2x0x1Ac
  27:        0b1.10.11       1       1             b10             b11                          -3.5       5.2x0x1Bc
  28:        0b1.11.00       1       2             b11             b00                     nan(snan)       5.2x0x1Cc
  29:        0b1.11.01       1       2             b11             b01                     nan(snan)       5.2x0x1Dc
  30:        0b1.11.10       1       2             b11             b10                     nan(snan)       5.2x0x1Ec
  31:        0b1.11.11       1       2             b11             b11                     nan(snan)       5.2x0x1Fc
*/

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	// testing cfloat without subnormals, supernormals, or saturation
	constexpr bool hasSubnormals = false;
	constexpr bool hasSupernormals = false;
	constexpr bool isSaturating = false;

	std::string test_suite = "Conversion from blocktriple to cfloat: ";
	std::string test_tag   = "conversion ";
	std::cout << test_suite << '\n';
	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

#if MANUAL_TESTING

	// to track conversion in more detail
	std::cout << std::setprecision(8);
	std::cerr << std::setprecision(8);


	// how do you round a non-normalized blocktriple, i.e. >= 2.0?
	// you would need to modify the lsb/guard/round/sticky bit masks
	// so that you use all info to make the rounding decision,
	// then normalize and apply the rounding decision.
	{
		// FAIL: (+, -3, 0b010.0) :   0.25  -> 0b0.00.1 != ref 0b0.00.0 or -0 != -0
		// FAIL: (+, -3, 0b011.0) :   0.375 -> 0b0.00.0 != ref 0b0.00.1 or 0 != 0
		// 
		// FAIL: (+, -2, 0b010.0) :   0.5   -> 0b0.01.0 != ref 0b0.00.1 or 1 != 0
		// FAIL: (+, -2, 0b010.1) :   0.625 -> 0b0.01.0 != ref 0b0.00.1 or 1 != 0
		// FAIL: (+, -2, 0b011.0) :   0.75  -> 0b0.01.1 != ref 0b0.01.0 or 1.5 != 1
		// FAIL: (+, -2, 0b011.1) :   0.875 -> 0b0.01.1 != ref 0b0.01.0 or 1.5 != 1
		// PASS: (+, -1, 0b001.0) :   0.5   -> 0b0.00.1 == ref 0b0.00.1 or 0 == 0
		// FAIL: (+, -1, 0b001.1) :   0.75  -> 0b0.00.1 != ref 0b0.01.0 or 0 != 1
		// FAIL: (+, -1, 0b010.0) :   1     -> 0b0.10.0 != ref 0b0.01.0 or 2 != 1
		// FAIL: (+, -1, 0b010.1) :   1.25  -> 0b0.10.1 != ref 0b0.01.0 or 3 != 1
		// FAIL: (+, -1, 0b011.0) :   1.5   -> 0b0.11.0 != ref 0b0.01.1 or nan != 1.5
		// FAIL: (+, -1, 0b011.1) :   1.75  -> 0b0.11.1 != ref 0b0.10.0 or nan != 2
		using Cfloat = cfloat<4, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>;
		// FAIL: (+, -1, 0b001.1) :   0.75  -> 0b0.00.1 != ref 0b0.01.0 or 0 != 1
//		GenerateConversionTest<Cfloat, BlockTripleOperator::ADD>(0x03ull, -1);
	}

	{
		using Cfloat = cfloat<5, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>;
		//FAIL: (+,  -2, 0b0'10.10) :           0.625 -> 0b0.00.01 != ref 0b0.00.10 or 0 != 0
		GenerateConversionTest<Cfloat, BlockTripleOperator::ADD>(0x0a, -2);
		//FAIL: (+,  -1, 0b0'01.01) :           0.625 -> 0b0.00.01 != ref 0b0.00.10 or 0 != 0
		GenerateConversionTest<Cfloat, BlockTripleOperator::ADD>(0x05, -1);
	}

	{
		using Cfloat = cfloat<5, 2, uint8_t, true, hasSupernormals, isSaturating>;
		//FAIL: (+,  -2, 0b0'10.10) :           0.625 -> 0b0.00.01 != ref 0b0.00.10 or 0 != 0
		GenerateConversionTest<Cfloat, BlockTripleOperator::ADD>(0x0a, -2);
		//FAIL: (+,  -1, 0b0'01.01) :           0.625 -> 0b0.00.01 != ref 0b0.00.10 or 0 != 0
		GenerateConversionTest<Cfloat, BlockTripleOperator::ADD>(0x05, -1);
	}

	return 0;

	{
		// checking the other side of the exponential adjustments with cfloats
		// that expand on the dynamic range of IEEE-754
		using Cfloat = cfloat<80, 15, uint8_t, hasSubnormals, hasSupernormals, isSaturating>;
		Cfloat a;
		a = -1.0f;
		std::cout << type_tag(a) << '\n' << to_binary(a) << " : " << a << '\n';
		//			a.constexprClassParameters();
	}



//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion<cfloat<4, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD>(bReportIndividualTestCases), test_tag, "cfloat<4,1,uint8_t,0,0,0> from blocktriple ADD");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion<cfloat<4, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD>(bReportIndividualTestCases), test_tag, "cfloat<4,2,uint8_t,0,0,0> from blocktriple ADD");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion<cfloat<5, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD>(bReportIndividualTestCases), test_tag, "cfloat<5,1,uint8_t,0,0,0> from blocktriple ADD");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion<cfloat<5, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD>(bReportIndividualTestCases), test_tag, "cfloat<5,2,uint8_t,0,0,0> from blocktriple ADD");


#define STRESS_TESTING 0
#if STRESS_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion<cfloat<8, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD>(bReportIndividualTestCases), tag, "cfloat<8,1,uint8_t,0,0,0> from blocktriple ADD");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion<cfloat<8, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD>(bReportIndividualTestCases), tag, "cfloat<8,2,uint8_t,0,0,0> from blocktriple ADD");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion<cfloat<8, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD>(bReportIndividualTestCases), tag, "cfloat<8,3,uint8_t,0,0,0> from blocktriple ADD");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion<cfloat<8, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD>(bReportIndividualTestCases), tag, "cfloat<8,4,uint8_t,0,0,0> from blocktriple ADD");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion<cfloat<8, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD>(bReportIndividualTestCases), tag, "cfloat<8,5,uint8_t,0,0,0> from blocktriple ADD");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion<cfloat<8, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD>(bReportIndividualTestCases), tag, "cfloat<8,6,uint8_t,0,0,0> from blocktriple ADD");
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	nrOfFailedTestCases = 0; // in manual testing we ignore failures for the regression system

#else  // !MANUAL_TESTING

#if REGRESSION_LEVEL_1
	// es = 1
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 3, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 3,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 4, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 4,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 5, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 5,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 6, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 6,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 7, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 7,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 8, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 8,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 9, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 9,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<10, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<10,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<12, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<12,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<16, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<16,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<18, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<18,1>");   // 3 blocks


	// es = 2
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 4, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 4,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 5, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 5,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 6, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 6,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 7, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 7,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 8, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 8,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<10, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<10,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<12, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<12,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<14, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<14,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<16, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<16,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<18, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<18,2>");   // 3 blocks


	// es = 3
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 5, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 5,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 6, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 6,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 7, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 7,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 8, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 8,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<10, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<10,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<12, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<12,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<14, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<14,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<18, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<18,3>");   // 3 blocks


	// es = 4
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 6, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 6,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 7, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 7,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 8, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 8,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<10, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<10,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<12, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<12,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<14, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<14,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<18, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<18,4>");   // 3 blocks


	// es = 5
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 7, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 7,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 8, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 8,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<10, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<10,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<12, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<12,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<14, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<14,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<18, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<18,5>");   // 3 blocks


	// es = 6
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 8, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 8,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 9, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 9,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<10, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<10,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<12, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<12,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<14, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<14,6>");


	// es = 7
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 9, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 9,7>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<10, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<10,7>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<12, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<12,7>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<14, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<14,7>");

	// still failing
	// es = 8
//	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<11, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<11,8>");
//	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<12, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<12,8>");
//	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<14, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<14,8>");
#endif

#if REGRESSION_LEVEL_2


#endif

#if REGRESSION_LEVEL_3


#endif

#if REGRESSION_LEVEL_4


#endif
	
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::cfloat_arithmetic_exception& err) {
	std::cerr << "Uncaught cfloat arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::cfloat_internal_exception& err) {
	std::cerr << "Uncaught cfloat internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
