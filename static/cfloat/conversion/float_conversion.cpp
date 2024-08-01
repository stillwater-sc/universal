// float_conversion.cpp: test suite runner for IEEE float conversions to classic cfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>
// Configure the cfloat template environment
// first: enable general or specialized configurations
#define CFLOAT_FAST_SPECIALIZATION 0
// second: enable/disable arithmetic exceptions
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 0
// third: enable trace conversion
#define TRACE_CONVERSION 0
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/cfloat_test_suite.hpp>
//#include <universal/number/cfloat/table.hpp> // only used for value table generation

#if BIT_CAST_IS_CONSEXPR
void ToNativeBug() {  // now resolved... exponentiation was incorrect
	using namespace sw::universal;
	constexpr size_t nbits = 32;
	constexpr size_t es = 8;
	using bt = uint32_t;
	constexpr bool hasSubnormals = true;
	constexpr bool hasSupernormals = true;
	constexpr bool isSaturating = false;
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> a{}, b{};
	// b1.00111111.00011001011010001001001 != b1.01111111.00011001011010001001001
	a.assign("0b1.00111111.00011001011010001001001");
	std::cout << "cfloat   : " << to_binary(a) << '\n';
	float f = float(a);
	std::cout << "float    : " << to_binary(f) << '\n';
	b = f;
	std::cout << "cfloat b : " << to_binary(b) << '\n';

	//blockbinary<nbits, bt> bits;
	//a.getbits(bits);
	//std::cout << "bits     : " << to_binary(bits, false) << '\n';

	// bit cast
	uint32_t bc = std::bit_cast<uint32_t, float>(f);
	std::cout << "float    : " << to_binary(f) << '\n';
	std::cout << "smask    : " << to_binary(ieee754_parameter<float>::smask, 32, false) << '\n';
	std::cout << "emask    : " << to_binary(ieee754_parameter<float>::emask, 32, false) << '\n';
	std::cout << "fmask    : " << to_binary(ieee754_parameter<float>::fmask, 32, false) << '\n';
	std::cout << "smask+bc : " << to_binary((ieee754_parameter<float>::smask & bc), 32, false) << '\n';
	std::cout << "emask+bc : " << to_binary((ieee754_parameter<float>::emask & bc), 32, false) << '\n';
	std::cout << "fmask+bc : " << to_binary((ieee754_parameter<float>::fmask & bc), 32, false) << '\n';
//	uint32_t rawSignbits     = (ieee754_parameter<float>::smask & bc);
	uint32_t rawExponentBits = (ieee754_parameter<float>::emask & bc) >> ieee754_parameter<float>::fbits;
	uint32_t rawFractionBits = (ieee754_parameter<float>::fmask & bc);
	std::cout << "raw exp  : " << to_binary(rawExponentBits) << '\n';
	std::cout << "raw frac : " << to_binary(rawFractionBits) << '\n';
}
#endif // BIT_CAST_IS_CONSTEXPR

/*
	b0.00000000.00000000000000000000001 : 1.401298464324817e-45
	b0.00000000.00000000000000000000010 : 2.802596928649634e-45
	b0.00000000.00000000000000000000100 : 5.605193857299268e-45
	b0.00000000.00000000000000000001000 : 1.121038771459854e-44
	b0.00000000.00000000000000000010000 : 2.242077542919707e-44
	b0.00000000.00000000000000000100000 : 4.484155085839415e-44
	b0.00000000.00000000000000001000000 : 8.968310171678829e-44
	b0.00000000.00000000000000010000000 : 1.793662034335766e-43
	b0.00000000.00000000000000100000000 : 3.587324068671532e-43
	b0.00000000.00000000000001000000000 : 7.174648137343063e-43
	b0.00000000.00000000000010000000000 : 1.434929627468613e-42
	b0.00000000.00000000000100000000000 : 2.869859254937225e-42
	b0.00000000.00000000001000000000000 : 5.739718509874451e-42
	b0.00000000.00000000010000000000000 : 1.147943701974890e-41
	b0.00000000.00000000100000000000000 : 2.295887403949780e-41
	b0.00000000.00000001000000000000000 : 4.591774807899561e-41
	b0.00000000.00000010000000000000000 : 9.183549615799121e-41
	b0.00000000.00000100000000000000000 : 1.836709923159824e-40
	b0.00000000.00001000000000000000000 : 3.673419846319648e-40
	b0.00000000.00010000000000000000000 : 7.346839692639297e-40
	b0.00000000.00100000000000000000000 : 1.469367938527859e-39
	b0.00000000.01000000000000000000000 : 2.938735877055719e-39
	b0.00000000.10000000000000000000000 : 5.877471754111438e-39
	b0.00000001.00000000000000000000000 : 1.175494350822288e-38
	b0.00000010.00000000000000000000000 : 2.350988701644575e-38
*/

