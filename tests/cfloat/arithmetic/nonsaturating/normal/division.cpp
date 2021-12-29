// division.cpp: test suite runner for division on classic floats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// minimum set of include files to reflect source code dependencies
#define CFLOAT_VERBOSE_OUTPUT
#define CFLOAT_TRACE_DIV
#define BLOCKTRIPLE_VERBOSE_OUTPUT
#define BLOCKTRIPLE_TRACE_DIV
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/cfloat_test_suite.hpp>

void ReportIeee754NotANumberArithmetic()
{ // special cases of snan/qnan
	constexpr float fa = std::numeric_limits<float>::quiet_NaN();
	constexpr float fb = -std::numeric_limits<float>::signaling_NaN();
	std::cout << "quiet NaN      : " << sw::universal::to_binary(fa) << " : " << fa << '\n';
	std::cout << "signalling NaN : " << sw::universal::to_binary(fb) << " : " << fb << '\n';
	std::cout << fa << " / " << fa << " = " << (fa / fa) << '\n';
	std::cout << fa << " / " << fb << " = " << (fa / fb) << '\n';
	std::cout << fb << " / " << fa << " = " << (fb / fa) << '\n';
	std::cout << fb << " / " << fb << " = " << (fb / fb) << '\n';
	std::cout << sw::universal::to_binary(fa / fb) << '\n';
}

/*
   0 /  inf =    0 : 0b0.00000000.00000000000000000000000
   0 / -inf =   -0 : 0b1.00000000.00000000000000000000000
   1 /  inf =    0 : 0b0.00000000.00000000000000000000000
   1 / -inf =   -0 : 0b1.00000000.00000000000000000000000
 inf /    0 =  inf : 0b0.11111111.00000000000000000000000
 inf /   -0 = -inf : 0b1.11111111.00000000000000000000000
-inf /    0 = -inf : 0b1.11111111.00000000000000000000000
-inf /   -0 =  inf : 0b0.11111111.00000000000000000000000
 inf /  inf = -nan(ind) : 0b1.11111111.10000000000000000000000
 inf / -inf = -nan(ind) : 0b1.11111111.10000000000000000000000
-inf /  inf = -nan(ind) : 0b1.11111111.10000000000000000000000
-inf / -inf = -nan(ind) : 0b1.11111111.10000000000000000000000
   0 /  inf =  0
 */

void ReportIeee754InfinityArithmetic()
{ // special cases of +-inf
	constexpr float fa = std::numeric_limits<float>::infinity();
	float fb = -fa;
	float zero = 0.0f;
	std::cout << 0.0f << " / " << fa << " = " << (0.0f / fa) << " : " << sw::universal::to_binary(0.0f / fa) << '\n';
	std::cout << 0.0f << " / " << fb << " = " << (0.0f / fb) << " : " << sw::universal::to_binary(0.0f / fb) << '\n';
	std::cout << 1.0f << " / " << fa << " = " << (1.0f / fa) << " : " << sw::universal::to_binary(1.0f / fa) << '\n';
	std::cout << 1.0f << " / " << fb << " = " << (1.0f / fb) << " : " << sw::universal::to_binary(1.0f / fb) << '\n';
	std::cout << fa << " / " <<  0.0f << " = " << (fa / zero) << " : " << sw::universal::to_binary(fa / zero) << '\n';
	std::cout << fa << " / " << -0.0f << " = " << (fa / -zero) << " : " << sw::universal::to_binary(fa / -zero) << '\n';
	std::cout << fb << " / " << 0.0f << " = " << (fb / zero) << " : " << sw::universal::to_binary(fb / zero) << '\n';
	std::cout << fb << " / " << -0.0f << " = " << (fb / -zero) << " : " << sw::universal::to_binary(fb / -zero) << '\n';
	std::cout << fa << " / " << fa << " = " << (fa / fa) << " : " << sw::universal::to_binary(fa / fa) << '\n';
	std::cout << fa << " / " << fb << " = " << (fa / fb) << " : " << sw::universal::to_binary(fa / fb) << '\n';
	std::cout << fb << " / " << fa << " = " << (fb / fa) << " : " << sw::universal::to_binary(fb / fa) << '\n';
	std::cout << fb << " / " << fb << " = " << (fb / fb) << " : " << sw::universal::to_binary(fb / fb) << '\n';
	std::cout << 0.0f << " / " << fa << " = " << (0.0f / fa) << '\n';
	std::cout << sw::universal::to_binary(fa * fb) << '\n';
}

