// to_blocktriple.cpp: test suite runner for conversion tests between classic cfloats and blocktriples
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

/*
   DESIGN and IMPLEMENTATION HISTORY

   The first floating-point back-end design, value<fbits>, had a fraction 
   bit parameter to select  among different normalizations for 
   addition, multiplication, and division. Inside, these operators
   we would expand and align the operands as needed, requiring a copy.
   
   But the normalization is NOT a generic op, it is very specific for 
   add, mul, div, or sqrt, thus having a fully parameterized interface 
   creates a state space for bugs that could get triggered by incorrect 
   calling of the normalize method. Secondly, no efficient unit test was 
   feasible as most of the state space would NOT be valid conversions.
   Given that context of the experience with value<> we decided to clamp down
   on this parameterization overkill and create explicit normalization 
   conversions for add, mul, div, and sqrt. 
 */

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

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
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

	std::string test_suite = "cfloat to blocktriple conversion validation: ";
	std::string test_tag   = "conversion to blocktriple:";
	std::cout << test_suite << '\n';
	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

#if MANUAL_TESTING

	// to track conversion in more detail
	std::cout << std::setprecision(8);
	std::cerr << std::setprecision(8);

	{

		{
			using Cfloat = cfloat<10, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>;
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
			using Cfloat = cfloat<4, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>;
			//Cfloat a; a.constexprClassParameters();
			nrOfFailedTestCases += ReportTestResult(VerifyCfloatToBlocktripleConversion<Cfloat, BlockTripleOperator::ADD>(bReportIndividualTestCases), tag, "cfloat<4,2> to blocktriple ADD");
			nrOfFailedTestCases += ReportTestResult(VerifyCfloatToBlocktripleConversion<Cfloat, BlockTripleOperator::MUL>(bReportIndividualTestCases), tag, "cfloat<4,2> to blocktriple MUL");
		}
		{
			using Cfloat = cfloat<5, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>;
			//Cfloat a; a.constexprClassParameters();
			nrOfFailedTestCases += ReportTestResult(VerifyCfloatToBlocktripleConversion<Cfloat, BlockTripleOperator::ADD>(bReportIndividualTestCases), tag, "cfloat<5,2> to blocktriple ADD");
			nrOfFailedTestCases += ReportTestResult(VerifyCfloatToBlocktripleConversion<Cfloat, BlockTripleOperator::MUL>(bReportIndividualTestCases), tag, "cfloat<5,2> to blocktriple MUL");
		}

		{
			using Cfloat = cfloat<8, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>;
			//Cfloat a; a.constexprClassParameters();
			nrOfFailedTestCases += ReportTestResult(VerifyCfloatToBlocktripleConversion<Cfloat, BlockTripleOperator::ADD>(bReportIndividualTestCases), tag, "cfloat<8,3> to blocktriple ADD");
			nrOfFailedTestCases += ReportTestResult(VerifyCfloatToBlocktripleConversion<Cfloat, BlockTripleOperator::MUL>(bReportIndividualTestCases), tag, "cfloat<8,3> to blocktriple MUL");
		}

		nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<10, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<10,2> ADD");

	}
	std::cout << "failed tests: " << nrOfFailedTestCases << '\n';
	nrOfFailedTestCases = 0; // in manual testing we ignore failures for the regression system

#if STRESS_TESTING

	// manual exhaustive test

#endif

#else  // !MANUAL_TESTING

