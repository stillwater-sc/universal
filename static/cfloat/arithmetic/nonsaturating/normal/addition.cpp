// addition.cpp: test suite runner for addition on classic floats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// minimum set of include files to reflect source code dependencies
#define BLOCKTRIPLE_VERBOSE_OUTPUT
//#define BLOCKTRIPLE_TRACE_ADD
#define TRACE_CONVERSION 0
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_case.hpp>
#include <universal/verification/test_suite_randoms.hpp>
#include <universal/verification/cfloat_test_suite.hpp>
#include <universal/number/cfloat/table.hpp>

/*
  Minimum number of operand bits for the adder = <abits> 
  to yield correctly rounded addition if you don't use sticky bit consolidation
  during argument normalization.

  You would never build the adder without the sticky bit consolidation
  but I am leaving this comment/table here to call out the computational
  dynamics of what is going on. The alignment will shift out an ULP of a
  small value that will be needed to break a tie. 

                          number of exponent bits = <es>
  nbits   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16
	 1    -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -
	 2    -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -
	 3    2   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -
	 4    3   3   -   -   -   -   -   -   -   -   -   -   -   -   -   -
	 5    4   4   4   -   -   -   -   -   -   -   -   -   -   -   -   -
	 6    5   5   6   4   -   -   -   -   -   -   -   -   -   -   -   -
	 7    6   6   8   6   4   -   -   -   -   -   -   -   -   -   -   -
	 8    7   7  10   8   6   4   -   -   -   -   -   -   -   -   -   -
	 9    8   8  11  10   8   6   4   -   -   -   -   -   -   -   -   -
	10    9   9  12  12  10   8   6   4   -   -   -   -   -   -   -   -
	11   10  10  13  14  12  10   8   6   4   -   -   -   -   -   -   -
	12   11  11  14  16  14  12  10   8   6   4   -   -   -   -   -   -
	13   12  12  15  18  16  14  12  10   8   6   4?  -   -   -   -   -
	14   13  13  16  20  18  16  14  12  10   8   6?  4?  -   -   -   -
	15   14  14  17  22  20  18  16  14  12  10   8?  6?  4?  -   -   -
	16   15  15  18  24  22  20  18  16  14  12  10?  8?  6?  4?  -   -

*/

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
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	// cfloat encoding configuration for the test
	constexpr bool hasSubnormals   = false;
	constexpr bool hasSupernormals = false;
	constexpr bool isSaturating    = false;

	std::string test_suite         = "classic cfloat addition validation with just normals, no subnormals or supernormals";
	std::string test_tag           = "cfloat_fff addition";
	bool reportTestCases           = true;
	int nrOfFailedTestCases        = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// shorthand alias types
	using c16  = cfloat< 16,  5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>;
	using c32  = cfloat< 32,  8, uint8_t, hasSubnormals, hasSupernormals, isSaturating>;
	using c48  = cfloat< 48,  8, uint8_t, hasSubnormals, hasSupernormals, isSaturating>;
	using c64  = cfloat< 64, 11, uint8_t, hasSubnormals, hasSupernormals, isSaturating>;
	using c80  = cfloat< 80, 11, uint8_t, hasSubnormals, hasSupernormals, isSaturating>;
	using c96  = cfloat< 96, 15, uint8_t, hasSubnormals, hasSupernormals, isSaturating>;
	using c128 = cfloat<128, 15, uint8_t, hasSubnormals, hasSupernormals, isSaturating>;

	// driving the intensity of the randomized arithmetic tests
	size_t nrRandoms = 0;

#if MANUAL_TESTING

//	GenerateCfloatExponentBounds();	
//	using TestType = cfloat<5, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>;
//	GenerateTable<TestType>(std::cout);
// 
	// generate individual testcases to hand trace/debug
