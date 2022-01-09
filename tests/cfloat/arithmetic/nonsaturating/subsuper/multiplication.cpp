// multiplication.cpp: test suite runner for multiplication on classic floats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// minimum set of include files to reflect source code dependencies
//#define CFLOAT_VERBOSE_OUTPUT
//#define CFLOAT_TRACE_MUL
#define BLOCKTRIPLE_VERBOSE_OUTPUT
//#define BLOCKTRIPLE_TRACE_MUL
//#define TRACE_CONVERSION 1
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
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	// cfloat encoding configuration for the test
	constexpr bool hasSubnormals   = true;
	constexpr bool hasSupernormals = true;
	constexpr bool isSaturating    = false;

	std::string test_suite         = "classic cfloat multiplication validation with subnormals, normals, and supernormals";
	std::string test_tag           = "cfloat_ttf multiplication";
	bool reportTestCases           = false;
	int nrOfFailedTestCases        = 0;

	std::cout << test_suite << '\n';

	// shorthand alias types
	using c16 = cfloat< 16, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>;
	using c32 = cfloat< 32, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating>;
	using c48 = cfloat< 48, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating>;
	using c64 = cfloat< 64, 11, uint8_t, hasSubnormals, hasSupernormals, isSaturating>;
	using c80 = cfloat< 80, 11, uint8_t, hasSubnormals, hasSupernormals, isSaturating>;
	using c96 = cfloat< 96, 15, uint8_t, hasSubnormals, hasSupernormals, isSaturating>;
	using c128 = cfloat<128, 15, uint8_t, hasSubnormals, hasSupernormals, isSaturating>;

	// driving the intensity of the randomized arithmetic tests
	size_t nrRandoms = 0;

#if MANUAL_TESTING

	/*
	cfloat<32, 8, uint32_t, true, true, false> a = 0.078125f;
	std::cout << to_binary(a) << " : " << to_triple(a) << " : " << a << '\n';
	a = 0.125f * 0.5f;
	std::cout << to_binary(a) << " : " << to_triple(a) << " : " << a << '\n';
	a = 0.125f - 0.078125f;
	std::cout << "diff " << a << '\n';
	a = 0.125f - 0.0625f;
	std::cout << "diff " << a << '\n';
	*/
	{
		float f;
		cfloat<6, 1, uint8_t, true, true, false> b;
		f = 0.0625f;
		b = f;
		std::cout << to_binary(b) << " : " << to_triple(b) << " : " << b << " : input " << f << '\n';
		f = 0.078125f;
		b = f;
		std::cout << to_binary(b) << " : " << to_triple(b) << " : " << b << " : input " << to_binary(f) << " : " << f << '\n';
		f = 0.08f;
		b = f;
		std::cout << to_binary(b) << " : " << to_triple(b) << " : " << b << " : input " << f << '\n';
		f = 0.09375f;
		b = f;
		std::cout << to_binary(b) << " : " << to_triple(b) << " : " << b << " : input " << f << '\n';
	}

//	nrOfFailedTestCases += TestCase< cfloat<4, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(TestCaseOperator::MUL, 0.5f, 0.5f);
//	nrOfFailedTestCases += TestCase< cfloat<6, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(TestCaseOperator::MUL, 0.125f, 0.25f); // exp is smaller than min_exp_subnormal
//	nrOfFailedTestCases += TestCase< cfloat<6, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(TestCaseOperator::MUL, 0.125f, 0.5f);  // round down to 0
	nrOfFailedTestCases += TestCase< cfloat<6, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(TestCaseOperator::MUL, 0.125f, 0.625f);
	nrOfFailedTestCases += TestCase< cfloat<6, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(TestCaseOperator::MUL, 0.125f, -0.625f);
//	nrOfFailedTestCases += TestCase< cfloat<6, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(TestCaseOperator::MUL, 0.125, 0.625);	nrOfFailedTestCases += TestCase< cfloat<6, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(TestCaseOperator::MUL, 0.125f, 0.75f); // round up to minpos 0.125
//	nrOfFailedTestCases += TestCase< cfloat<6, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(TestCaseOperator::MUL, 0.125f, 0.25f);
//	nrOfFailedTestCases += TestCase< cfloat<6, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(TestCaseOperator::MUL, 0.125f, 0.5f);
   	nrOfFailedTestCases += TestCase< cfloat<6, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(TestCaseOperator::MUL, 0.125f, 0.625f);
	nrOfFailedTestCases += TestCase< cfloat<6, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(TestCaseOperator::MUL, 0.125f, -0.625f);
//	nrOfFailedTestCases += TestCase< cfloat<6, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(TestCaseOperator::MUL, 0.125f, 0.75f);

