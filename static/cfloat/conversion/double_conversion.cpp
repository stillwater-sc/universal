// double_conversion.cpp: test suite runner for double conversions to classic cfloats
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
#define CFLOAT_FAST_SPECIALIZATION
// second: enable/disable arithmetic exceptions
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 0
// third: enable trace conversion
#define TRACE_CONVERSION 0
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/cfloat_test_suite.hpp>
#include <universal/number/cfloat/table.hpp> // only used for value table generation

// sign of 0 is flipped on MSVC Release builds
void CompilerBug() {
	using namespace sw::universal;
	{
		cfloat<5, 1, uint8_t, true, true, false> a; // uninitialized
		a.setbits(0x0);
		std::cout << "cfloat<5,1> : " << to_binary(a) << " : " << a << '\n';
		float f = float(a);
		std::cout << "float      : " << f << '\n';
		double d = double(a);
		std::cout << "double     : " << d << '\n';
	}
	{
		cfloat<5, 1, uint8_t, true, true, false> a; // uninitialized
		a.setbits(0x10);
		std::cout << "cfloat<5,1> : " << to_binary(a) << " : " << a << '\n';
		float f = float(a);
		std::cout << "float      : " << f << '\n';
		double d = double(a);
		std::cout << "double     : " << d << '\n';
	}

	{
		cfloat<6, 1, uint8_t, true, true, false> a; // uninitialized
		a.setbits(0x0);
		std::cout << "cfloat<6,1> : " << to_binary(a) << " : " << a << '\n';
		float f = float(a);
		std::cout << "float      : " << f << '\n';
		double d = double(a);
		std::cout << "double     : " << d << '\n';
	}
	{
		cfloat<6, 1, uint8_t, true, true, false> a; // uninitialized
		a.setbits(0x20);
		std::cout << "cfloat<6,1> : " << to_binary(a) << " : " << a << '\n';
		float f = float(a);
		std::cout << "float      : " << f << '\n';
		double d = double(a);
		std::cout << "double     : " << d << '\n';
	}
}