void GenerateSinglePrecisionSubnormals() 
{
	using namespace sw::universal;
	constexpr size_t nbits = 32;
	constexpr size_t es = 8;
	using bt = uint32_t;
	constexpr bool hasSubnormals = true;
	constexpr bool hasSupernormals = true;
	constexpr bool isSaturating = false;
	cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> a{};
	++a;
	float f = float(a);
	std::cout << std::setprecision(16);
	std::cout << to_binary(a) << " : " << a << '\n';
	std::cout << to_binary(f) << " : " << f << '\n';
	for (int i = 0; i < 24; ++i) {
		f *= 2;
		std::cout << to_binary(f) << " : " << f << '\n';
	}
	for (int i = 0; i < 24; ++i) {
		f = ieee754_float_subnormals[i];
		std::cout << to_binary(f) << " : " << f << '\n';
	}
	std::cout << std::setprecision(5);
}

template<typename cfloatType>
void Test1() 
{
	cfloatType a{};
	a.constexprClassParameters();

	float testValue = 8.0f;
	a = testValue;
	float f = float(a);
	std::cout << to_binary(a) << " : " << a << " : " << f << " : " << std::setprecision(8) << testValue << '\n';
}

template<typename cfloatType>
void Test2()
{
	using namespace sw::universal;
	
	cfloat<8, 6, uint8_t, cfloatType::hasSubnormals, cfloatType::hasSupernormals, cfloatType::isSaturating> a{};
	float testValue = 14680063.0f;
	a = testValue;
	float f = float(a);
	std::cout << to_binary(a) << " : " << a << " : " << f << " : " << std::setprecision(8) << testValue << '\n';
	f = 4 * 1024.0 * 1024.0;
	for (size_t i = 0; i < 10; ++i) {
		float fulp = ulp(f);
		std::cout << to_binary(f, true) << " : " << f << '\n';
		std::cout << to_binary(fulp, true) << " : " << fulp << '\n';
		f *= 2.0f;
	}
}

template<size_t nbits, size_t es, typename bt>
void testConversion(float f) {
	sw::universal::cfloat<nbits, es, bt> a{};
	a.convert_ieee754(f);
}