#if REGRESSION_LEVEL_1

	// es = 1 is invalid for this cfloat configuration
	/*
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 3, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat< 3,1> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 4, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat< 4,1> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 5, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat< 5,1> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 6, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat< 6,1> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 7, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat< 7,1> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 8, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat< 8,1> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 9, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat< 9,1> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<10, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat<10,1> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<12, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat<12,1> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<16, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat<16,1> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<18, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat<18,1> ADD");   // 3 blocks

	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 3, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat< 3,1> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 4, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat< 4,1> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 5, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat< 5,1> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 6, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat< 6,1> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 7, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat< 7,1> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 8, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat< 8,1> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 9, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat< 9,1> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<10, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat<10,1> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<12, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat<12,1> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<16, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat<16,1> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<18, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat<18,1> MUL");   // 3 blocks
	*/

	// es = 2
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 4, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat< 4,2> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 5, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat< 5,2> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 6, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat< 6,2> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 7, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat< 7,2> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 8, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat< 8,2> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<10, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat<10,2> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<12, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat<12,2> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<14, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat<14,2> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<16, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat<16,2> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<18, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat<18,2> MUL");   // 3 blocks

	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 4, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat< 4,2> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 5, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat< 5,2> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 6, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat< 6,2> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 7, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat< 7,2> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 8, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat< 8,2> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<10, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat<10,2> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<12, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat<12,2> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<14, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat<14,2> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<16, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat<16,2> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<18, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat<18,2> MUL");   // 3 blocks

	// es = 3
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 5, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat< 5,3> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 6, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat< 6,3> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 7, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat< 7,3> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 8, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat< 8,3> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<10, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat<10,3> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<12, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat<12,3> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<14, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat<14,3> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<18, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat<18,3> ADD");   // 3 blocks

	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 5, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat< 5,3> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 6, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat< 6,3> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 7, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat< 7,3> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 8, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat< 8,3> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<10, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat<10,3> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<12, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat<12,3> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<14, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat<14,3> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<18, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat<18,3> MUL");   // 3 blocks

	// es = 4
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 6, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat< 6,4> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 7, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat< 7,4> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 8, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat< 8,4> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<10, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat<10,4> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<12, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat<12,4> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<14, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat<14,4> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<18, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat<18,4> ADD");   // 3 blocks

	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 6, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat< 6,4> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 7, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat< 7,4> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 8, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat< 8,4> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<10, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat<10,4> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<12, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat<12,4> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<14, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat<14,4> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<18, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat<18,4> MUL");   // 3 blocks

	// es = 5
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 7, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat< 7,5> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 8, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat< 8,5> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<10, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat<10,5> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<12, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat<12,5> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<14, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat<14,5> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<18, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat<18,5> ADD");   // 3 blocks

	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 7, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat< 7,5> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 8, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat< 8,5> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<10, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat<10,5> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<12, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat<12,5> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<14, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat<14,5> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<18, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat<18,5> MUL");   // 3 blocks

	// es = 6
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 8, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat< 8,6> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 9, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat< 9,6> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<10, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat<10,6> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<12, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat<12,6> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<14, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat<14,6> ADD");

	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 8, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat< 8,6> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 9, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat< 9,6> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<10, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat<10,6> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<12, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat<12,6> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<14, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat<14,6> MUL");

	// es = 7
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 9, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat< 9,7> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<10, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat<10,7> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<12, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat<12,7> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<14, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat<14,7> ADD");

	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat< 9, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat< 9,7> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<10, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat<10,7> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<12, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat<12,7> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<14, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat<14,7> MUL");

	// es = 8
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<11, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat<11,8> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<12, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat<12,8> ADD");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<14, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), test_tag, "cfloat<14,8> ADD");

	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<11, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat<11,8> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<12, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat<12,8> MUL");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<14, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL >(bReportIndividualTestCases), test_tag, "cfloat<14,8> MUL");
#endif

#if REGRESSION_LEVEL_2

#endif

#if REGRESSION_LEVEL_3

#endif

#if REGRESSION_LEVEL_4

	nrOfFailedTestCases = ReportTestResult(VerifyCfloatToBlocktripleConversion< cfloat<25, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::MUL> >(bReportIndividualTestCases), tag, "cfloat<25,2> MUL");   // 4 blocks

#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	std::cout << test_suite << (nrOfFailedTestCases == 0 ? "PASSED" : "FAILED") << std::endl;

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
