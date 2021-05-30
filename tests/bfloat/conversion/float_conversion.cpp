// float_conversion.cpp: test suite runner for IEEE float conversions to bfloats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>
// Configure the bfloat template environment
// first: enable general or specialized configurations
#define BFLOAT_FAST_SPECIALIZATION
// second: enable/disable arithmetic exceptions
#define BFLOAT_THROW_ARITHMETIC_EXCEPTION 0
// third: enable trace conversion
#define TRACE_CONVERSION 0

// minimum set of include files to reflect source code dependencies
#include <universal/number/bfloat/bfloat.hpp>
#include <universal/number/bfloat/manipulators.hpp>
#include <universal/number/bfloat/math_functions.hpp>
#include <universal/verification/test_suite_conversion.hpp>
#include <universal/verification/bfloat_test_suite.hpp>
#include <universal/number/bfloat/table.hpp> // only used for value table generation

#if BIT_CAST_SUPPORT
void ToNativeBug() {  // now resolved... exponentiation was incorrect
	using namespace sw::universal;
	bfloat<32, 8, uint32_t> a, b;
	// b1.00111111.00011001011010001001001 != b1.01111111.00011001011010001001001
	a = parse<32, 8, uint32_t>("b1.00111111.00011001011010001001001");
	std::cout << "bfloat   : " << to_binary(a) << '\n';
	float f = float(a);
	std::cout << "float    : " << to_binary(f) << '\n';
	b = f;
	std::cout << "bfloat b : " << to_binary(b) << '\n';

	blockbinary<32, uint32_t> bits;
	a.getbits(bits);
	std::cout << "bits     : " << to_binary(bits, false) << '\n';
	// bit cast
	uint64_t bc = std::bit_cast<uint32_t, float>(f);
	std::cout << "float    : " << to_binary(f) << '\n';
	std::cout << "emask    : " << to_binary(ieee754_parameter<float>::emask, 32, false) << std::endl;
	std::cout << "raw bits : " << to_binary(bc, 32, false) << std::endl;
	std::cout << "fmask    : " << to_binary(ieee754_parameter<float>::fmask, 32, false) << std::endl;
	std::cout << "emask+bc : " << to_binary((ieee754_parameter<float>::emask & bc), 32, false) << std::endl;
	bool s = (ieee754_parameter<float>::smask & bc);
	uint64_t rawExponentBits = (ieee754_parameter<float>::emask & bc) >> ieee754_parameter<float>::fbits;
	uint64_t rawFractionBits = (ieee754_parameter<float>::fmask & bc);
}
#endif // BIT_CAST_SUPPORT

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
b0.00000000.00000000010000000000000 : 1.14794370197489e-41
b0.00000000.00000000100000000000000 : 2.29588740394978e-41
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
// float subnormals with the last entry being the smallest normal value
float ieee754_float_subnormals[24] = {
 1.401298464324817e-45,
 2.802596928649634e-45,
 5.605193857299268e-45,
 1.121038771459854e-44,
 2.242077542919707e-44,
 4.484155085839415e-44,
 8.968310171678829e-44,
 1.793662034335766e-43,
 3.587324068671532e-43,
 7.174648137343063e-43,
 1.434929627468613e-42,
 2.869859254937225e-42,
 5.739718509874451e-42,
 1.14794370197489e-41,
 2.29588740394978e-41,
 4.591774807899561e-41,
 9.183549615799121e-41,
 1.836709923159824e-40,
 3.673419846319648e-40,
 7.346839692639297e-40,
 1.469367938527859e-39,
 2.938735877055719e-39,
 5.877471754111438e-39,
 1.175494350822288e-38     // smallest normal value
};

