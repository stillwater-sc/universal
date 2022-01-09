// addition.cpp: test suite runner for addition on classic floats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// minimum set of include files to reflect source code dependencies
//#define CFLOAT_VERBOSE_OUTPUT
//#define CFLOAT_TRACE_ADD
//#define BLOCKTRIPLE_VERBOSE_OUTPUT
//#define BLOCKTRIPLE_TRACE_ADD
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/test_suite_random.hpp>
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
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	// cfloat encoding configuration for the test
	constexpr bool hasSubnormals   = true;
	constexpr bool hasSupernormals = true;
	constexpr bool isSaturating    = false;

	std::string test_suite         = "classic cfloat addition validation with subnormals, normals, and supernormals";
	std::string test_tag           = "cfloat_ttf addition";
	bool reportTestCases           = false;
	int nrOfFailedTestCases        = 0;

	std::cout << test_suite << '\n';

	// shorthand alias types
	using c16  = cfloat< 16,  5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>;
	using c24  = cfloat< 24, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>;
	using c32  = cfloat< 32,  8, uint8_t, hasSubnormals, hasSupernormals, isSaturating>;
	using c48  = cfloat< 48,  8, uint8_t, hasSubnormals, hasSupernormals, isSaturating>;
	using c64  = cfloat< 64, 11, uint8_t, hasSubnormals, hasSupernormals, isSaturating>;
	using c80  = cfloat< 80, 11, uint8_t, hasSubnormals, hasSupernormals, isSaturating>;
	using c96  = cfloat< 96, 15, uint8_t, hasSubnormals, hasSupernormals, isSaturating>;
	using c128 = cfloat<128, 15, uint8_t, hasSubnormals, hasSupernormals, isSaturating>;

	// driving the intensity of the randomized arithmetic tests
	size_t nrRandoms = 0;

#if MANUAL_TESTING

//  debug helpers
//	ReportCfloatClassParameters<8, 2>();
//	GenerateCfloatExponentBounds();

	// generate individual testcases to hand trace/debug
//	TestCase< cfloat<3, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(TestCaseOperator::ADD, 1.0f, 1.0f);
	TestCase< cfloat<4, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float>(TestCaseOperator::ADD, 0.5f, 0.5f);
	TestCase< cfloat<4, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float>(TestCaseOperator::ADD, 0.5f, 1.0f);
	TestCase< cfloat<4, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float>(TestCaseOperator::ADD, 0.5f, -1.0f);
//	TestCase< cfloat<4, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float>(TestCaseOperator::ADD, 0.5f, 0.5f);
//	TestCase< cfloat<8, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float>(TestCaseOperator::ADD, 1.0f, 1.0f);
//	TestCase< cfloat<16, 8, uint16_t, hasSubnormals, hasSupernormals, isSaturating>, double>(TestCaseOperator::ADD, INFINITY, INFINITY);

