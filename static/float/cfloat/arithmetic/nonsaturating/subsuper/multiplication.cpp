// multiplication.cpp: test suite runner for multiplication on classic floats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// minimum set of include files to reflect source code dependencies
//#define ALGORITHM_VERBOSE_OUTPUT
//#define ALGORITHM_TRACE_MUL
//#define BLOCKTRIPLE_VERBOSE_OUTPUT
//#define BLOCKTRIPLE_TRACE_MUL
//#define TRACE_CONVERSION 1
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/test_suite_randoms.hpp>
#include <universal/verification/cfloat_test_suite.hpp>

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	// cfloat encoding configuration for the test
	constexpr bool hasSubnormals   = true;
	constexpr bool hasMaxExpValues = true;
	constexpr bool isSaturating    = false;

	std::string test_suite         = "classic cfloat multiplication validation with subnormals, normals, and max-exponent values";
	std::string test_tag           = "cfloat_ttf multiplication";
	bool reportTestCases           = false;
	int nrOfFailedTestCases        = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// shorthand alias types
	using c16  = cfloat< 16, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
	using c24  = cfloat< 24, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
	using c32  = cfloat< 32, 8, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
	using c48  = cfloat< 48, 8, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
	using c64  = cfloat< 64, 11, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
	using c80  = cfloat< 80, 11, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
	using c96  = cfloat< 96, 15, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
	using c128 = cfloat<128, 15, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;

	// driving the intensity of the randomized arithmetic tests
	size_t nrRandoms = 0;

#if MANUAL_TESTING
	
//	nrOfFailedTestCases += TestCase< c48, double >(TestCaseOperator::MUL, "0b0.11101100.101101110100011010000110110101110011110", "0b1.10001111.110100000100000110010100100011110100111");

	// some historical debug cases
//	nrOfFailedTestCases += TestCase< cfloat<4, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(TestCaseOperator::MUL, 0.5f, 0.5f);
//	nrOfFailedTestCases += TestCase< cfloat<6, 1, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(TestCaseOperator::MUL, 0.125f, 0.25f); // exp is smaller than min_exp_subnormal
//	nrOfFailedTestCases += TestCase< cfloat<6, 1, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(TestCaseOperator::MUL, 0.125f, 0.5f);  // round down to 0
	nrOfFailedTestCases += TestCase< cfloat<6, 1, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(TestCaseOperator::MUL, 0.125f, 0.625f);
	nrOfFailedTestCases += TestCase< cfloat<6, 1, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(TestCaseOperator::MUL, 0.125f, -0.625f);
//	nrOfFailedTestCases += TestCase< cfloat<6, 1, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(TestCaseOperator::MUL, 0.125, 0.625);	
//  nrOfFailedTestCases += TestCase< cfloat<6, 1, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(TestCaseOperator::MUL, 0.125f, 0.75f); // round up to minpos 0.125
//	nrOfFailedTestCases += TestCase< cfloat<6, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(TestCaseOperator::MUL, 0.125f, 0.25f);
//	nrOfFailedTestCases += TestCase< cfloat<6, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(TestCaseOperator::MUL, 0.125f, 0.5f);
   	nrOfFailedTestCases += TestCase< cfloat<6, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(TestCaseOperator::MUL, 0.125f, 0.625f);
	nrOfFailedTestCases += TestCase< cfloat<6, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(TestCaseOperator::MUL, 0.125f, -0.625f);
//	nrOfFailedTestCases += TestCase< cfloat<6, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(TestCaseOperator::MUL, 0.125f, 0.75f);