void GenerateSinglePrecisionSubnormals() 
{
	using namespace sw::universal;
	constexpr size_t nbits = 32;
	constexpr size_t es = 8;
	using bt = uint32_t;
	bfloat<nbits, es, bt> a, b;
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

template<typename BfloatType>
void Test1() 
{
	BfloatType a;
	a.constexprClassParameters();

	float testValue = 8.0f;
	a = testValue;
	float f = float(a);
	std::cout << to_binary(a) << " : " << a << " : " << f << " : " << std::setprecision(8) << testValue << '\n';
}

template<typename BfloatType>
void Test2()
{
	using namespace sw::universal;
	
	bfloat<8, 6, uint8_t> a;
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
	sw::universal::bfloat<nbits, es, bt> a;
	a.convert_ieee754(f);
}

template<size_t es>
void compareSmallBfloats(float f) {
	std::cout << "----------------- small bfloat comparision with es = " << es << '\n';
	testConversion<4, es, uint8_t>(f);
	testConversion<5, es, uint8_t>(f);
	testConversion<6, es, uint8_t>(f);
	testConversion<7, es, uint8_t>(f);
	testConversion<8, es, uint8_t>(f);
	std::cout << std::endl;
}


// conditional compile flags
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	print_cmd_line(argc, argv);

	int nrOfFailedTestCases = 0;
	std::string tag = "float conversion: ";

#if MANUAL_TESTING

	// bfloat<> is a linear floating-point

	// to track conversion in more detail
	std::cout << std::setprecision(8);
	std::cerr << std::setprecision(8);

	float f = ieee754_float_subnormals[1];
	std::cout << to_binary(0.5f*f) << '\n' 
		<< to_binary(f) << '\n'
		<< to_binary(2*f) << std::endl;

//	GenerateTable< bfloat<4, 1, uint8_t> >(std::cout);
	f = 1.875f + 0.0625f;
	compareSmallBfloats<1>(f);
	compareSmallBfloats<2>(f);

	bool bReportIndividualTestCases = true;
	nrOfFailedTestCases += ReportTestResult(VerifySinglePrecision<uint8_t>(bReportIndividualTestCases), tag, "bfloat<32, 8, uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifySinglePrecision<uint16_t>(bReportIndividualTestCases), tag, "bfloat<32, 8, uint16_t>");
	nrOfFailedTestCases += ReportTestResult(VerifySinglePrecision<uint32_t>(bReportIndividualTestCases), tag, "bfloat<32, 8, uint32_t>");
	nrOfFailedTestCases += ReportTestResult(VerifySinglePrecision<uint64_t>(bReportIndividualTestCases), tag, "bfloat<32, 8, uint64_t>");
	return 0;

	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat< 4, 1, uint8_t>, float >(true), tag, "bfloat<4,1,uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat< 6, 2, uint8_t>, float >(false), tag, "bfloat<6,2,uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat< 8, 3, uint8_t>, float >(false), tag, "bfloat<8,3,uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<10, 4, uint8_t>, float >(false), tag, "bfloat<10,4,uint8_t>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<12, 5, uint8_t>, float >(false), tag, "bfloat<12,5,uint8_t>");

	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat< 8, 6, uint8_t>, float >(false), tag, "bfloat<8,6,uint8_t>");

	std::cout << "failed tests: " << nrOfFailedTestCases << endl;
	nrOfFailedTestCases = 0; // in manual testing we ignore failures for the regression system

#if STRESS_TESTING

	// manual exhaustive test

#endif