//	TestCase< cfloat<8, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float>(TestCaseOperator::ADD, 15.0f, 0.265625f);
//	TestCase< cfloat<16, 8, uint16_t, hasSubnormals, hasSupernormals, isSaturating>, double>(TestCaseOperator::ADD, INFINITY, INFINITY);
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<8, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 8, 4,uint8_t, f,f,f>", "addition");
	
	/*
	FAIL   283384055313989632 + nan(snan) != nan(snan) golden reference is                  nan
	result 0b1.11111111.11111111111111111111111
	vs ref 0b0.11111111.10000000000000000000000
	0b0.10111000.11110111011001000010011 + 0b1.11111111.00010010010001011110100

	FAIL                  nan + 3.5262673662011272545e+31 != nan golden reference is                  nan
	result 0b0.11111111.11111111111111111111111
	vs ref 0b0.11111111.10000000000000000000000
	0b0.11111111.01101010111100010101100 + 0b0.11100111.10111101000100111101100
	*/
	c32 a, b, c;
	a.assign("0b0.10111000.11110111011001000010011");
	b.assign("0b1.11111111.00010010010001011110100");
	c = a + b;
	std::cout << a << " + " << b << " = " << c << '\n';
	double da = double(a);
	double db = double(b);
	double dc = da + db;
	c32 testresult, testref;
	executeBinary(OPCODE_ADD, da, db, a, b, testresult, testref);
	std::cout << da << " + " << db << " = " << dc << '\n';
	std::cout << to_binary(testresult) << " : " << testresult << '\n';
	std::cout << to_binary(testref) << " : " << testref << '\n';

	{
		constexpr float fa = std::numeric_limits<float>::infinity();
		float fb = -fa;
		std::cout << fa << " + " << fa << " = " << (fa + fa) << '\n';
		std::cout << fa << " + " << fb << " = " << (fa + fb) << '\n';
		std::cout << fb << " + " << fa << " = " << (fb + fa) << '\n';
		std::cout << fb << " + " << fb << " = " << (fb + fb) << '\n';
		std::cout << to_binary(fa + fb) << '\n';
	}

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<4, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 4, 2,uint8_t, f,f,f>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<5, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 5, 2,uint8_t, f,f,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<5, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 5, 3,uint8_t, f,f,f>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<6, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 6, 2,uint8_t, f,f,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<6, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 6, 3,uint8_t, f,f,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<6, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 6, 4,uint8_t, f,f,f>", "addition");

	nrRandoms = 5;
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c16  >(reportTestCases, OPCODE_ADD, nrRandoms), typeid(c16).name(), "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c32  >(reportTestCases, OPCODE_ADD, nrRandoms), typeid(c32).name(), "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c48  >(reportTestCases, OPCODE_ADD, nrRandoms), typeid(c48).name(), "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c64  >(reportTestCases, OPCODE_ADD, nrRandoms), typeid(c64).name(), "addition");
	// TBD > double precision requires a vector of 64bit words to construct the random bits
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c80  >(reportTestCases, OPCODE_ADD, nrRandoms), typeid(c80).name(), "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c96  >(reportTestCases, OPCODE_ADD, nrRandoms), typeid(c96).name(), "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c128 >(reportTestCases, OPCODE_ADD, nrRandoms), typeid(c128).name(), "addition");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures

#else

#if REGRESSION_LEVEL_1
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<3, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 3, 1,uint8_t, f,f,f>", "addition");

//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<4, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 4, 1,uint8_t, f,f,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<4, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 4, 2,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<5, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 5, 1,uint8_t, f,f,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<5, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 5, 2,uint8_t, f,f,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<5, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 5, 3,uint8_t, f,f,f>", "addition");

//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<6, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 6, 1,uint8_t, f,f,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<6, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 6, 2,uint8_t, f,f,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<6, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 6, 3,uint8_t, f,f,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<6, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 6, 4,uint8_t, f,f,f>", "addition");

//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<7, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 7, 1,uint8_t, f,f,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<7, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 7, 2,uint8_t, f,f,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<7, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 7, 3,uint8_t, f,f,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<7, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 7, 4,uint8_t, f,f,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<7, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 7, 5,uint8_t, f,f,f>", "addition");