//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<3, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<3,1,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<4, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(true), "cfloat<4,1,uint8_t, t,t,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<4, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<4,2,uint8_t, t,t,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<8, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<8,2,uint8_t, t,t,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<8, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<8,3,uint8_t, t,t,f>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<8, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<8,4,uint8_t, t,t,f>", "addition");

	/*
	FAIL -6.4456353792503362653e+38 + -1.0196798390402521646e-21 != -6.4456353792503362653e+38 golden reference is -3.4028236692093846346e+38
	 result 0b1.11111111.11100100111010100110110
	 vs ref 0b1.11111111.00000000000000000000000
	0b1.11111111.11100100111010100110110 + 0b1.00111001.00110100001011011110100
	class sw::universal::cfloat<32,8,unsigned char,1,1,0>        addition FAIL 1 failed test cases
	*/
	{
		c32 a, b, c;
		a.assign("0b1.11111111.11100100111010100110110");
		b.assign("0b1.00111001.00110100001011011110100");
		c = a + b;
		std::cout << a << " + " << b << " = " << c << '\n';
		double da = double(a);
		double db = double(b);
		double dc = da + db;
		std::cout << da << " + " << db << " = " << dc << '\n';
		std::cout << to_binary(c) << '\n';
		std::cout << to_binary(dc) << '\n';
		c = dc;
		std::cout << to_binary(c) << '\n';
	}
	/*
	FAIL 4.5090873941731668264e+273 + -inf                 != -inf                 golden reference is -inf
	 result 0b1.11111111111.0011001010101101001101111011011101010010011011100000
	 vs ref 0b1.11111111111.0000000000000000000000000000000000000000000000000000
	0b0.11110001100.0000101010111001011010101011110101001100010010011001 + 0b1.11111111111.0011001010101101001101111011011101010010011011100000
	 */
	{
		c64 a, b, c;
		a.assign("0b0.11110001100.0000101010111001011010101011110101001100010010011001");
		b.assign("0b1.11111111111.0011001010101101001101111011011101010010011011100000");
		c = a + b;
		std::cout << a << " + " << b << " = " << c << '\n';
		double da = double(a);
		double db = double(b);
		double dc = da + db;
		std::cout << da << " + " << db << " = " << dc << '\n';
		std::cout << to_binary(c) << '\n';
		std::cout << to_binary(dc) << '\n';
		c = dc;
		std::cout << to_binary(c) << '\n';
	}

	reportTestCases = true;
	nrRandoms = 1000;
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c16  >(reportTestCases, OPCODE_ADD, nrRandoms), typeid(c16).name(), "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c24  >(reportTestCases, OPCODE_ADD, nrRandoms), typeid(c24).name(), "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c32  >(reportTestCases, OPCODE_ADD, nrRandoms), typeid(c32).name(), "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c48  >(reportTestCases, OPCODE_ADD, nrRandoms), typeid(c48).name(), "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c64  >(reportTestCases, OPCODE_ADD, nrRandoms), typeid(c64).name(), "addition");
	nrRandoms = 0; // TBD > double precision requires a vector of 64bit words to construct the random bits
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c80  >(reportTestCases, OPCODE_ADD, nrRandoms), typeid(c80).name(), "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c96  >(reportTestCases, OPCODE_ADD, nrRandoms), typeid(c96).name(), "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c128 >(reportTestCases, OPCODE_ADD, nrRandoms), typeid(c128).name(), "addition");


	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else  // !MANUAL_TESTING

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<3, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 3, 1,uint8_t, t,t,f>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<4, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 4, 1,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<4, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 4, 2,uint8_t, t,t,f>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<5, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 5, 1,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<5, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 5, 2,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<5, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 5, 3,uint8_t, t,t,f>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<6, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 6, 1,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<6, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 6, 2,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<6, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 6, 3,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<6, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 6, 4,uint8_t, t,t,f>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<7, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 7, 1,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<7, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 7, 2,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<7, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 7, 3,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<7, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 7, 4,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<7, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 7, 5,uint8_t, t,t,f>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<8, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 8, 1,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<8, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 8, 2,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<8, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 8, 3,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<8, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 8, 4,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<8, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 8, 5,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<8, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 8, 6,uint8_t, t,t,f>", "addition");

	nrRandoms = 10000;
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c16  >(reportTestCases, OPCODE_ADD, nrRandoms), typeid(c16).name(), "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c24  >(reportTestCases, OPCODE_ADD, nrRandoms), typeid(c24).name(), "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c32  >(reportTestCases, OPCODE_ADD, nrRandoms), typeid(c32).name(), "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c48  >(reportTestCases, OPCODE_ADD, nrRandoms), typeid(c48).name(), "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c64  >(reportTestCases, OPCODE_ADD, nrRandoms), typeid(c64).name(), "addition");
	nrRandoms = 0; // TBD > double precision requires a vector of 64bit words to construct the random bits
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c80  >(reportTestCases, OPCODE_ADD, nrRandoms), typeid(c80).name(), "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c96  >(reportTestCases, OPCODE_ADD, nrRandoms), typeid(c96).name(), "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c128 >(reportTestCases, OPCODE_ADD, nrRandoms), typeid(c128).name(), "addition");

#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<9, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 9, 1,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<9, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 9, 2,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<9, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 9, 3,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<9, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 9, 4,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<9, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 9, 5,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<9, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 9, 6,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<9, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 9, 7,uint8_t, t,t,f>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<10, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<10, 1,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<10, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<10, 2,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<10, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<10, 3,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<10, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<10, 4,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<10, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<10, 5,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<10, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<10, 6,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<10, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<10, 7,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<10, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<10, 8,uint8_t, t,t,f>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<11, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<11, 1,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<11, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<11, 2,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<11, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<11, 3,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<11, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<11, 4,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<11, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<11, 5,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<11, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<11, 6,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<11, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<11, 7,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<11, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<11, 8,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<11, 9, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<11, 9,uint8_t, t,t,f>", "addition");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12, 1,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12, 2,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12, 3,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12, 4,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12, 5,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12, 6,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12, 7,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12, 8,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12, 9, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12, 9,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12,10, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12,10,uint8_t, t,t,f>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<13, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<13, 3,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<13, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<13, 4,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<13, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<13, 5,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<13, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<13, 6,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<13, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<13, 7,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<13, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<13, 8,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<13, 9, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<13, 9,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<13, 10, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<13,10,uint8_t ,t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<13, 11, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<13,11,uint8_t, t,t,f>", "addition");
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<14, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<14, 3,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<14, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<14, 4,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<14, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<14, 5,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<14, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<14, 6,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<14, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<14, 7,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<14, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<14, 8,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<14, 9, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<14, 9,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<14, 10, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<14,10,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<14, 11, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<14,11,uint8_t, t,t,f>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<15, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<15, 3,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<15, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<15, 4,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<15, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<15, 5,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<15, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<15, 6,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<15, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<15, 7,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<15, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<15, 8,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<15, 9, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<15, 9,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<15, 10, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<15,10,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<15, 11, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<15,11,uint8_t, t,t,f>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<16, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<16, 3,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<16, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<16, 4,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<16, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<16, 5,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<16, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<16, 6,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<16, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<16, 7,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<16, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<16, 8,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<16, 9, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<16, 9,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<16, 10, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<16,10,uint8_t, t,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<16, 11, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<16,11,uint8_t, t,t,f>", "addition");
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