template<size_t es>
void compareSmallcfloats(float f) {
	std::cout << "----------------- small cfloat comparision with es = " << es << '\n';
	testConversion<4, es, uint8_t>(f);
	testConversion<5, es, uint8_t>(f);
	testConversion<6, es, uint8_t>(f);
	testConversion<7, es, uint8_t>(f);
	testConversion<8, es, uint8_t>(f);
	std::cout << std::endl;
}

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

	constexpr bool hasSubnormals   = true;
	constexpr bool hasSupernormals = true;
	constexpr bool isSaturating    = false;

	std::string test_suite         = "ieee754 float conversion to cfloat";
	std::string test_tag           = "float conversion ";
	bool reportTestCases           = false;
	int nrOfFailedTestCases        = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// cfloat<> is a linear floating-point

	// to track conversion in more detail
	std::cout << std::setprecision(8);
	std::cerr << std::setprecision(8);

	//	ToNativeBug();
	// 
	// 
	// test case harnass
	{
		using Cfloat = cfloat<80, 11, uint16_t, hasSubnormals, hasSupernormals, isSaturating>;
		Cfloat a("0b0.01110000100.11110101000111000011111000000000000000000000000000000000000000000000");
		std::cout << to_binary(a) << " : " << a << '\n';

		cfloat<32, 8, uint32_t, hasSubnormals, hasSupernormals, isSaturating> ref{}; // this is a superset of IEEE-754 float with supernormals
		ref.setbits(0x027a8e1f);
		float refValue = float(ref);
		std::cout << to_binary(refValue) << " : " << refValue << '\n';
		{
			cfloat<80, 11, uint8_t, hasSubnormals, hasSupernormals, isSaturating> nut{};
			nut = refValue;
			float testValue = float(nut);
			std::cout << to_binary(testValue) << " : " << testValue << '\n';
			std::cout << to_binary(nut) << " : " << nut << '\n';
		}
		{
			cfloat<80, 11, uint16_t, hasSubnormals, hasSupernormals, isSaturating> nut{};
			nut = refValue;
			float testValue = float(nut);
			std::cout << to_binary(testValue) << " : " << testValue << '\n';
			std::cout << to_binary(nut) << " : " << nut << '\n';
		}
	}
	{
		float f = 0.0625f + std::pow(2.0f, -20.0f);
		f = 0.125f; // - std::pow(2.0f, -20.0f);
		std::cout << to_binary(f) << " : " << f << std::endl;

		cfloat<6, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating> a{};
		a.convert_ieee754(f);
		std::cout << to_binary(a, true) << " : " << a << '\n';
	}

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat< 4, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(true), test_tag, "cfloat<  4,  1, uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat< 6, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(true), test_tag, "cfloat<  6,  1, uint8_t>");


	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat< 4, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat<  4,  1, uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat< 6, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat<  6,  2, uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat< 8, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat<  8,  3, uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat<10, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat< 10,  4, uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat<12, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat< 12,  5, uint8_t>");

	std::cerr << "                                                     ignoring subnormals for the moment\n";
	nrOfFailedTestCases += ReportTestResult(VerifyFloat2CfloatConversionRnd< cfloat<40, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, 1000), test_tag, "cfloat< 40,  8, uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyFloat2CfloatConversionRnd< cfloat<80, 11, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, 1000), test_tag, "cfloat <80, 11, uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyFloat2CfloatConversionRnd< cfloat<96, 11, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, 1000), test_tag, "cfloat< 96, 11, uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyFloat2CfloatConversionRnd< cfloat<112, 11, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, 1000), test_tag, "cfloat<112, 11, uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyFloat2CfloatConversionRnd< cfloat<128, 11, uint8_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, 1000), test_tag, "cfloat<128, 11, uint8_t>");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures

#else  // !MANUAL_TESTING

	std::cerr << "                                                     ignoring subnormals for the moment\n";

	size_t NR_RNDS = 10000;

