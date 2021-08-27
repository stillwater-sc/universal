// addition.cpp: test suite runner for addition on classic floats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// minimum set of include files to reflect source code dependencies
#define BLOCKTRIPLE_VERBOSE_OUTPUT
//#define BLOCKTRIPLE_TRACE_ADD
#include <universal/number/cfloat/cfloat_impl.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_case.hpp>
//#include <universal/verification/test_suite_arithmetic.hpp>
#include <universal/verification/cfloat_test_suite.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/number/cfloat/table.hpp>

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

/*
  Minimum number of operand bits for the adder = <abits> 
  to yield correctly rounded addition

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
	13   12  12  15  18  16  14  12  10   8   6   ?   -   -   -   -   -
	14   13  13  16  20  18  16  14  12  10   8   ?   ?   -   -   -   -
	15   14  14  17  22  20  18  16  14  12  10   ?   ?   ?   -   -   -
	16   15  15  18  24  22  20  18  16  14  12   ?   ?   ?   ?   -   -

*/

int main()
try {
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;
	std::string tag = "cfloat_ftf addition failed: ";

	// cfloat encoding configuration for the test
	constexpr bool hasSubnormals = false;
	constexpr bool hasSupernormals = true;
	constexpr bool isSaturating = false;

#if MANUAL_TESTING

	GenerateCfloatExponentBounds();

	// 9,176 0b0.0001.001 0b1.0110.000 0b1.0110.000 0b1.0101.111 -0.48242
	// FAIL          0.017578125 + -0.5 != -0.5 golden reference is - 0.46875 result 0b1.0110.000 vs ref 0b1.0101.111
	std::cout << "Manual Testing\n";
	{
		float fa = 0.017578125; // 0.375; // 0.3125f;  //  0.03125f; // 0.21875f; 
//		float fb = std::numeric_limits<float>::signaling_NaN();
//		float fb = std::numeric_limits<float>::quiet_NaN();
//		float fa = std::numeric_limits<float>::infinity();
		float fb = -0.5f; // 7.625f; // 0.0625f; 3.9375f; 

		using Cfloat = cfloat < 8, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating >;
		Cfloat a, b, c, cref;
		a.constexprClassParameters();
		a = fa;
		b = fb;
		c = a + b;
		std::cout << a << " + " << b << " = " << c << '\n';
		std::cout << to_binary(a) << " + " << to_binary(b) << " = " << to_binary(c) << '\n';

		TestCaseAdd< Cfloat, float>(fa, fb);
	}

	{
		float fa = std::numeric_limits<float>::infinity();
		float fb = -fa;
		std::cout << fa << " + " << fa << " = " << (fa + fa) << '\n';
		std::cout << fa << " + " << fb << " = " << (fa + fb) << '\n';
		std::cout << fb << " + " << fa << " = " << (fb + fa) << '\n';
		std::cout << fb << " + " << fb << " = " << (fb + fb) << '\n';
		std::cout << to_binary(fa + fb) << '\n';
	}

	{
		cfloat<8, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating > c(SpecificValue::maxpos);
		cfloat<9, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> d(SpecificValue::maxpos);
		std::cout << to_binary(c) << " : " << c << '\n';
		std::cout << to_binary(d) << " : " << d << '\n';
		d.setbits(0x0fa);
		std::cout << to_binary(d) << " : " << d << '\n';
		d.setbits(0x0fb);
		std::cout << to_binary(d) << " : " << d << '\n';

		std::cout << '\n';
		d = float(c);
		++d;
		std::cout << to_binary(d) << " : " << d << '\n';

		{
			cfloat<8,2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> c(SpecificValue::maxneg);
			cfloat<9,2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> d;
			d = double(c);
			std::cout << to_binary(d) << " : " << d << '\n';
			--d;
			std::cout << to_binary(d) << " : " << d << '\n';

		}
	}

	// generate individual testcases to hand trace/debug
	TestCaseAdd< cfloat<8, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float>(1.0f, 1.0f);

	TestCaseAdd< cfloat<16, 8, uint16_t, hasSubnormals, hasSupernormals, isSaturating>, double>(INFINITY, INFINITY);


	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatAddition< cfloat<8, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(true), "cfloat<8,2,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatAddition< cfloat<8, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(true), "cfloat<8,3,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatAddition< cfloat<8, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(true), "cfloat<8,4,uint8_t,f,t,f>", "addition");

	std::cout << "Number of failed test cases : " << nrOfFailedTestCases << std::endl;
	nrOfFailedTestCases = 0; // disregard any test failures in manual testing mode

