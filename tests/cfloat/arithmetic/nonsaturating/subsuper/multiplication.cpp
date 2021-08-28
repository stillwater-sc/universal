// multiplication.cpp: test suite runner for multiplication on classic floats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// minimum set of include files to reflect source code dependencies
#define CFLOAT_VERBOSE_OUTPUT
#define CFLOAT_TRACE_MUL
#include <universal/number/cfloat/cfloat_impl.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_case.hpp>
//#include <universal/verification/test_suite_arithmetic.hpp>
#include <universal/verification/cfloat_test_suite.hpp>
#include <universal/utility/bit_cast.hpp>
//#include <universal/number/cfloat/table.hpp>

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main()
try {
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;
	std::string tag = "cfloat_ttf multiplication failed: ";

	// cfloat encoding configuration for the test
	constexpr bool hasSubnormals = true;
	constexpr bool hasSupernormals = true;
	constexpr bool isSaturating = false;

#if MANUAL_TESTING

	std::cout << "Manual Testing\n";

	{
		float fa = 0.25f; 
//		float fb = std::numeric_limits<float>::signaling_NaN();
//		float fb = std::numeric_limits<float>::quiet_NaN();
//		float fb = std::numeric_limits<float>::infinity();
		float fb = 0.75f;
		float fc = fa * fb;

		constexpr size_t nbits = 5;
		constexpr size_t es = 2;
		using Cfloat = cfloat < nbits, es, uint8_t, hasSubnormals, hasSupernormals, isSaturating >;

		TestCase< Cfloat, float>(TestCaseOperator::MUL, fa, fb);
		Cfloat c; 
		// c.constexprClassParameters();
		c = fc;
		std::cout << "c = " << c << " : " << to_binary(c) << '\n';
	}
	return 0;
	{ // special cases of snan/qnan
		constexpr float fa = std::numeric_limits<float>::quiet_NaN();
		constexpr float fb = std::numeric_limits<float>::signaling_NaN();
		std::cout << fa << " * " << fa << " = " << (fa * fa) << '\n';
		std::cout << fa << " * " << fb << " = " << (fa * fb) << '\n';
		std::cout << fb << " * " << fa << " = " << (fb * fa) << '\n';
		std::cout << fb << " * " << fb << " = " << (fb * fb) << '\n';
		std::cout << to_binary(fa * fb) << '\n';
	}

	{ // special cases of +-inf
		constexpr float fa = std::numeric_limits<float>::infinity();
		float fb = -fa;
		std::cout << fa << " * " << fa << " = " << (fa * fa) << '\n';
		std::cout << fa << " * " << fb << " = " << (fa * fb) << '\n';
		std::cout << fb << " * " << fa << " = " << (fb * fa) << '\n';
		std::cout << fb << " * " << fb << " = " << (fb * fb) << '\n';
		std::cout << 0.0f << " * " << fa << " = " << (0.0f * fa) << '\n';
		std::cout << to_binary(fa * fb) << '\n';
	}

	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatMultiplication< 
		cfloat<5, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(true), 
		"cfloat<5,2,uint8_t,t,t,f>", 
		"multiplication");
//	nrOfFailedTestCases += ReportTestResult(
//		VerifyCfloatMultiplication<
//		cfloat<4, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(true),
//		"cfloat<4,1,uint8_t,t,t,f>",
//		"multiplication");

	std::cout << "Number of failed test cases : " << nrOfFailedTestCases << std::endl;
	nrOfFailedTestCases = 0; // disregard any test failures in manual testing mode

#else
	std::cout << "classic floating-point multiplication validation\n";

	bool bReportIndividualTestCases = false;
	constexpr bool hasSubnormals = true;
	constexpr bool hasSupernormals = true;
	constexpr bool isSaturating = true;

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<3, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 3, 1,uint8_t,t,t,f>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<4, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 4, 1,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<4, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 4, 2,uint8_t,t,t,f>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<5, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 5, 1,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<5, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 5, 2,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<5, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 5, 3,uint8_t,t,t,f>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<6, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 6, 1,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<6, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 6, 2,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<6, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 6, 3,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<6, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 6, 4,uint8_t,t,t,f>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<7, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 7, 1,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<7, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 7, 2,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<7, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 7, 3,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<7, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 7, 4,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<7, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 7, 5,uint8_t,t,t,f>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<8, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 8, 1,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<8, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 8, 2,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<8, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 8, 3,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<8, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 8, 4,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<8, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 8, 5,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<8, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 8, 6,uint8_t,t,t,f>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<9, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 9, 1,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<9, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 9, 2,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<9, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 9, 3,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<9, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 9, 4,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<9, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 9, 5,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<9, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 9, 6,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<9, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat< 9, 7,uint8_t,t,t,f>", "multiplication");

#if STRESS_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<10, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<10, 1,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<10, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<10, 2,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<10, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<10, 3,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<10, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<10, 4,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<10, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<10, 5,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<10, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<10, 6,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<10, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<10, 7,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<10, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<10, 8,uint8_t,t,t,f>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<11, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<11, 1,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<11, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<11, 2,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<11, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<11, 3,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<11, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<11, 4,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<11, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<11, 5,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<11, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<11, 6,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<11, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<11, 7,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<11, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<11, 8,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<11, 9, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<11, 9,uint8_t,t,t,f>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<12, 1,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<12, 2,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<12, 3,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<12, 4,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<12, 5,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<12, 6,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<12, 7,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<12, 8,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12, 9, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<12, 9,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<12,10, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<12,10,uint8_t,t,t,f>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<13, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<13, 3,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<13, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<13, 4,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<13, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<13, 5,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<13, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<13, 6,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<13, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<13, 7,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<13, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<13, 8,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<13, 9, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<13, 9,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<13, 10, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<13,10,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<13, 11, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<13,11,uint8_t,t,t,f>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<14, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<14, 3,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<14, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<14, 4,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<14, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<14, 5,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<14, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<14, 6,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<14, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<14, 7,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<14, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<14, 8,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<14, 9, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<14, 9,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<14, 10, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<14,10,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<14, 11, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<14,11,uint8_t,t,t,f>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<15, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<15, 3,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<15, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<15, 4,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<15, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<15, 5,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<15, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<15, 6,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<15, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<15, 7,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<15, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<15, 8,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<15, 9, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<15, 9,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<15, 10, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<15,10,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<15, 11, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<15,11,uint8_t,t,t,f>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<16, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<16, 3,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<16, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<16, 4,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<16, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<16, 5,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<16, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<16, 6,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<16, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<16, 7,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<16, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<16, 8,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<16, 9, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<16, 9,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<16, 10, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<16,10,uint8_t,t,t,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<16, 11, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(bReportIndividualTestCases), "cfloat<16,11,uint8_t,t,t,f>", "multiplication");

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