/* IEEE-754 double precision subnormals
b0.00000000000.0000000000000000000000000000000000000000000000000001 : 4.940656458412465e-324
b0.00000000000.0000000000000000000000000000000000000000000000000001 : 4.940656458412465e-324
b0.00000000000.0000000000000000000000000000000000000000000000000010 : 9.881312916824931e-324
b0.00000000000.0000000000000000000000000000000000000000000000000100 : 1.976262583364986e-323
b0.00000000000.0000000000000000000000000000000000000000000000001000 : 3.952525166729972e-323
b0.00000000000.0000000000000000000000000000000000000000000000010000 : 7.905050333459945e-323
b0.00000000000.0000000000000000000000000000000000000000000000100000 : 1.581010066691989e-322
b0.00000000000.0000000000000000000000000000000000000000000001000000 : 3.162020133383978e-322
b0.00000000000.0000000000000000000000000000000000000000000010000000 : 6.324040266767956e-322
b0.00000000000.0000000000000000000000000000000000000000000100000000 : 1.264808053353591e-321
b0.00000000000.0000000000000000000000000000000000000000001000000000 : 2.529616106707182e-321
b0.00000000000.0000000000000000000000000000000000000000010000000000 : 5.059232213414365e-321
b0.00000000000.0000000000000000000000000000000000000000100000000000 : 1.011846442682873e-320
b0.00000000000.0000000000000000000000000000000000000001000000000000 : 2.023692885365746e-320
b0.00000000000.0000000000000000000000000000000000000010000000000000 : 4.047385770731492e-320
b0.00000000000.0000000000000000000000000000000000000100000000000000 : 8.094771541462983e-320
b0.00000000000.0000000000000000000000000000000000001000000000000000 : 1.618954308292597e-319
b0.00000000000.0000000000000000000000000000000000010000000000000000 : 3.237908616585193e-319
b0.00000000000.0000000000000000000000000000000000100000000000000000 : 6.475817233170387e-319
b0.00000000000.0000000000000000000000000000000001000000000000000000 : 1.295163446634077e-318
b0.00000000000.0000000000000000000000000000000010000000000000000000 : 2.590326893268155e-318
b0.00000000000.0000000000000000000000000000000100000000000000000000 : 5.180653786536309e-318
b0.00000000000.0000000000000000000000000000001000000000000000000000 : 1.036130757307262e-317
b0.00000000000.0000000000000000000000000000010000000000000000000000 : 2.072261514614524e-317
b0.00000000000.0000000000000000000000000000100000000000000000000000 : 4.144523029229047e-317
b0.00000000000.0000000000000000000000000001000000000000000000000000 : 8.289046058458095e-317
b0.00000000000.0000000000000000000000000010000000000000000000000000 : 1.657809211691619e-316
b0.00000000000.0000000000000000000000000100000000000000000000000000 : 3.315618423383238e-316
b0.00000000000.0000000000000000000000001000000000000000000000000000 : 6.631236846766476e-316
b0.00000000000.0000000000000000000000010000000000000000000000000000 : 1.326247369353295e-315
b0.00000000000.0000000000000000000000100000000000000000000000000000 : 2.65249473870659e-315
b0.00000000000.0000000000000000000001000000000000000000000000000000 : 5.304989477413181e-315
b0.00000000000.0000000000000000000010000000000000000000000000000000 : 1.060997895482636e-314
b0.00000000000.0000000000000000000100000000000000000000000000000000 : 2.121995790965272e-314
b0.00000000000.0000000000000000001000000000000000000000000000000000 : 4.243991581930545e-314
b0.00000000000.0000000000000000010000000000000000000000000000000000 : 8.487983163861089e-314
b0.00000000000.0000000000000000100000000000000000000000000000000000 : 1.697596632772218e-313
b0.00000000000.0000000000000001000000000000000000000000000000000000 : 3.395193265544436e-313
b0.00000000000.0000000000000010000000000000000000000000000000000000 : 6.790386531088871e-313
b0.00000000000.0000000000000100000000000000000000000000000000000000 : 1.358077306217774e-312
b0.00000000000.0000000000001000000000000000000000000000000000000000 : 2.716154612435549e-312
b0.00000000000.0000000000010000000000000000000000000000000000000000 : 5.432309224871097e-312
b0.00000000000.0000000000100000000000000000000000000000000000000000 : 1.086461844974219e-311
b0.00000000000.0000000001000000000000000000000000000000000000000000 : 2.172923689948439e-311
b0.00000000000.0000000010000000000000000000000000000000000000000000 : 4.345847379896878e-311
b0.00000000000.0000000100000000000000000000000000000000000000000000 : 8.691694759793755e-311
b0.00000000000.0000001000000000000000000000000000000000000000000000 : 1.738338951958751e-310
b0.00000000000.0000010000000000000000000000000000000000000000000000 : 3.476677903917502e-310
b0.00000000000.0000100000000000000000000000000000000000000000000000 : 6.953355807835004e-310
b0.00000000000.0001000000000000000000000000000000000000000000000000 : 1.390671161567001e-309
b0.00000000000.0010000000000000000000000000000000000000000000000000 : 2.781342323134002e-309
b0.00000000000.0100000000000000000000000000000000000000000000000000 : 5.562684646268003e-309
b0.00000000000.1000000000000000000000000000000000000000000000000000 : 1.112536929253601e-308
b0.00000000001.0000000000000000000000000000000000000000000000000000 : 2.225073858507201e-308
b0.00000000010.0000000000000000000000000000000000000000000000000000 : 4.450147717014403e-308
*/

double smallest_normal = 2.225073858507201e-308;