//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<4, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 4, 1,uint8_t,t,t,f>", "multiplication");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<4, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 4, 2,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<6, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(true), "cfloat< 6, 1,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<6, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(true), "cfloat< 6, 2,uint8_t,t,t,f>", "multiplication");

	/*
	  some big failures still in the code
	exponent value is out of range: -137
	FAIL -3.7568684608692538955e-23 * -2.3456466726830362118e-19 != 3.0725674823402735975e-42 golden reference is 8.8122860135968420007e-42
	 result 0b0.00000000.000000000001000100100001010100001001101
	 vs ref 0b0.00000000.000000000011000100100001010100001001110
	0b1.00110100.011010110101011110001110110011000000100 * 0b1.01000001.000101001110110011010010000101101001110
	FAIL 6.5536195403635990107e-31 * 0.0013127282327562994624 != 8.6031213973660722105e-34 golden reference is 8.6031213973800851952e-34
	 result 0b0.00010001.000111011110001101000011010001101101111
	 vs ref 0b0.00010001.000111011110001101000011010001101110000   <---- rounding
	0b0.00011010.101010010101101010101110100111010001101 * 0b0.01110101.010110000001111110110011010100000001101
	exponent value is out of range: -131
	FAIL 6.5527924811910106766e-30 * 9.2103024202326837927e-11 != 2.3619001984093845608e-40 golden reference is 6.0353200449428542001e-40
	 result 0b0.00000000.000001010010010011001101101010010010111
	 vs ref 0b0.00000000.000011010010010011001101101010010011000
	0b0.00011110.000010011101000000010110011011101110001 * 0b0.01011101.100101010001001011001001010011111000000
	FAIL -6.7993293468744535226e-15 * 2.703705738435287182e-18 != -1.8383385772642795884e-32 golden reference is -1.838338577266521666e-32
	 result 0b1.00010101.011111011100111011011000000101110100100
	 vs ref 0b1.00010101.011111011100111011011000000101110100101   <---- rounding
	0b1.01001111.111010011111000101111101000000111000000 * 0b0.01000100.100011101111111100011101011010000001110
	FAIL 1.1136956898215988858e+33 * -118849.58031165599823 != -8.9826969465018360387e+37 golden reference is -1.3236226533013566832e+38
	 result 0b1.11111101.000011100101000000101110000111001000001
	 vs ref 0b1.11111101.100011100101000000101110000111001000001     <------- an error at the MSB!!!!!
	0b0.11101100.101101110100011010000110110101110011110 * 0b1.10001111.110100000100000110010100100011110100111
	class sw::universal::cfloat<48,8,unsigned char,1,1,0>        multiplication FAIL 5 failed test cases



	FAIL -1.8942454371876639841e+272 * 3.3238837284248946071e+242 != -inf                 golden reference is -inf
	 result 0b1.11111111111.1111111111111111111111111111111111111111111111111110
	 vs ref 0b1.11111111111.0000000000000000000000000000000000000000000000000000
	0b1.11110000111.0110011010001110111111111111000110101100111111100111 * 0b0.11100100100.1000111011001001000100011011111000100010001111000100
	FAIL -5.2007338420249700377e-99 * 0.44028771310810488337 != -2.2898192097891017225e-99 golden reference is -2.2898192097891021285e-99
	 result 0b1.01010110111.0100000010001001110001001100000100001011000100010111
	 vs ref 0b1.01010110111.0100000010001001110001001100000100001011000100011000   <------ rounding
	0b1.01010111000.0110110000000010100001111011110011010101100101001101 * 0b0.01111111101.1100001011011010110010000100001010000101000011110100
	FAIL -2.5503942468822248532e+55 * -4.3898477589713688968e+164 != 1.1195842469169405759e+220 golden reference is 1.1195842469169407013e+220
	 result 0b0.11011011001.1111101101110101001100001010011001001000011010010010
	 vs ref 0b0.11011011001.1111101101110101001100001010011001001000011010010011   <------ rounding
	0b1.10010110111.0000101001000110000010011001001011111001010000000101 * 0b1.11000100001.1110011111100000111011011001111000100000010000110101
	FAIL -1.418446546706757038e+208 * -3.8217933368532045564e+164 != inf                  golden reference is inf
	 result 0b0.11111111111.1111111111111111111111111111111111111111111111111110
	 vs ref 0b0.11111111111.0000000000000000000000000000000000000000000000000000
	0b1.11010110010.0110000101110010110001011101100111100010101100010101 * 0b1.11000100001.1010100010111111000010001101101110011110110101011111
	class sw::universal::cfloat<64,11,unsigned char,1,1,0>       multiplication FAIL 4 failed test cases
	*/
	reportTestCases = true;
	nrRandoms = 10;
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c16  >(reportTestCases, OPCODE_MUL, nrRandoms), typeid(c16).name(), "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c32  >(reportTestCases, OPCODE_MUL, nrRandoms), typeid(c32).name(), "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c48  >(reportTestCases, OPCODE_MUL, nrRandoms), typeid(c48).name(), "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c64  >(reportTestCases, OPCODE_MUL, nrRandoms), typeid(c64).name(), "multiplication");
	// TBD > double precision requires a vector of 64bit words to construct the random bits
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c80  >(reportTestCases, OPCODE_MUL, nrRandoms), typeid(c80).name(), "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c96  >(reportTestCases, OPCODE_MUL, nrRandoms), typeid(c96).name(), "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c128 >(reportTestCases, OPCODE_MUL, nrRandoms), typeid(c128).name(), "multiplication");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<3, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 3, 1,uint8_t,t,t,f>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<4, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 4, 1,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<4, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 4, 2,uint8_t,t,t,f>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<5, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 5, 1,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<5, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 5, 2,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<5, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 5, 3,uint8_t,t,t,f>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<6, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 6, 1,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<6, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 6, 2,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<6, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 6, 3,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<6, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 6, 4,uint8_t,t,t,f>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<7, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 7, 1,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<7, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 7, 2,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<7, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 7, 3,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<7, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 7, 4,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<7, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 7, 5,uint8_t,t,t,f>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<8, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 8, 1,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<8, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 8, 2,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<8, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 8, 3,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<8, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 8, 4,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<8, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 8, 5,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<8, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 8, 6,uint8_t,t,t,f>", "multiplication");

	nrRandoms = 0;
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c16  >(reportTestCases, OPCODE_MUL, nrRandoms), typeid(c16).name(), "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c32  >(reportTestCases, OPCODE_MUL, nrRandoms), typeid(c32).name(), "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c48  >(reportTestCases, OPCODE_MUL, nrRandoms), typeid(c48).name(), "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c64  >(reportTestCases, OPCODE_MUL, nrRandoms), typeid(c64).name(), "multiplication");
	nrRandoms = 0; // TBD > double precision requires a vector of 64bit words to construct the random bits
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c80  >(reportTestCases, OPCODE_MUL, nrRandoms), typeid(c80).name(), "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c96  >(reportTestCases, OPCODE_MUL, nrRandoms), typeid(c96).name(), "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms< c128 >(reportTestCases, OPCODE_MUL, nrRandoms), typeid(c128).name(), "multiplication");

