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
//#include <universal/verification/test_suite_arithmetic.hpp>
#include <universal/verification/cfloat_test_suite.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/number/cfloat/table.hpp>

// generate specific test case that you can trace with the trace conditions in cfloat.hpp
// for most bugs they are traceable with _trace_conversion and _trace_add
template<typename cfloatConfiguration, typename Ty>
void GenerateTestCase(Ty _a, Ty _b) {
	constexpr size_t nbits = cfloatConfiguration::nbits;
	cfloatConfiguration a, b, sum, ref;
	a = _a;
	b = _b;
	sum = a + b;
	// generate the reference
	Ty reference = _a + _b;
	ref = reference;

	std::cout << std::setprecision(10);
//	std::cout << "native: " << std::setw(nbits) << _a << " + " << std::setw(nbits) << _b << " = " << std::setw(nbits) << reference << std::endl;
	Ty _c{ reference };
	std::cout << sw::universal::to_binary(_a) << " : " << _a << '\n';
	std::cout << sw::universal::to_binary(_b) << " : " << _b << '\n';
	std::cout << sw::universal::to_binary(_c) << " : " << _c << '\n';
	std::cout << a << " + " << b << " = " << sum << " (reference: " << ref << ")   ";
	std::cout << to_binary(a, true) << " + " << to_binary(b, true) << " = " << to_binary(sum, true) << " (reference: " << to_binary(ref, true) << ")   ";
	std::cout << (ref == sum ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
}

template<typename Real>
void test754functions(Real value) {
	using namespace std;
	using namespace sw::universal;
	cout << to_hex(value) << '\n';
	cout << to_binary(value) << '\n';
	cout << to_triple(value) << '\n';
	cout << to_base2_scientific(value) << '\n';
	cout << color_print(value) << '\n';
}

void TableCfloatExponentBounds()
{
	// max exp values as a function of es
	constexpr int WIDTH = 15;
	std::cout <<
		std::setw(WIDTH) << "es" <<
		std::setw(WIDTH) << "RAW_MAX_EXP" <<
		std::setw(WIDTH) << "EXP_BIAS" <<
		std::setw(WIDTH) << "MAX_EXP" <<
		std::setw(WIDTH) << "MIN_EXP_NORMAL" <<
		std::setw(WIDTH) << "MIN_NORMAL"
		<< '\n';
	for (size_t es = 1; es < 20; ++es) {
		int EXP_BIAS = ((1l << (es - 1ull)) - 1l);
		int RAW_MAX_EXP = (es == 1) ? 1 : ((1l << es) - 1);
		int MAX_EXP = (es == 1) ? 1 : ((1l << es) - EXP_BIAS - 1);
		int MIN_EXP_NORMAL = 1 - EXP_BIAS;
		double MIN_NORMAL = std::pow(2.0, MIN_EXP_NORMAL);
		// MIN_EXP_SUBNORMAL = 1 - EXP_BIAS - int(fbits); // the scale of smallest ULP
		std::cout <<
			std::setw(WIDTH) << es <<
			std::setw(WIDTH) << RAW_MAX_EXP <<
			std::setw(WIDTH) << EXP_BIAS <<
			std::setw(WIDTH) << MAX_EXP <<
			std::setw(WIDTH) << MIN_EXP_NORMAL <<
			std::setw(WIDTH) << MIN_NORMAL <<
			'\n';
	}
}

template<typename Cfloat>
void testCfloatOrderedSet() {
	std::vector<Cfloat> set;
	GenerateOrderedCfloatSet<Cfloat>(set);
	for (auto v : set) {
		std::cout << to_binary(v) << " : " << v << '\n';
	}
}

#define MANUAL_TESTING 0
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
	16   15  15  18  22  22  20  18  16  14  12   ?   ?   ?   ?   -   -

*/

int main()
try {
	using namespace std;
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;
	std::string tag = "Addition failed: ";

#if MANUAL_TESTING

	TableCfloatExponentBounds();

	// 9,176 0b0.0001.001 0b1.0110.000 0b1.0110.000 0b1.0101.111 -0.48242
	// FAIL          0.017578125 + -0.5 != -0.5 golden reference is - 0.46875 result 0b1.0110.000 vs ref 0b1.0101.111
	std::cout << "Manual Testing\n";
	{
		float fa = 0.017578125; // 0.375; // 0.3125f;  //  0.03125f; // 0.21875f; 
//		float fb = std::numeric_limits<float>::signaling_NaN();
//		float fb = std::numeric_limits<float>::quiet_NaN();
//		float fa = std::numeric_limits<float>::infinity();
		float fb = -0.5f; // 7.625f; // 0.0625f; 3.9375f; 

		cfloat < 8, 4, uint8_t > a, b, c, cref;
		a.constexprClassParameters();
		a = fa;
		b = fb;
		c = a + b;
		std::cout << a << " + " << b << " = " << c << '\n';
		std::cout << to_binary(a) << " + " << to_binary(b) << " = " << to_binary(c) << '\n';

		GenerateTestCase< cfloat<8, 4, uint8_t>, float>(fa, fb);
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
#ifdef LATER
	{
		cfloat<8, 2> c(SpecificValue::maxpos);
		cfloat<9, 2> d(SpecificValue::maxpos);
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
			cfloat<8,2> c(SpecificValue::maxneg);
			cfloat<9,2> d;
			d = double(c);
			std::cout << to_binary(d) << " : " << d << '\n';
			--d;
			std::cout << to_binary(d) << " : " << d << '\n';

		}
	}

	std::cout << "single precision IEEE-754\n";
	float f = 1.06125f;
	test754functions(f);
	std::cout << "double precision IEEE-754\n";
	double d = 1.06125;
	test754functions(d);

	// generate individual testcases to hand trace/debug
	GenerateTestCase< cfloat<8, 2, uint8_t>, float>(1.0f, 1.0f);

	GenerateTestCase< cfloat<16, 8, uint16_t>, double>(INFINITY, INFINITY);
#endif

	constexpr bool hasSubnormals = true;
	constexpr bool hasSupernormals = true;
	constexpr bool isSaturating = true;
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatAddition< cfloat<8, 2, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(true), "cfloat<8,2,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatAddition< cfloat<8, 3, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(true), "cfloat<8,3,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatAddition< cfloat<8, 4, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(true), "cfloat<8,4,uint8_t,subnormals,supernormals,!saturating>", "addition");

	std::cout << "Number of failed test cases : " << nrOfFailedTestCases << std::endl;
	nrOfFailedTestCases = 0; // disregard any test failures in manual testing mode

#else
	cout << "classic floating-point addition validation" << endl;

	bool bReportIndividualTestCases = false;
	constexpr bool hasSubnormals = true;
	constexpr bool hasSupernormals = true;
	constexpr bool isSaturating = true;

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<3, 1, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat< 3, 1,uint8_t,subnormals,supernormals,!saturating>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<4, 1, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat< 4, 1,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<4, 2, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat< 4, 2,uint8_t,subnormals,supernormals,!saturating>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<5, 1, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat< 5, 1,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<5, 2, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat< 5, 2,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<5, 3, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat< 5, 3,uint8_t,subnormals,supernormals,!saturating>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<6, 1, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat< 6, 1,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<6, 2, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat< 6, 2,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<6, 3, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat< 6, 3,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<6, 4, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat< 6, 4,uint8_t,subnormals,supernormals,!saturating>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<7, 1, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat< 7, 1,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<7, 2, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat< 7, 2,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<7, 3, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat< 7, 3,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<7, 4, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat< 7, 4,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<7, 5, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat< 7, 5,uint8_t,subnormals,supernormals,!saturating>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<8, 1, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat< 8, 1,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<8, 2, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat< 8, 2,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<8, 3, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat< 8, 3,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<8, 4, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat< 8, 4,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<8, 5, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat< 8, 5,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<8, 6, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat< 8, 6,uint8_t,subnormals,supernormals,!saturating>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<9, 1, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat< 9, 1,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<9, 2, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat< 9, 2,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<9, 3, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat< 9, 3,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<9, 4, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat< 9, 4,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<9, 5, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat< 9, 5,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<9, 6, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat< 9, 6,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<9, 7, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat< 9, 7,uint8_t,subnormals,supernormals,!saturating>", "addition");

#if STRESS_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<10, 1, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<10, 1,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<10, 2, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<10, 2,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<10, 3, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<10, 3,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<10, 4, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<10, 4,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<10, 5, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<10, 5,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<10, 6, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<10, 6,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<10, 7, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<10, 7,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<10, 8, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<10, 8,uint8_t,subnormals,supernormals,!saturating>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<11, 1, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<11, 1,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<11, 2, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<11, 2,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<11, 3, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<11, 3,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<11, 4, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<11, 4,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<11, 5, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<11, 5,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<11, 6, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<11, 6,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<11, 7, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<11, 7,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<11, 8, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<11, 8,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<11, 9, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<11, 9,uint8_t,subnormals,supernormals,!saturating>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12, 1, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<12, 1,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12, 2, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<12, 2,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12, 3, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<12, 3,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12, 4, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<12, 4,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12, 5, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<12, 5,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12, 6, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<12, 6,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12, 7, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<12, 7,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12, 8, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<12, 8,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12, 9, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<12, 9,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<12,10, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<12,10,uint8_t,subnormals,supernormals,!saturating>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<13, 3, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<13, 3,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<13, 4, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<13, 4,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<13, 5, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<13, 5,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<13, 6, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<13, 6,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<13, 7, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<13, 7,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<13, 8, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<13, 8,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<13, 9, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<13, 9,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<13, 10, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<13,10,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<13, 11, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<13,11,uint8_t,subnormals,supernormals,!saturating>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<14, 3, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<14, 3,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<14, 4, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<14, 4,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<14, 5, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<14, 5,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<14, 6, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<14, 6,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<14, 7, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<14, 7,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<14, 8, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<14, 8,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<14, 9, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<14, 9,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<14, 10, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<14,10,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<14, 11, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<14,11,uint8_t,subnormals,supernormals,!saturating>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<15, 3, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<15, 3,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<15, 4, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<15, 4,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<15, 5, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<15, 5,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<15, 6, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<15, 6,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<15, 7, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<15, 7,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<15, 8, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<15, 8,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<15, 9, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<15, 9,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<15, 10, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<15,10,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<15, 11, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<15,11,uint8_t,subnormals,supernormals,!saturating>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<16, 3, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<16, 3,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<16, 4, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<16, 4,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<16, 5, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<16, 5,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<16, 6, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<16, 6,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<16, 7, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<16, 7,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<16, 8, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<16, 8,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<16, 9, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<16, 9,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<16, 10, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<16,10,uint8_t,subnormals,supernormals,!saturating>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatAddition< cfloat<16, 11, uint8_t, hasSubnormals, hasSupernormals, !isSaturating> >(bReportIndividualTestCases), "cfloat<16,11,uint8_t,subnormals,supernormals,!saturating>", "addition");

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