//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<8, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 8, 1,uint8_t, f,f,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<8, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 8, 2,uint8_t, f,f,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<8, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 8, 3,uint8_t, f,f,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<8, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 8, 4,uint8_t, f,f,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<8, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 8, 5,uint8_t, f,f,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<8, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 8, 6,uint8_t, f,f,f>", "addition");

	nrRandoms = 5;
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c16  >(reportTestCases, RandomsOp::OPCODE_ADD, nrRandoms), typeid(c16).name(), "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c32  >(reportTestCases, RandomsOp::OPCODE_ADD, nrRandoms), typeid(c32).name(), "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c48  >(reportTestCases, RandomsOp::OPCODE_ADD, nrRandoms), typeid(c48).name(), "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c64  >(reportTestCases, RandomsOp::OPCODE_ADD, nrRandoms), typeid(c64).name(), "addition");
	nrRandoms = 0; // TBD > double precision requires a vector of 64bit words to construct the random bits
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c80  >(reportTestCases, RandomsOp::OPCODE_ADD, nrRandoms), typeid(c80).name(), "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c96  >(reportTestCases, RandomsOp::OPCODE_ADD, nrRandoms), typeid(c96).name(), "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c128 >(reportTestCases, RandomsOp::OPCODE_ADD, nrRandoms), typeid(c128).name(), "addition");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<9, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 9, 2,uint8_t, f,f,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<9, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 9, 3,uint8_t, f,f,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<9, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 9, 4,uint8_t, f,f,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<9, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 9, 5,uint8_t, f,f,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<9, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 9, 6,uint8_t, f,f,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<9, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 9, 7,uint8_t, f,f,f>", "addition");
#endif

#if	REGRESSION_LEVEL_3
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<10, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<10, 2,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<10, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<10, 3,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<10, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<10, 4,uint8_t, f,f,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<10, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<10, 5,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<10, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<10, 6,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<10, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<10, 7,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<10, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<10, 8,uint8_t, f,f,f>", "addition");

//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<11, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<11, 2,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<11, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<11, 3,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<11, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<11, 4,uint8_t, f,f,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<11, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<11, 5,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<11, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<11, 6,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<11, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<11, 7,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<11, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<11, 8,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<11, 9, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<11, 9,uint8_t, f,f,f>", "addition");

//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12, 2,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12, 3,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12, 4,uint8_t, f,f,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12, 5,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12, 6,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12, 7,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12, 8,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12, 9, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12, 9,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12,10, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12,10,uint8_t, f,f,f>", "addition");
#endif

#if	REGRESSION_LEVEL_4
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<13, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<13, 3,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<13, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<13, 4,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<13, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<13, 5,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<13, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<13, 6,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<13, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<13, 7,uint8_t, f,f,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<13, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<13, 8,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<13, 9, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<13, 9,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<13,10, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<13,10,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<13,11, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<13,11,uint8_t, f,f,f>", "addition");

//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<14, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<14, 3,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<14, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<14, 4,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<14, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<14, 5,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<14, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<14, 6,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<14, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<14, 7,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<14, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<14, 8,uint8_t, f,f,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<14, 9, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<14, 9,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<14,10, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<14,10,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<14,11, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<14,11,uint8_t, f,f,f>", "addition");

//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<15, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<15, 3,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<15, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<15, 4,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<15, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<15, 5,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<15, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<15, 6,uint8_t, f,f,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<15, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<15, 7,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<15, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<15, 8,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<15, 9, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<15, 9,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<15,10, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<15,10,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<15,11, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<15,11,uint8_t, f,f,f>", "addition");

//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<16, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<16, 3,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<16, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<16, 4,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<16, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<16, 5,uint8_t, f,f,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<16, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<16, 6,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<16, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<16, 7,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<16, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<16, 8,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<16, 9, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<16, 9,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<16,10, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<16,10,uint8_t, f,f,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<16,11, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<16,11,uint8_t, f,f,f>", "addition");
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