#if REGRESSION_LEVEL_1

	nrOfFailedTestCases += ReportTestResult(VerifyFloat2CfloatConversionRnd< cfloat<40,  8, uint8_t , hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<40,  8, uint8_t >");
	nrOfFailedTestCases += ReportTestResult(VerifyFloat2CfloatConversionRnd< cfloat<40,  8, uint16_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<40,  8, uint16_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyFloat2CfloatConversionRnd< cfloat<40,  8, uint32_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<40,  8, uint32_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyFloat2CfloatConversionRnd< cfloat<40,  8, uint64_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<40,  8, uint64_t>");

	nrOfFailedTestCases += ReportTestResult(VerifyFloat2CfloatConversionRnd< cfloat<48,  8, uint8_t , hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<48,  8, uint8_t >");
	nrOfFailedTestCases += ReportTestResult(VerifyFloat2CfloatConversionRnd< cfloat<48,  8, uint16_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<48,  8, uint16_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyFloat2CfloatConversionRnd< cfloat<48,  8, uint32_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<48,  8, uint32_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyFloat2CfloatConversionRnd< cfloat<48,  8, uint64_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<48,  8, uint64_t>");

	nrOfFailedTestCases += ReportTestResult(VerifyFloat2CfloatConversionRnd< cfloat<56,  8, uint8_t , hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<56,  8, uint8_t >");
	nrOfFailedTestCases += ReportTestResult(VerifyFloat2CfloatConversionRnd< cfloat<56,  8, uint16_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<56,  8, uint16_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyFloat2CfloatConversionRnd< cfloat<56,  8, uint32_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<56,  8, uint32_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyFloat2CfloatConversionRnd< cfloat<56,  8, uint64_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<56,  8, uint64_t>");

	nrOfFailedTestCases += ReportTestResult(VerifyFloat2CfloatConversionRnd< cfloat<64,  8, uint8_t , hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<64,  8, uint8_t >");
	nrOfFailedTestCases += ReportTestResult(VerifyFloat2CfloatConversionRnd< cfloat<64,  8, uint16_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<64,  8, uint16_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyFloat2CfloatConversionRnd< cfloat<64,  8, uint32_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<64,  8, uint32_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyFloat2CfloatConversionRnd< cfloat<64,  8, uint64_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<64,  8, uint64_t>");

	nrOfFailedTestCases += ReportTestResult(VerifyFloat2CfloatConversionRnd< cfloat<64,  9, uint8_t , hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<64,  9, uint8_t >");
	nrOfFailedTestCases += ReportTestResult(VerifyFloat2CfloatConversionRnd< cfloat<64,  9, uint16_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<64,  9, uint16_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyFloat2CfloatConversionRnd< cfloat<64,  9, uint32_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<64,  9, uint32_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyFloat2CfloatConversionRnd< cfloat<64,  9, uint64_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<64,  9, uint64_t>");

	nrOfFailedTestCases += ReportTestResult(VerifyFloat2CfloatConversionRnd< cfloat<80, 11, uint8_t , hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<80, 11, uint8_t >");
	nrOfFailedTestCases += ReportTestResult(VerifyFloat2CfloatConversionRnd< cfloat<80, 11, uint16_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<80, 11, uint16_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyFloat2CfloatConversionRnd< cfloat<80, 11, uint32_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<80, 11, uint32_t>");

	// weird case of only special cases
	//Generate table for a class sw::universal::cfloat<3, 1, unsigned char> in TXT format
	//	#   Binary    sign   scale        exponent        fraction         value      hex_format
	//	0:  b000       0       0              b0              b0             0        3.1x0x0r
	//	1 : b001       0       0              b0              b1             1        3.1x0x1r
	//	2 : b010       0       1              b1              b0           inf        3.1x0x2r
	//	3 : b011       0       1              b1              b1           nan        3.1x0x3r
	//	4 : b100       1       0              b0              b0             0        3.1x0x4r
	//	5 : b101       1       0              b0              b1            -1        3.1x0x5r
	//	6 : b110       1       1              b1              b0            -inf      3.1x0x6r
	//	7 : b111       1       1              b1              b1         nan(snan)    3.1x0x7r
	// 	   we need to special case this as the relationship between values and the special encodings
	// 	   is aliased.
	// nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat<3, 1, uint8_t>, float >(reportTestCases), test_tag, "cfloat<3,1>");

	// es = 1
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat< 4, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat< 4,1>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat< 5, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat< 5,1>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat< 6, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat< 6,1>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat< 7, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat< 7,1>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat< 8, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat< 8,1>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat< 9, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat< 9,1>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat<10, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat<10,1>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat<12, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat<12,1>");


	// es = 2
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat< 4, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat< 4,2>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat< 5, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat< 5,2>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat< 6, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat< 6,2>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat< 7, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat< 7,2>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat< 8, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat< 8,2>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat<10, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat<10,2>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat<12, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat<12,2>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat<14, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat<14,2>");


	// es = 3
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat< 5, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat< 5,3>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat< 6, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat< 6,3>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat< 7, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat< 7,3>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat< 8, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat< 8,3>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat<10, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat<10,3>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat<12, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat<12,3>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat<14, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat<14,3>");


	// es = 4
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat< 6, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat< 6,4>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat< 7, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat< 7,4>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat< 8, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat< 8,4>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat<10, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat<10,4>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat<12, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat<12,4>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat<14, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat<14,4>");


	// es = 5
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat< 7, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat< 7,5>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat< 8, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat< 8,5>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat<10, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat<10,5>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat<12, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat<12,5>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat<14, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat<14,5>");


	// es = 6
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat< 8, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat< 8,6>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat< 9, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat< 9,6>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat<10, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat<10,6>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat<12, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat<12,6>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat<14, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat<14,6>");


	// es = 7
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat< 9, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat< 9,7>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat<10, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat<10,7>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat<12, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat<12,7>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat<14, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat<14,7>");

	// es = 8
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat<11, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat<11,8>");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat<12, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat<12,8>");
//	nrOfFailedTestCases += ReportTestResult(VerifyCfloatConversion< cfloat<14, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, float >(reportTestCases), test_tag, "cfloat<14,8>");

#endif

#if REGRESSION_LEVEL_2


#endif

#if REGRESSION_LEVEL_3


#endif

#if REGRESSION_LEVEL_4


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