#else
	std::cout << "classic cfloat_fff addition validation\n";

	bool bReportIndividualTestCases = false;

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<3, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 3, 1,uint8_t,f,t,f>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<4, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 4, 1,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<4, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 4, 2,uint8_t,f,t,f>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<5, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 5, 1,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<5, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 5, 2,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<5, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 5, 3,uint8_t,f,t,f>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<6, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 6, 1,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<6, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 6, 2,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<6, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 6, 3,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<6, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 6, 4,uint8_t,f,t,f>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<7, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 7, 1,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<7, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 7, 2,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<7, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 7, 3,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<7, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 7, 4,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<7, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 7, 5,uint8_t,f,t,f>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<8, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 8, 1,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<8, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 8, 2,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<8, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 8, 3,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<8, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 8, 4,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<8, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 8, 5,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<8, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 8, 6,uint8_t,f,t,f>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<9, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 9, 1,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<9, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 9, 2,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<9, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 9, 3,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<9, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 9, 4,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<9, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 9, 5,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<9, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 9, 6,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<9, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 9, 7,uint8_t,f,t,f>", "addition");

#if STRESS_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<10, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<10, 1,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<10, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<10, 2,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<10, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<10, 3,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<10, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<10, 4,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<10, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<10, 5,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<10, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<10, 6,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<10, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<10, 7,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<10, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<10, 8,uint8_t,f,t,f>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<11, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<11, 1,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<11, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<11, 2,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<11, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<11, 3,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<11, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<11, 4,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<11, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<11, 5,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<11, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<11, 6,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<11, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<11, 7,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<11, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<11, 8,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<11, 9, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<11, 9,uint8_t,f,t,f>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<12, 1,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<12, 2,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<12, 3,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<12, 4,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<12, 5,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<12, 6,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<12, 7,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<12, 8,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12, 9, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<12, 9,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12,10, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<12,10,uint8_t,f,t,f>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<13, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<13, 3,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<13, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<13, 4,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<13, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<13, 5,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<13, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<13, 6,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<13, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<13, 7,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<13, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<13, 8,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<13, 9, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<13, 9,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<13, 10, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<13,10,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<13, 11, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<13,11,uint8_t,f,t,f>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<14, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<14, 3,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<14, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<14, 4,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<14, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<14, 5,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<14, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<14, 6,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<14, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<14, 7,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<14, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<14, 8,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<14, 9, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<14, 9,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<14, 10, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<14,10,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<14, 11, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<14,11,uint8_t,f,t,f>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<15, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<15, 3,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<15, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<15, 4,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<15, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<15, 5,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<15, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<15, 6,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<15, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<15, 7,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<15, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<15, 8,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<15, 9, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<15, 9,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<15, 10, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<15,10,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<15, 11, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<15,11,uint8_t,f,t,f>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<16, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<16, 3,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<16, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<16, 4,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<16, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<16, 5,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<16, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<16, 6,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<16, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<16, 7,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<16, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<16, 8,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<16, 9, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<16, 9,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<16, 10, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<16,10,uint8_t,f,t,f>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<16, 11, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<16,11,uint8_t,f,t,f>", "addition");

#endif  // STRESS_TESTING


#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::cfloat_divide_by_zero& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
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