/*
 0 /  0 = -nan(ind) : 0b1.11111111.10000000000000000000000
 0 / -0 = -nan(ind) : 0b1.11111111.10000000000000000000000
-0 /  0 = -nan(ind) : 0b1.11111111.10000000000000000000000
-0 / -0 = -nan(ind) : 0b1.11111111.10000000000000000000000

 1 /  0 =  inf : 0b0.11111111.00000000000000000000000
 1 / -0 = -inf : 0b1.11111111.00000000000000000000000
-1 /  0 = -inf : 0b1.11111111.00000000000000000000000
-1 / -0 =  inf : 0b0.11111111.00000000000000000000000

 0 /  1 =  0 : 0b0.00000000.00000000000000000000000
 0 / -1 = -0 : 0b1.00000000.00000000000000000000000
-0 /  1 = -0 : 0b1.00000000.00000000000000000000000
-0 / -1 =  0 : 0b0.00000000.00000000000000000000000
 */

void ReportIeee754SpecialCases()
{
	float fa = 0.0f;
	float fb = 0.0f;
	std::cout <<  fa << " / " <<  fb << " = " << ( fa /  fb) << " : " << sw::universal::to_binary( fa /  fb) << '\n';
	std::cout <<  fa << " / " << -fb << " = " << ( fa / -fb) << " : " << sw::universal::to_binary( fa / -fb) << '\n';
	std::cout << -fa << " / " <<  fb << " = " << (-fa /  fb) << " : " << sw::universal::to_binary(-fa /  fb) << '\n';	
	std::cout << -fa << " / " << -fb << " = " << (-fa / -fb) << " : " << sw::universal::to_binary(-fa / -fb) << '\n';
	fa = 1.0f;
	std::cout <<  fa << " / " <<  fb << " = " << ( fa /  fb) << " : " << sw::universal::to_binary( fa /  fb) << '\n';
	std::cout <<  fa << " / " << -fb << " = " << ( fa / -fb) << " : " << sw::universal::to_binary( fa / -fb) << '\n';
	std::cout << -fa << " / " <<  fb << " = " << (-fa /  fb) << " : " << sw::universal::to_binary(-fa /  fb) << '\n';
	std::cout << -fa << " / " << -fb << " = " << (-fa / -fb) << " : " << sw::universal::to_binary(-fa / -fb) << '\n';
	fa = 0.0f;
	fb = 1.0f;
	std::cout <<  fa << " / " <<  fb << " = " << ( fa /  fb) << " : " << sw::universal::to_binary( fa /  fb) << '\n';
	std::cout <<  fa << " / " << -fb << " = " << ( fa / -fb) << " : " << sw::universal::to_binary( fa / -fb) << '\n';
	std::cout << -fa << " / " <<  fb << " = " << (-fa /  fb) << " : " << sw::universal::to_binary(-fa /  fb) << '\n';
	std::cout << -fa << " / " << -fb << " = " << (-fa / -fb) << " : " << sw::universal::to_binary(-fa / -fb) << '\n';
}

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
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	// cfloat encoding configuration for the test
	constexpr bool hasSubnormals   = false;
	constexpr bool hasSupernormals = false;
	constexpr bool isSaturating    = false;

	std::string test_suite         = "classic cfloat division validation with just normals, no subnormals or supernormals";
	std::string test_tag           = "cfloat_fff division";
	bool reportTestCases           = true;
	int nrOfFailedTestCases        = 0;

	std::cout << test_suite << '\n';

#if MANUAL_TESTING

