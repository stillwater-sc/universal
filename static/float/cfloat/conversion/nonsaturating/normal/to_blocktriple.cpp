// to_blocktriple.cpp: test suite runner for conversion tests between classic cfloats and blocktriples
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
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

#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite_conversion.hpp>
#include <universal/verification/cfloat_test_suite.hpp>

/*
How do you test the conversion state space of blocktriple to cfloat?
We need to convert the blocktriple that comes out of an ADD, a MUL, and a DIV operation.
The blocktriples have bits that need to be rounded by convert.
How do you test that rounding?

Convert the blocktriple to a value.
Use the cfloat operator=() to round. That is your reference. This assumes that cfloat operator=() has been validated.
Use convert() to convert to a cfloat.
Compare the operator=() and convert() cfloat patterns to check correctness
 */

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	// testing saturating cfloat without subnormals nor max-exponent values
	constexpr bool hasSubnormals    = false;
	constexpr bool hasMaxExpValues  = false;
	constexpr bool isSaturating     = true;

	std::string test_suite          = "cfloat to blocktriple conversion validation";
	std::string test_tag            = "conversion to blocktriple";
	bool reportTestCases            = false;
	int nrOfFailedTestCases         = 0;
	
	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// to track conversion in more detail
	std::cout << std::setprecision(8);
	std::cerr << std::setprecision(8);

	{

		{
			using Cfloat = cfloat<10, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
			constexpr size_t fbits = Cfloat::fbits;
			typedef Cfloat::BlockType bt;
			Cfloat nut("0b1.10.0000000");
 
			float v = float(nut);
			blocktriple<fbits, BlockTripleOperator::ADD, bt> b, ref; // blocktriple type that comes out of an ADD/SUB operation
			nut.normalizeAddition(b);
			ref = v;
			std::cout << "cfloat          : " << to_binary(nut) << " : " << nut << '\n';
			std::cout << "cfloat          : " << to_triple(nut) << " : " << nut << '\n';
			std::cout << "blocktriple     : " << to_binary(b) << " : " << float(b) << '\n';
			std::cout << "blocktriple ref : " << to_binary(ref) << " : " << ref << '\n';
		}

		{
			using Cfloat = cfloat<4, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
			//Cfloat a; a.constexprClassParameters();
			nrOfFailedTestCases += ReportTestResult(VerifyCfloatToBlocktripleConversion<Cfloat, BlockTripleOperator::ADD>(reportTestCases), test_tag, "cfloat<4,2, uint8_t, fft> to blocktriple ADD");
			nrOfFailedTestCases += ReportTestResult(VerifyCfloatToBlocktripleConversion<Cfloat, BlockTripleOperator::MUL>(reportTestCases), test_tag, "cfloat<4,2, uint8_t, fft> to blocktriple MUL");
			nrOfFailedTestCases += ReportTestResult(VerifyCfloatToBlocktripleConversion<Cfloat, BlockTripleOperator::DIV>(reportTestCases), test_tag, "cfloat<4,2, uint8_t, fft> to blocktriple DIV");
		}
		{
			using Cfloat = cfloat<5, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
			//Cfloat a; a.constexprClassParameters();
			nrOfFailedTestCases += ReportTestResult(VerifyCfloatToBlocktripleConversion<Cfloat, BlockTripleOperator::ADD>(reportTestCases), test_tag, "cfloat<5,2, uint8_t, fft> to blocktriple ADD");
			nrOfFailedTestCases += ReportTestResult(VerifyCfloatToBlocktripleConversion<Cfloat, BlockTripleOperator::MUL>(reportTestCases), test_tag, "cfloat<5,2, uint8_t, fft> to blocktriple MUL");
			nrOfFailedTestCases += ReportTestResult(VerifyCfloatToBlocktripleConversion<Cfloat, BlockTripleOperator::DIV>(reportTestCases), test_tag, "cfloat<5,2, uint8_t, fft> to blocktriple DIV");
		}

		{
			using Cfloat = cfloat<8, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
			//Cfloat a; a.constexprClassParameters();
			nrOfFailedTestCases += ReportTestResult(VerifyCfloatToBlocktripleConversion<Cfloat, BlockTripleOperator::ADD>(reportTestCases), test_tag, "cfloat<8,3, uint8_t, fft> to blocktriple ADD");
			nrOfFailedTestCases += ReportTestResult(VerifyCfloatToBlocktripleConversion<Cfloat, BlockTripleOperator::MUL>(reportTestCases), test_tag, "cfloat<8,3, uint8_t, fft> to blocktriple MUL");
			nrOfFailedTestCases += ReportTestResult(VerifyCfloatToBlocktripleConversion<Cfloat, BlockTripleOperator::DIV>(reportTestCases), test_tag, "cfloat<8,3, uint8_t, fft> to blocktriple DIV");
		}

		nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<10, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<10,2, uint8_t, fft> ADD");

	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else  // !MANUAL_TESTING

#if REGRESSION_LEVEL_1

	// es = 1 is invalid for this cfloat configuration

	// es = 2
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 4, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 4,2, uint8_t, fft> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 5, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 5,2, uint8_t, fft> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 6, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 6,2, uint8_t, fft> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 7, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 7,2, uint8_t, fft> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 8, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 8,2, uint8_t, fft> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<10, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<10,2, uint8_t, fft> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<12, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<12,2, uint8_t, fft> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<14, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<14,2, uint8_t, fft> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<16, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<16,2, uint8_t, fft> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<18, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<18,2, uint8_t, fft> MUL");   // 3 blocks

	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 4, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat< 4,2, uint8_t, fft> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 5, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat< 5,2, uint8_t, fft> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 6, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat< 6,2, uint8_t, fft> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 7, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat< 7,2, uint8_t, fft> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 8, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat< 8,2, uint8_t, fft> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<10, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat<10,2, uint8_t, fft> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<12, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat<12,2, uint8_t, fft> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<14, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat<14,2, uint8_t, fft> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<16, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat<16,2, uint8_t, fft> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<18, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat<18,2, uint8_t, fft> MUL");   // 3 blocks

	// es = 3
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 5, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 5,3, uint8_t, fft> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 6, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 6,3, uint8_t, fft> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 7, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 7,3, uint8_t, fft> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 8, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 8,3, uint8_t, fft> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<10, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<10,3, uint8_t, fft> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<12, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<12,3, uint8_t, fft> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<14, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<14,3, uint8_t, fft> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<18, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<18,3, uint8_t, fft> ADD");   // 3 blocks

	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 5, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat< 5,3, uint8_t, fft> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 6, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat< 6,3, uint8_t, fft> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 7, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat< 7,3, uint8_t, fft> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 8, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat< 8,3, uint8_t, fft> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<10, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat<10,3, uint8_t, fft> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<12, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat<12,3, uint8_t, fft> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<14, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat<14,3, uint8_t, fft> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<18, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat<18,3, uint8_t, fft> MUL");   // 3 blocks

	// es = 4
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 6, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 6,4, uint8_t, fft> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 7, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 7,4, uint8_t, fft> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 8, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 8,4, uint8_t, fft> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<10, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<10,4, uint8_t, fft> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<12, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<12,4, uint8_t, fft> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<14, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<14,4, uint8_t, fft> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<18, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<18,4, uint8_t, fft> ADD");   // 3 blocks

	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 6, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat< 6,4, uint8_t, fft> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 7, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat< 7,4, uint8_t, fft> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 8, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat< 8,4, uint8_t, fft> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<10, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat<10,4, uint8_t, fft> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<12, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat<12,4, uint8_t, fft> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<14, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat<14,4, uint8_t, fft> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<18, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat<18,4, uint8_t, fft> MUL");   // 3 blocks

	// es = 5
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 7, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 7,5, uint8_t, fft> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 8, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 8,5, uint8_t, fft> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<10, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<10,5, uint8_t, fft> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<12, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<12,5, uint8_t, fft> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<14, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<14,5, uint8_t, fft> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<18, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<18,5, uint8_t, fft> ADD");   // 3 blocks

	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 7, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat< 7,5, uint8_t, fft> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 8, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat< 8,5, uint8_t, fft> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<10, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat<10,5, uint8_t, fft> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<12, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat<12,5, uint8_t, fft> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<14, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat<14,5, uint8_t, fft> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<18, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat<18,5, uint8_t, fft> MUL");   // 3 blocks

	// es = 6
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 8, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 8,6, uint8_t, fft> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 9, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 9,6, uint8_t, fft> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<10, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<10,6, uint8_t, fft> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<12, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<12,6, uint8_t, fft> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<14, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<14,6, uint8_t, fft> ADD");

	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 8, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat< 8,6, uint8_t, fft> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 9, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat< 9,6, uint8_t, fft> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<10, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat<10,6, uint8_t, fft> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<12, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat<12,6, uint8_t, fft> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<14, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat<14,6, uint8_t, fft> MUL");

	// es = 7
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 9, 7, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat< 9,7, uint8_t, fft> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<10, 7, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<10,7, uint8_t, fft> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<12, 7, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<12,7, uint8_t, fft> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<14, 7, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<14,7, uint8_t, fft> ADD");

	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 9, 7, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat< 9,7, uint8_t, fft> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<10, 7, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat<10,7, uint8_t, fft> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<12, 7, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat<12,7, uint8_t, fft> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<14, 7, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat<14,7, uint8_t, fft> MUL");

	// es = 8
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<11, 8, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<11,8, uint8_t, fft> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<12, 8, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<12,8, uint8_t, fft> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<14, 8, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::ADD >(reportTestCases), test_tag, "cfloat<14,8, uint8_t, fft> ADD");

	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<11, 8, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat<11,8, uint8_t, fft> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<12, 8, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat<12,8, uint8_t, fft> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<14, 8, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat<14,8, uint8_t, fft> MUL");
#endif

#if REGRESSION_LEVEL_2

#endif

#if REGRESSION_LEVEL_3

#endif

#if REGRESSION_LEVEL_4

	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<25, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>, BlockTripleOperator::MUL >(reportTestCases), test_tag, "cfloat<25,2, uint8_t, fft> MUL");   // 4 blocks

#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}