#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<9, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 9, 1,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<9, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 9, 2,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<9, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 9, 3,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<9, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 9, 4,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<9, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 9, 5,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<9, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 9, 6,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<9, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 9, 7,uint8_t,t,t,f>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<10, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<10, 1,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<10, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<10, 2,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<10, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<10, 3,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<10, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<10, 4,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<10, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<10, 5,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<10, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<10, 6,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<10, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<10, 7,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<10, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<10, 8,uint8_t,t,t,f>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<11, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<11, 1,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<11, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<11, 2,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<11, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<11, 3,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<11, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<11, 4,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<11, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<11, 5,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<11, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<11, 6,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<11, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<11, 7,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<11, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<11, 8,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<11, 9, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<11, 9,uint8_t,t,t,f>", "multiplication");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12, 1,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12, 2,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12, 3,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12, 4,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12, 5,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12, 6,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12, 7,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12, 8,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12, 9, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12, 9,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12,10, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12,10,uint8_t,t,t,f>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<13, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<13, 3,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<13, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<13, 4,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<13, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<13, 5,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<13, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<13, 6,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<13, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<13, 7,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<13, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<13, 8,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<13, 9, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<13, 9,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<13, 10, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<13,10,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<13, 11, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<13,11,uint8_t,t,t,f>", "multiplication");
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<14, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<14, 3,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<14, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<14, 4,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<14, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<14, 5,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<14, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<14, 6,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<14, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<14, 7,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<14, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<14, 8,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<14, 9, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<14, 9,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<14, 10, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<14,10,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<14, 11, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<14,11,uint8_t,t,t,f>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<15, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<15, 3,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<15, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<15, 4,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<15, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<15, 5,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<15, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<15, 6,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<15, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<15, 7,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<15, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<15, 8,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<15, 9, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<15, 9,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<15, 10, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<15,10,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<15, 11, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<15,11,uint8_t,t,t,f>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<16, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<16, 3,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<16, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<16, 4,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<16, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<16, 5,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<16, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<16, 6,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<16, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<16, 7,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<16, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<16, 8,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<16, 9, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<16, 9,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<16, 10, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<16,10,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<16, 11, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<16,11,uint8_t,t,t,f>", "multiplication");
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
