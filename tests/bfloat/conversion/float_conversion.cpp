// float_conversion.cpp: test suite runner for IEEE float conversions to bfloats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#if defined(_MSC_VER)
#pragma warning(disable : 4514)  // unreferenced function is removed
#pragma warning(disable : 4710)  // function is not inlined
#pragma warning(disable : 4820)  // bytes padding added after data member
#pragma warning(disable : 5045)  // Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
#endif
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


template<typename TestType>
void EnumerateSubnormals(float topOfRange, size_t bitRange) {
	TestType a;
	float testValue = topOfRange;
	for (size_t i = 0; i < bitRange; ++i) {
		a = testValue;
		std::cout << sw::universal::to_binary(a, true) << " : " << sw::universal::color_print(a) << " : " << a << '\n';
		std::cout << sw::universal::to_binary(testValue, true) << " : " << testValue << "\n---\n";
		testValue *= 0.5f;
	}
}

// conditional compile flags
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	if (argc > 0) {
		std::cout << argv[0] << std::endl;
	}

	int nrOfFailedTestCases = 0;
	std::string tag = "conversion: ";

#if MANUAL_TESTING

	// bfloat<> is a linear floating-point

#if 1
	// to track conversion in more detail
	std::cout << std::setprecision(8);
	std::cerr << std::setprecision(8);
#endif
//	EnumerateSubnormals<bfloat<6, 2, uint8_t>>(1.0f, 6);

	{
		bfloat<6,2> a;
		a.debug();
		std::cout << "maxpos : " << maxpos(a) << " : " << scale(a) << '\n';
		std::cout << "minpos : " << minpos(a) << " : " << scale(a) << '\n';
		std::cout << "zero   : " << zero(a)   << " : " << scale(a) << '\n';
		std::cout << "minneg : " << minneg(a) << " : " << scale(a) << '\n';
		std::cout << "maxneg : " << maxneg(a) << " : " << scale(a) << '\n';
		float testValue = 8.0f;
		a = testValue;
		float f = float(a);
		std::cout << to_binary(a) << " : " << a << " : " << f << " : " << setprecision(8) << testValue << endl;
	}

	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<4, 1, uint8_t>, float >(false), tag, "bfloat<4,1,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<5, 1, uint8_t>, float >(false), tag, "bfloat<5,1,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<5, 2, uint8_t>, float >(false), tag, "bfloat<5,2,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<6, 2, uint8_t>, float >(false), tag, "bfloat<6,2,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<7, 2, uint8_t>, float >(false), tag, "bfloat<7,2,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<8, 2, uint8_t>, float >(false), tag, "bfloat<8,2,uint8_t>");

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
	std::cerr << msg << std::endl;
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