//	ReportIeee754InfinityArithmetic();
//	ReportIeee754NotANumberArithmetic();
//	ReportIeee754SpecialCases();

	TestCase< cfloat<6, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(TestCaseOperator::DIV, 1.0f, 1.5f);
	return 0;
	TestCase< cfloat<6, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(TestCaseOperator::DIV, 1.0f, -1.0f);
	TestCase< cfloat<6, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(TestCaseOperator::DIV, 1.625f, -1.625f);

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision<cfloat<4, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<4,2,uint8_t,f,f,f>", "division");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision<cfloat<5, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<5,2,uint8_t,f,f,f>", "division");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision<cfloat<6, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<6,2,uint8_t,f,f,f>", "division");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<3, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 3, 1,uint8_t,f,f,f>", "division");

//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<4, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 4, 1,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<4, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 4, 2,uint8_t,f,f,f>", "division");

//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<5, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 5, 1,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<5, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 5, 2,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<5, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 5, 3,uint8_t,f,f,f>", "division");

//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<6, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 6, 1,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<6, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 6, 2,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<6, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 6, 3,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<6, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 6, 4,uint8_t,f,f,f>", "division");

//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<7, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 7, 1,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<7, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 7, 2,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<7, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 7, 3,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<7, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 7, 4,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<7, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 7, 5,uint8_t,f,f,f>", "division");

//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<8, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 8, 1,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<8, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 8, 2,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<8, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 8, 3,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<8, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 8, 4,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<8, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 8, 5,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<8, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 8, 6,uint8_t,f,f,f>", "division");
#endif

#if REGRESSION_LEVEL_2
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<9, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 9, 1,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<9, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 9, 2,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<9, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 9, 3,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<9, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 9, 4,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<9, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 9, 5,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<9, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 9, 6,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<9, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat< 9, 7,uint8_t,f,f,f>", "division");

//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<10, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<10, 1,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<10, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<10, 2,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<10, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<10, 3,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<10, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<10, 4,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<10, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<10, 5,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<10, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<10, 6,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<10, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<10, 7,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<10, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<10, 8,uint8_t,f,f,f>", "division");

//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<11, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<11, 1,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<11, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<11, 2,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<11, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<11, 3,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<11, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<11, 4,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<11, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<11, 5,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<11, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<11, 6,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<11, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<11, 7,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<11, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<11, 8,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<11, 9, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<11, 9,uint8_t,f,f,f>", "division");
#endif

#if REGRESSION_LEVEL_3
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<12, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12, 1,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<12, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12, 2,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<12, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12, 3,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<12, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12, 4,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<12, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12, 5,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<12, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12, 6,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<12, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12, 7,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<12, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12, 8,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<12, 9, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12, 9,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<12,10, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<12,10,uint8_t,f,f,f>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<13, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<13, 3,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<13, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<13, 4,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<13, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<13, 5,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<13, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<13, 6,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<13, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<13, 7,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<13, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<13, 8,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<13, 9, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<13, 9,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<13, 10, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<13,10,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<13, 11, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<13,11,uint8_t,f,f,f>", "division");
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<14, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<14, 3,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<14, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<14, 4,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<14, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<14, 5,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<14, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<14, 6,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<14, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<14, 7,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<14, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<14, 8,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<14, 9, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<14, 9,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<14, 10, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<14,10,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<14, 11, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<14,11,uint8_t,f,f,f>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<15, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<15, 3,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<15, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<15, 4,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<15, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<15, 5,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<15, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<15, 6,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<15, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<15, 7,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<15, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<15, 8,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<15, 9, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<15, 9,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<15, 10, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<15,10,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<15, 11, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<15,11,uint8_t,f,f,f>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<16, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<16, 3,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<16, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<16, 4,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<16, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<16, 5,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<16, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<16, 6,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<16, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<16, 7,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<16, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<16, 8,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<16, 9, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<16, 9,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<16, 10, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<16,10,uint8_t,f,f,f>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatDivision< cfloat<16, 11, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases), "cfloat<16,11,uint8_t,f,f,f>", "division");
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