#else  // !MANUAL_TESTING
	bool bReportIndividualTestCases = false;
	cout << "BFLOAT conversion from float validation" << endl;

	// es = 1
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<3, 1>, float >(bReportIndividualTestCases), tag, "bfloat<3,1>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<4, 1>, float >(bReportIndividualTestCases), tag, "bfloat<4,1>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<5, 1>, float >(bReportIndividualTestCases), tag, "bfloat<5,1>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<6, 1>, float >(bReportIndividualTestCases), tag, "bfloat<6,1>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<7, 1>, float >(bReportIndividualTestCases), tag, "bfloat<7,1>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<8, 1>, float >(bReportIndividualTestCases), tag, "bfloat<8,1>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<9, 1>, float >(bReportIndividualTestCases), tag, "bfloat<9,1>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<10, 1>, float >(bReportIndividualTestCases), tag, "bfloat<10,1>");
	nrOfFailedTestCases+ = ReportTestResult(VerifyBfloatConversion< bfloat<12, 1>, float >(bReportIndividualTestCases), tag, "bfloat<12,1>");


	// es = 2
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<4, 2>, float >(bReportIndividualTestCases), tag, "bfloat<4,2>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<5, 2>, float >(bReportIndividualTestCases), tag, "bfloat<5,2>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<6, 2>, float >(bReportIndividualTestCases), tag, "bfloat<6,2>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<7, 2>, float >(bReportIndividualTestCases), tag, "bfloat<7,2>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<8, 2>, float >(bReportIndividualTestCases), tag, "bfloat<8,2>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<10, 2>, float >(bReportIndividualTestCases), tag, "bfloat<10,2>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<12, 2>, float >(bReportIndividualTestCases), tag, "bfloat<12,2>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<14, 2>, float >(bReportIndividualTestCases), tag, "bfloat<14,2>");


	// es = 3
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<5, 3>, float >(bReportIndividualTestCases), tag, "bfloat<5,3>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<6, 3>, float >(bReportIndividualTestCases), tag, "bfloat<6,3>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<7, 3>, float >(bReportIndividualTestCases), tag, "bfloat<7,3>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<8, 3>, float >(bReportIndividualTestCases), tag, "bfloat<8,3>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<10, 3>, float >(bReportIndividualTestCases), tag, "bfloat<10,3>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<12, 3>, float >(bReportIndividualTestCases), tag, "bfloat<12,3>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<14, 3>, float >(bReportIndividualTestCases), tag, "bfloat<14,3>");


	// es = 4
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<6, 4>, float >(bReportIndividualTestCases), tag, "bfloat<6,4>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<7, 4>, float >(bReportIndividualTestCases), tag, "bfloat<7,4>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<8, 4>, float >(bReportIndividualTestCases), tag, "bfloat<8,4>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<10, 4>, float >(bReportIndividualTestCases), tag, "bfloat<10,4>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<12, 4>, float >(bReportIndividualTestCases), tag, "bfloat<12,4>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<14, 4>, float >(bReportIndividualTestCases), tag, "bfloat<14,4>");


	// es = 5
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<7, 5>, float >(bReportIndividualTestCases), tag, "bfloat<7,5>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<8, 5>, float >(bReportIndividualTestCases), tag, "bfloat<8,5>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<10, 5>, float >(bReportIndividualTestCases), tag, "bfloat<10,5>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<12, 5>, float >(bReportIndividualTestCases), tag, "bfloat<12,5>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<14, 5>, float >(bReportIndividualTestCases), tag, "bfloat<14,5>");

#ifdef LATER
	// es = 6
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<8, 6>, float >(bReportIndividualTestCases), tag, "bfloat<8,6>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<9, 6>, float >(bReportIndividualTestCases), tag, "bfloat<9,6>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<10, 6>, float >(bReportIndividualTestCases), tag, "bfloat<10,6>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<12, 6>, float >(bReportIndividualTestCases), tag, "bfloat<12,6>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<14, 6>, float >(bReportIndividualTestCases), tag, "bfloat<14,6>");


	// es = 7
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat< 9, 7>, float >(bReportIndividualTestCases), tag, "bfloat<9,7>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<10, 7>, float >(bReportIndividualTestCases), tag, "bfloat<10,7>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<12, 7>, float >(bReportIndividualTestCases), tag, "bfloat<12,7>");
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<14, 7>, float >(bReportIndividualTestCases), tag, "bfloat<14,7>");

	// es = 8
//	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<11, 8>, float >(bReportIndividualTestCases), tag, "bfloat<11,8>");
//	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<12, 8>, float >(bReportIndividualTestCases), tag, "bfloat<12,8>");
//	nrOfFailedTestCases += ReportTestResult(VerifyBfloatConversion< bfloat<14, 8>, float >(bReportIndividualTestCases), tag, "bfloat<14,8>");
#endif

#if STRESS_TESTING

#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::bfloat_arithmetic_exception& err) {
	std::cerr << "Uncaught bfloat arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::bfloat_internal_exception& err) {
	std::cerr << "Uncaught bfloat internal exception: " << err.what() << std::endl;
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


/*

  To generate:
  	GenerateFixedPointComparisonTable<4, 0>(std::string("-"));
	GenerateFixedPointComparisonTable<4, 1>(std::string("-"));
	GenerateFixedPointComparisonTable<4, 2>(std::string("-"));
	

 */