void GenerateDoublePrecisionSubnormals()
{
	constexpr size_t nbits = 64;
	constexpr size_t es = 11;
	using bt = uint64_t;
	using namespace sw::universal;
	cfloat<nbits, es, bt> a{};
	++a;
	double d = double(a);
	std::cout << std::setprecision(20);
	std::cout << to_binary(a) << " : " << a << '\n';
	std::cout << to_binary(d) << " : " << d << '\n';
	for (int i = 0; i < 53; ++i) {
		d *= 2;
		std::cout << to_binary(d) << " : " << d << '\n';
	}
	for (int i = 0; i < 53; ++i) {
		d = ieee754_double_subnormals[i];
		std::cout << to_binary(d) << " : " << d << '\n';
	}
	std::cout << std::setprecision(5);
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

	std::string test_suite         = "ieee754 double conversion to cfloat";
	std::string test_tag           = "conversion";
	bool reportTestCases           = false;
	int nrOfFailedTestCases        = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// to track conversion in more detail
	std::cout << std::setprecision(15);
	std::cerr << std::setprecision(15);


	{
		constexpr size_t nbits = 64;
		constexpr size_t es = 11;
		using bt = uint8_t;  // exercise the block algorithms
		constexpr bool hasSubnormals = true;
		constexpr bool hasSupernormals = true;
		constexpr bool isSaturating = false;

		cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> ref = parse<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>("b0.11010111001.0111010010011101001011001010000101110001000011010111");
		double testValue = double(ref);
		std::cout << "ref : " << to_binary(ref) << " : " << ref << '\n';
		std::cout << "test: " << to_binary(testValue) << " : " << testValue << '\n';
		cfloat<64, 8, uint8_t> nut;  // uninitialized
		//		a.constexprClassParameters();
		nut = testValue;
		double da = double(nut);
		std::cout << "nut : " << to_binary(nut) << " : " << nut << '\n';
		std::cout << "da  : " << to_binary(da) << " : " << da << '\n';
	}

#ifdef FLOATING_POINT_CONTRACTION_TESTS
	// double 2 cfloat conversion uses an ieee-754 double value to assign.
	// When converting to a 'smaller' cfloat, there is a high probability of underflow and overflow
	// 
	// a cfloat<64, 8> will have a 8x smaller dynamic range and thus we will have a 7 in 8 change to saturate
	// as we saturate to maxpos, which is a regularly looking value, it is difficult to recognize this failure mode
	nrOfFailedTestCases += ReportTestResult(VerifyDouble2CfloatConversionRnd< cfloat< 64, 8, uint8_t > >(reportTestCases, 10), test_tag, "cfloat< 64, 8, uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyDouble2CfloatConversionRnd< cfloat< 64, 8, uint16_t> >(reportTestCases, 10), test_tag, "cfloat< 64, 8, uint16_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyDouble2CfloatConversionRnd< cfloat< 64, 8, uint32_t> >(reportTestCases, 10), test_tag, "cfloat< 64, 8, uint32_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyDouble2CfloatConversionRnd< cfloat< 64, 8, uint64_t> >(reportTestCases, 10), test_tag, "cfloat< 64, 8, uint64_t>");
#endif

	nrOfFailedTestCases += ReportTestResult(VerifyDouble2CfloatConversionRnd< cfloat< 64, 11, uint8_t > >(reportTestCases, 1000), test_tag, "cfloat< 64, 11, uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyDouble2CfloatConversionRnd< cfloat< 64, 11, uint16_t> >(reportTestCases, 1000), test_tag, "cfloat< 64, 11, uint16_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyDouble2CfloatConversionRnd< cfloat< 64, 11, uint32_t> >(reportTestCases, 1000), test_tag, "cfloat< 64, 11, uint32_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyDouble2CfloatConversionRnd< cfloat< 64, 11, uint64_t> >(reportTestCases, 1000), test_tag, "cfloat< 64, 11, uint64_t>");

	nrOfFailedTestCases += ReportTestResult(VerifyDouble2CfloatConversionRnd< cfloat< 80, 11, uint8_t > >(reportTestCases, 1000), test_tag, "cfloat< 80, 11, uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyDouble2CfloatConversionRnd< cfloat< 96, 11, uint8_t > >(reportTestCases, 1000), test_tag, "cfloat< 96, 11, uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyDouble2CfloatConversionRnd< cfloat<112, 11, uint8_t > >(reportTestCases, 1000), test_tag, "cfloat<112, 11, uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyDouble2CfloatConversionRnd< cfloat<128, 11, uint8_t > >(reportTestCases, 1000), test_tag, "cfloat<128, 11, uint8_t>");

	// es = 1
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat< 4, 1, uint8_t, true, true, false>, double >(reportTestCases), test_tag, "cfloat< 4,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat< 5, 1, uint8_t, true, true, false>, double >(reportTestCases), test_tag, "cfloat< 5,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat< 6, 1, uint8_t, true, true, false>, double >(reportTestCases), test_tag, "cfloat< 6,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat< 7, 1, uint8_t, true, true, false>, double >(reportTestCases), test_tag, "cfloat< 7,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat< 8, 1, uint8_t, true, true, false>, double >(reportTestCases), test_tag, "cfloat< 8,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat< 9, 1, uint8_t, true, true, false>, double >(reportTestCases), test_tag, "cfloat< 9,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat<10, 1, uint8_t, true, true, false>, double >(reportTestCases), test_tag, "cfloat<10,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat<12, 1, uint8_t, true, true, false>, double >(reportTestCases), test_tag, "cfloat<12,1>");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else  // !MANUAL_TESTING

	std::cerr << "                                                     ignoring subnormals for the moment\n";

	size_t NR_RNDS = 10000;

	// conversion of an IEEE-754 double to a smaller cfloat exhibits many overflow and underflow situations that destroy information
#ifdef FLOATING_POINT_CONTRACTION_TESTS
	nrOfFailedTestCases += ReportTestResult(VerifyDouble2CfloatConversionRnd< cfloat<40,  8, uint8_t , hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<40,  8, uint8_t >");
	nrOfFailedTestCases += ReportTestResult(VerifyDouble2CfloatConversionRnd< cfloat<40,  8, uint16_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<40,  8, uint16_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyDouble2CfloatConversionRnd< cfloat<40,  8, uint32_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<40,  8, uint32_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyDouble2CfloatConversionRnd< cfloat<40,  8, uint64_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<40,  8, uint64_t>");

	nrOfFailedTestCases += ReportTestResult(VerifyDouble2CfloatConversionRnd< cfloat<48,  8, uint8_t , hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<48,  8, uint8_t >");
	nrOfFailedTestCases += ReportTestResult(VerifyDouble2CfloatConversionRnd< cfloat<48,  8, uint16_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<48,  8, uint16_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyDouble2CfloatConversionRnd< cfloat<48,  8, uint32_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<48,  8, uint32_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyDouble2CfloatConversionRnd< cfloat<48,  8, uint64_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<48,  8, uint64_t>");

	nrOfFailedTestCases += ReportTestResult(VerifyDouble2CfloatConversionRnd< cfloat<56,  8, uint8_t , hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<56,  8, uint8_t >");
	nrOfFailedTestCases += ReportTestResult(VerifyDouble2CfloatConversionRnd< cfloat<56,  8, uint16_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<56,  8, uint16_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyDouble2CfloatConversionRnd< cfloat<56,  8, uint32_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<56,  8, uint32_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyDouble2CfloatConversionRnd< cfloat<56,  8, uint64_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<56,  8, uint64_t>");

	nrOfFailedTestCases += ReportTestResult(VerifyDouble2CfloatConversionRnd< cfloat<64,  8, uint8_t , hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<64,  8, uint8_t >");
	nrOfFailedTestCases += ReportTestResult(VerifyDouble2CfloatConversionRnd< cfloat<64,  8, uint16_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<64,  8, uint16_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyDouble2CfloatConversionRnd< cfloat<64,  8, uint32_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<64,  8, uint32_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyDouble2CfloatConversionRnd< cfloat<64,  8, uint64_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<64,  8, uint64_t>");

	nrOfFailedTestCases += ReportTestResult(VerifyDouble2CfloatConversionRnd< cfloat<64,  9, uint8_t , hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<64,  9, uint8_t >");
	nrOfFailedTestCases += ReportTestResult(VerifyDouble2CfloatConversionRnd< cfloat<64,  9, uint16_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<64,  9, uint16_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyDouble2CfloatConversionRnd< cfloat<64,  9, uint32_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<64,  9, uint32_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyDouble2CfloatConversionRnd< cfloat<64,  9, uint64_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<64,  9, uint64_t>");
#endif
	nrOfFailedTestCases += ReportTestResult(VerifyDouble2CfloatConversionRnd< cfloat<64, 11, uint64_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<64, 11, uint64_t>");

	nrOfFailedTestCases += ReportTestResult(VerifyDouble2CfloatConversionRnd< cfloat<80, 11, uint8_t , hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<80, 11, uint8_t >");
	nrOfFailedTestCases += ReportTestResult(VerifyDouble2CfloatConversionRnd< cfloat<80, 11, uint16_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<80, 11, uint16_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyDouble2CfloatConversionRnd< cfloat<80, 11, uint32_t, hasSubnormals, hasSupernormals, isSaturating> >(reportTestCases, NR_RNDS), test_tag, "cfloat<80, 11, uint32_t>");


	// es = 1
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat< 4, 1, uint8_t, true, true, isSaturating>, double >(reportTestCases), test_tag, "cfloat< 4,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat< 5, 1, uint8_t, true, true, isSaturating>, double >(reportTestCases), test_tag, "cfloat< 5,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat< 6, 1, uint8_t, true, true, isSaturating>, double >(reportTestCases), test_tag, "cfloat< 6,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat< 7, 1, uint8_t, true, true, isSaturating>, double >(reportTestCases), test_tag, "cfloat< 7,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat< 8, 1, uint8_t, true, true, isSaturating>, double >(reportTestCases), test_tag, "cfloat< 8,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat< 9, 1, uint8_t, true, true, isSaturating>, double >(reportTestCases), test_tag, "cfloat< 9,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat<10, 1, uint8_t, true, true, isSaturating>, double >(reportTestCases), test_tag, "cfloat<10,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat<12, 1, uint8_t, true, true, isSaturating>, double >(reportTestCases), test_tag, "cfloat<12,1>");


	// es = 2
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat< 5, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double >(reportTestCases), test_tag, "cfloat< 5,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat< 6, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double >(reportTestCases), test_tag, "cfloat< 6,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat< 7, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double >(reportTestCases), test_tag, "cfloat< 7,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat< 8, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double >(reportTestCases), test_tag, "cfloat< 8,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat<10, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double >(reportTestCases), test_tag, "cfloat<10,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat<12, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double >(reportTestCases), test_tag, "cfloat<12,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat<14, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double >(reportTestCases), test_tag, "cfloat<14,2>");


	// es = 3
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat< 6, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double >(reportTestCases), test_tag, "cfloat< 6,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat< 7, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double >(reportTestCases), test_tag, "cfloat< 7,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat< 8, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double >(reportTestCases), test_tag, "cfloat< 8,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat<10, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double >(reportTestCases), test_tag, "cfloat<10,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat<12, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double >(reportTestCases), test_tag, "cfloat<12,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat<14, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double >(reportTestCases), test_tag, "cfloat<14,3>");


	// es = 4
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat< 7, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double >(reportTestCases), test_tag, "cfloat< 7,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat< 8, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double >(reportTestCases), test_tag, "cfloat< 8,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat<10, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double >(reportTestCases), test_tag, "cfloat<10,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat<12, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double >(reportTestCases), test_tag, "cfloat<12,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat<14, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double >(reportTestCases), test_tag, "cfloat<14,4>");


	// es = 5
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat< 8, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double >(reportTestCases), test_tag, "cfloat< 8,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat<10, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double >(reportTestCases), test_tag, "cfloat<10,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat<12, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double >(reportTestCases), test_tag, "cfloat<12,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat<14, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double >(reportTestCases), test_tag, "cfloat<14,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat<16, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double >(reportTestCases), test_tag, "cfloat<16,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat<16, 5, uint16_t, hasSubnormals, hasSupernormals, isSaturating>, double >(reportTestCases), test_tag, "cfloat<16,5, uint16_t>");

#ifdef LATER
	// es = 6
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat< 9, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double >(reportTestCases), test_tag, "cfloat< 9,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat<10, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double >(reportTestCases), test_tag, "cfloat<10,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat<12, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double >(reportTestCases), test_tag, "cfloat<12,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat<14, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double >(reportTestCases), test_tag, "cfloat<14,6>");


	// es = 7
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat<10, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double >(reportTestCases), test_tag, "cfloat<10,7>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat<12, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double >(reportTestCases), test_tag, "cfloat<12,7>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat<14, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double >(reportTestCases), test_tag, "cfloat<14,7>");


	// es = 8
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat<11, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double >(reportTestCases), test_tag, "cfloat<11,8>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat<12, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double >(reportTestCases), test_tag, "cfloat<12,8>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatConversion< cfloat<14, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, double >(reportTestCases), test_tag, "cfloat<14,8>");

#endif // LATER

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
