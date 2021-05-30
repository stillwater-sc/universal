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



	float f = 2.2420775e-44f;
	std::cout << to_binary(0.5f*f) << '\n' 
		<< to_binary(f) << '\n'
		<< to_binary(2*f) << std::endl;


	GenerateTable< bfloat<4, 1, uint8_t> >(cout);
	f = 1.875f + 0.0625f;
	compareSmallBfloats<1>(f);
	compareSmallBfloats<2>(f);
	return 0;

	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat< 4, 1, uint8_t>, float >(true), tag, "bfloat<4,1,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat< 6, 2, uint8_t>, float >(false), tag, "bfloat<6,2,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat< 8, 3, uint8_t>, float >(false), tag, "bfloat<8,3,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<10, 4, uint8_t>, float >(false), tag, "bfloat<10,4,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<12, 5, uint8_t>, float >(false), tag, "bfloat<12,5,uint8_t>");

	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat< 8, 6, uint8_t>, float >(false), tag, "bfloat<8,6,uint8_t>");

	std::cout << "failed tests: " << nrOfFailedTestCases << endl;
	nrOfFailedTestCases = 0; // in manual testing we ignore failures for the regression system

#if STRESS_TESTING

	// manual exhaustive test

#endif

#else  // !MANUAL_TESTING
	bool bReportIndividualTestCases = false;
	cout << "BFLOAT conversion from float validation" << endl;

	// es = 1
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<3, 1>, float >(bReportIndividualTestCases), tag, "bfloat<3,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<4, 1>, float >(bReportIndividualTestCases), tag, "bfloat<4,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<5, 1>, float >(bReportIndividualTestCases), tag, "bfloat<5,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<6, 1>, float >(bReportIndividualTestCases), tag, "bfloat<6,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<7, 1>, float >(bReportIndividualTestCases), tag, "bfloat<7,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<8, 1>, float >(bReportIndividualTestCases), tag, "bfloat<8,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<9, 1>, float >(bReportIndividualTestCases), tag, "bfloat<9,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<10, 1>, float >(bReportIndividualTestCases), tag, "bfloat<10,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<12, 1>, float >(bReportIndividualTestCases), tag, "bfloat<12,1>");


	// es = 2
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<4, 2>, float >(bReportIndividualTestCases), tag, "bfloat<4,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<5, 2>, float >(bReportIndividualTestCases), tag, "bfloat<5,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<6, 2>, float >(bReportIndividualTestCases), tag, "bfloat<6,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<7, 2>, float >(bReportIndividualTestCases), tag, "bfloat<7,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<8, 2>, float >(bReportIndividualTestCases), tag, "bfloat<8,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<10, 2>, float >(bReportIndividualTestCases), tag, "bfloat<10,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<12, 2>, float >(bReportIndividualTestCases), tag, "bfloat<12,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<14, 2>, float >(bReportIndividualTestCases), tag, "bfloat<14,2>");


	// es = 3
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<5, 3>, float >(bReportIndividualTestCases), tag, "bfloat<5,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<6, 3>, float >(bReportIndividualTestCases), tag, "bfloat<6,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<7, 3>, float >(bReportIndividualTestCases), tag, "bfloat<7,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<8, 3>, float >(bReportIndividualTestCases), tag, "bfloat<8,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<10, 3>, float >(bReportIndividualTestCases), tag, "bfloat<10,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<12, 3>, float >(bReportIndividualTestCases), tag, "bfloat<12,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<14, 3>, float >(bReportIndividualTestCases), tag, "bfloat<14,3>");


	// es = 4
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<6, 4>, float >(bReportIndividualTestCases), tag, "bfloat<6,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<7, 4>, float >(bReportIndividualTestCases), tag, "bfloat<7,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<8, 4>, float >(bReportIndividualTestCases), tag, "bfloat<8,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<10, 4>, float >(bReportIndividualTestCases), tag, "bfloat<10,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<12, 4>, float >(bReportIndividualTestCases), tag, "bfloat<12,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<14, 4>, float >(bReportIndividualTestCases), tag, "bfloat<14,4>");


	// es = 5
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<7, 5>, float >(bReportIndividualTestCases), tag, "bfloat<7,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<8, 5>, float >(bReportIndividualTestCases), tag, "bfloat<8,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<10, 5>, float >(bReportIndividualTestCases), tag, "bfloat<10,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<12, 5>, float >(bReportIndividualTestCases), tag, "bfloat<12,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<14, 5>, float >(bReportIndividualTestCases), tag, "bfloat<14,5>");

#ifdef LATER
	// es = 6
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<8, 6>, float >(bReportIndividualTestCases), tag, "bfloat<8,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<9, 6>, float >(bReportIndividualTestCases), tag, "bfloat<9,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<10, 6>, float >(bReportIndividualTestCases), tag, "bfloat<10,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<12, 6>, float >(bReportIndividualTestCases), tag, "bfloat<12,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<14, 6>, float >(bReportIndividualTestCases), tag, "bfloat<14,6>");


	// es = 7
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat< 9, 7>, float >(bReportIndividualTestCases), tag, "bfloat<9,7>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<10, 7>, float >(bReportIndividualTestCases), tag, "bfloat<10,7>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<12, 7>, float >(bReportIndividualTestCases), tag, "bfloat<12,7>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<14, 7>, float >(bReportIndividualTestCases), tag, "bfloat<14,7>");

	// es = 8
//	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<11, 8>, float >(bReportIndividualTestCases), tag, "bfloat<11,8>");
//	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<12, 8>, float >(bReportIndividualTestCases), tag, "bfloat<12,8>");
//	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<14, 8>, float >(bReportIndividualTestCases), tag, "bfloat<14,8>");
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