//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<4, 1, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 4, 1,uint8_t,t,t,f>", "multiplication");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<4, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 4, 2,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<6, 1, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(true), "cfloat< 6, 1,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<6, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(true), "cfloat< 6, 2,uint8_t,t,t,f>", "multiplication");

	nrOfFailedTestCases += TestCase< c48, double >(TestCaseOperator::MUL, "0b0.11101100.101101110100011010000110110101110011110", "0b1.10001111.110100000100000110010100100011110100111");

	/*
	 for c48 and c64 we are getting rounding errors: can that be caused by double rounding cases in the test bench? 
	 The conversion of a c48 operarnd value into a double will have a rounding event, then the reference calculation might round slightly differently.
	 Why does this happen for nbits > 32?
	*/
	reportTestCases = true;
	nrRandoms = 100000;
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c16  >(reportTestCases, OPCODE_MUL, nrRandoms), typeid(c16).name(), "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c24  >(reportTestCases, OPCODE_MUL, nrRandoms), typeid(c24).name(), "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c32  >(reportTestCases, OPCODE_MUL, nrRandoms), typeid(c32).name(), "multiplication");
	nrRandoms = 10;
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c48  >(reportTestCases, OPCODE_MUL, nrRandoms), typeid(c48).name(), "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c64  >(reportTestCases, OPCODE_MUL, nrRandoms), typeid(c64).name(), "multiplication");
	nrRandoms = 0; // TBD -> configurations that are more precise then double precision require a vector of 64bit words to construct the random bits
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c80  >(reportTestCases, OPCODE_MUL, nrRandoms), typeid(c80).name(), "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c96  >(reportTestCases, OPCODE_MUL, nrRandoms), typeid(c96).name(), "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c128 >(reportTestCases, OPCODE_MUL, nrRandoms), typeid(c128).name(), "multiplication");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<3, 1, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 3, 1,uint8_t,t,t,f>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<4, 1, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 4, 1,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<4, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 4, 2,uint8_t,t,t,f>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<5, 1, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 5, 1,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<5, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 5, 2,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<5, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 5, 3,uint8_t,t,t,f>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<6, 1, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 6, 1,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<6, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 6, 2,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<6, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 6, 3,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<6, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 6, 4,uint8_t,t,t,f>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<7, 1, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 7, 1,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<7, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 7, 2,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<7, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 7, 3,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<7, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 7, 4,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<7, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 7, 5,uint8_t,t,t,f>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<8, 1, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 8, 1,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<8, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 8, 2,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<8, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 8, 3,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<8, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 8, 4,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<8, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 8, 5,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<8, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 8, 6,uint8_t,t,t,f>", "multiplication");

	nrRandoms = 10000;
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c16  >(reportTestCases, RandomsOp::OPCODE_MUL, nrRandoms), typeid(c16).name(), "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c24  >(reportTestCases, RandomsOp::OPCODE_MUL, nrRandoms), typeid(c24).name(), "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c32  >(reportTestCases, RandomsOp::OPCODE_MUL, nrRandoms), typeid(c32).name(), "multiplication");
	nrRandoms = 0; // TBD -> there are double rounding errors in the test bench
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c48  >(reportTestCases, RandomsOp::OPCODE_MUL, nrRandoms), typeid(c48).name(), "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c64  >(reportTestCases, RandomsOp::OPCODE_MUL, nrRandoms), typeid(c64).name(), "multiplication");
	nrRandoms = 0; // TBD -> configurations that are more precise then double precision require a vector of 64bit words to construct the random bits
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c80  >(reportTestCases, RandomsOp::OPCODE_MUL, nrRandoms), typeid(c80).name(), "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c96  >(reportTestCases, RandomsOp::OPCODE_MUL, nrRandoms), typeid(c96).name(), "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c128 >(reportTestCases, RandomsOp::OPCODE_MUL, nrRandoms), typeid(c128).name(), "multiplication");

#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<9, 1, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 9, 1,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<9, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 9, 2,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<9, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 9, 3,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<9, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 9, 4,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<9, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 9, 5,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<9, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 9, 6,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<9, 7, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 9, 7,uint8_t,t,t,f>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<10, 1, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<10, 1,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<10, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<10, 2,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<10, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<10, 3,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<10, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<10, 4,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<10, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<10, 5,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<10, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<10, 6,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<10, 7, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<10, 7,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<10, 8, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<10, 8,uint8_t,t,t,f>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<11, 1, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<11, 1,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<11, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<11, 2,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<11, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<11, 3,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<11, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<11, 4,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<11, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<11, 5,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<11, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<11, 6,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<11, 7, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<11, 7,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<11, 8, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<11, 8,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<11, 9, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<11, 9,uint8_t,t,t,f>", "multiplication");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12, 1, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<12, 1,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<12, 2,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<12, 3,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<12, 4,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<12, 5,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<12, 6,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12, 7, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<12, 7,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12, 8, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<12, 8,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12, 9, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<12, 9,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12,10, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<12,10,uint8_t,t,t,f>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<13, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<13, 3,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<13, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<13, 4,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<13, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<13, 5,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<13, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<13, 6,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<13, 7, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<13, 7,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<13, 8, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<13, 8,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<13, 9, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<13, 9,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<13, 10, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<13,10,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<13, 11, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<13,11,uint8_t,t,t,f>", "multiplication");
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<14, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<14, 3,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<14, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<14, 4,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<14, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<14, 5,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<14, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<14, 6,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<14, 7, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<14, 7,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<14, 8, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<14, 8,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<14, 9, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<14, 9,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<14, 10, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<14,10,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<14, 11, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<14,11,uint8_t,t,t,f>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<15, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<15, 3,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<15, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<15, 4,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<15, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<15, 5,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<15, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<15, 6,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<15, 7, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<15, 7,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<15, 8, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<15, 8,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<15, 9, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<15, 9,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<15, 10, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<15,10,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<15, 11, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<15,11,uint8_t,t,t,f>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<16, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<16, 3,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<16, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<16, 4,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<16, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<16, 5,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<16, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<16, 6,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<16, 7, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<16, 7,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<16, 8, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<16, 8,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<16, 9, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<16, 9,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<16, 10, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<16,10,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<16, 11, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<16,11,uint8_t,t,t,f>", "multiplication");
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
