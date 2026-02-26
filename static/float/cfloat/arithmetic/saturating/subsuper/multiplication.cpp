// multiplication.cpp: test suite runner for multiplication on classic floats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// minimum set of include files to reflect source code dependencies
#define ALGORITHM_VERBOSE_OUTPUT
#define ALGORITHM_TRACE_MUL
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_case.hpp>
//#include <universal/verification/test_suite_arithmetic.hpp>
#include <universal/verification/cfloat_test_suite.hpp>
#include <universal/number/cfloat/table.hpp>

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
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	// cfloat encoding configuration for the test
	constexpr bool hasSubnormals   = true;
	constexpr bool hasMaxExpValues = true;
	constexpr bool isSaturating    = true;

	std::string test_suite         = "Arithmetic multiplication with classic saturating floating-point configurations with subnormals and max-exponent values";
	std::string test_tag           = "cfloat_ttt multiplication";
	bool reportTestCases           = true;
	int nrOfFailedTestCases        = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

/*
Generate table for a class sw::universal::cfloat<3,1,unsigned char,1,1,0> in TXT format
   #           Binary    sign   scale        exponent        fraction                         value      hex_format
   0:          0b0.0.0       0       0              b0              b0                             0        3.1x0x0c
   1:          0b0.0.1       0       0              b0              b1                             1        3.1x0x1c
   2:          0b0.1.0       0       1              b1              b0                           inf        3.1x0x2c
   3:          0b0.1.1       0       1              b1              b1                           nan        3.1x0x3c
   4:          0b1.0.0       1       0              b0              b0                            -0        3.1x0x4c
   5:          0b1.0.1       1       0              b0              b1                            -1        3.1x0x5c
   6:          0b1.1.0       1       1              b1              b0                          -inf        3.1x0x6c
   7:          0b1.1.1       1       1              b1              b1                     nan(snan)        3.1x0x7c

   Generate table for a class sw::universal::cfloat<4,2,unsigned char,1,1,0> in TXT format
   #           Binary    sign   scale        exponent        fraction                         value      hex_format
   0:         0b0.00.0       0      -1             b00              b0                             0        4.2x0x0c
   1:         0b0.00.1       0      -1             b00              b1                           0.5        4.2x0x1c
   2:         0b0.01.0       0       0             b01              b0                             1        4.2x0x2c
   3:         0b0.01.1       0       0             b01              b1                           1.5        4.2x0x3c
   4:         0b0.10.0       0       1             b10              b0                             2        4.2x0x4c
   5:         0b0.10.1       0       1             b10              b1                             3        4.2x0x5c
   6:         0b0.11.0       0       2             b11              b0                           inf        4.2x0x6c
   7:         0b0.11.1       0       2             b11              b1                           nan        4.2x0x7c
   8:         0b1.00.0       1      -1             b00              b0                            -0        4.2x0x8c
   9:         0b1.00.1       1      -1             b00              b1                          -0.5        4.2x0x9c
  10:         0b1.01.0       1       0             b01              b0                            -1        4.2x0xAc
  11:         0b1.01.1       1       0             b01              b1                          -1.5        4.2x0xBc
  12:         0b1.10.0       1       1             b10              b0                            -2        4.2x0xCc
  13:         0b1.10.1       1       1             b10              b1                            -3        4.2x0xDc
  14:         0b1.11.0       1       2             b11              b0                          -inf        4.2x0xEc
  15:         0b1.11.1       1       2             b11              b1                     nan(snan)        4.2x0xFc
   */
	{
		float fa = 0.5f; 
//		float fb = std::numeric_limits<float>::signaling_NaN();
//		float fb = std::numeric_limits<float>::quiet_NaN();
//		float fb = std::numeric_limits<float>::infinity();
		float fb = 1.5f;

		constexpr unsigned nbits = 4;
		constexpr unsigned es = 2;
		using Cfloat = cfloat < nbits, es, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating >;
		Cfloat a, b, c; // uninitialized
//		GenerateTable<Cfloat>(cout);
//		a.constexprClassParameters();
		a = fa;
		b = fb;
		c = a * b;
		std::cout << a << " * " << b << " = " << c << '\n';
		std::cout << to_binary(a) << " * " << to_binary(b) << " = " << to_binary(c) << '\n';

		TestCase< Cfloat, float>(TestCaseOperator::MUL, fa, fb);
	}

	{ // special cases of snan/qnan
		constexpr float fa = std::numeric_limits<float>::quiet_NaN();
		constexpr float fb = std::numeric_limits<float>::signaling_NaN();
		std::cout << fa << " * " << fa << " = " << (fa * fa) << '\n';
		std::cout << fa << " * " << fb << " = " << (fa * fb) << '\n';
		std::cout << fb << " * " << fa << " = " << (fb * fa) << '\n';
		std::cout << fb << " * " << fb << " = " << (fb * fb) << '\n';
		std::cout << to_binary(fa - fb) << '\n';
	}

	{ // special cases of +-inf
		constexpr float fa = std::numeric_limits<float>::infinity();
		float fb = -fa;
		std::cout << fa << " * " << fa << " = " << (fa * fa) << '\n';
		std::cout << fa << " * " << fb << " = " << (fa * fb) << '\n';
		std::cout << fb << " * " << fa << " = " << (fb * fa) << '\n';
		std::cout << fb << " * " << fb << " = " << (fb * fb) << '\n';
		std::cout << 0.0f << " * " << fa << " = " << (0.0f * fa) << '\n';
		std::cout << to_binary(fa - fb) << '\n';
	}

	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatMultiplication< 
		cfloat<3, 1, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(true), 
		"cfloat<3,1,uint8_t,t,t,t>", 
		"multiplication");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatMultiplication<
		cfloat<4, 1, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(true),
		"cfloat<4,1,uint8_t,t,t,t>",
		"multiplication");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<3, 1, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 3, 1,uint8_t,t,t,t>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<4, 1, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 4, 1,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<4, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 4, 2,uint8_t,t,t,t>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<5, 1, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 5, 1,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<5, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 5, 2,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<5, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 5, 3,uint8_t,t,t,t>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<6, 1, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 6, 1,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<6, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 6, 2,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<6, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 6, 3,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<6, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 6, 4,uint8_t,t,t,t>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<7, 1, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 7, 1,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<7, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 7, 2,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<7, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 7, 3,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<7, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 7, 4,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<7, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 7, 5,uint8_t,t,t,t>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<8, 1, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 8, 1,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<8, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 8, 2,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<8, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 8, 3,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<8, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 8, 4,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<8, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 8, 5,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<8, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 8, 6,uint8_t,t,t,t>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<9, 1, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 9, 1,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<9, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 9, 2,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<9, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 9, 3,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<9, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 9, 4,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<9, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 9, 5,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<9, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 9, 6,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<9, 7, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat< 9, 7,uint8_t,t,t,t>", "multiplication");
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<10, 1, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<10, 1,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<10, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<10, 2,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<10, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<10, 3,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<10, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<10, 4,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<10, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<10, 5,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<10, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<10, 6,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<10, 7, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<10, 7,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<10, 8, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<10, 8,uint8_t,t,t,t>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<11, 1, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<11, 1,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<11, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<11, 2,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<11, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<11, 3,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<11, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<11, 4,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<11, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<11, 5,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<11, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<11, 6,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<11, 7, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<11, 7,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<11, 8, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<11, 8,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<11, 9, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<11, 9,uint8_t,t,t,t>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12, 1, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<12, 1,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12, 2, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<12, 2,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<12, 3,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<12, 4,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<12, 5,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<12, 6,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12, 7, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<12, 7,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12, 8, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<12, 8,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12, 9, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<12, 9,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12,10, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<12,10,uint8_t,t,t,t>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<13, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<13, 3,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<13, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<13, 4,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<13, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<13, 5,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<13, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<13, 6,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<13, 7, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<13, 7,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<13, 8, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<13, 8,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<13, 9, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<13, 9,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<13, 10, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<13,10,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<13, 11, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<13,11,uint8_t,t,t,t>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<14, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<14, 3,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<14, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<14, 4,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<14, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<14, 5,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<14, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<14, 6,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<14, 7, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<14, 7,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<14, 8, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<14, 8,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<14, 9, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<14, 9,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<14, 10, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<14,10,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<14, 11, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<14,11,uint8_t,t,t,t>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<15, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<15, 3,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<15, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<15, 4,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<15, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<15, 5,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<15, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<15, 6,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<15, 7, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<15, 7,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<15, 8, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<15, 8,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<15, 9, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<15, 9,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<15, 10, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<15,10,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<15, 11, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<15,11,uint8_t,t,t,t>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<16, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<16, 3,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<16, 4, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<16, 4,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<16, 5, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<16, 5,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<16, 6, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<16, 6,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<16, 7, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<16, 7,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<16, 8, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<16, 8,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<16, 9, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<16, 9,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<16, 10, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<16,10,uint8_t,t,t,t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<16, 11, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(reportTestCases), "cfloat<16,11,uint8_t,t,t,t>", "multiplication");
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
