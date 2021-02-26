// double_conversion.cpp: test suite runner for double conversions to bfloats
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

// sign of 0 is flipped on MSVC Release builds
void CompilerBug() {
	using namespace std;
	using namespace sw::universal;
	{
		bfloat<5, 1> a;
		a.set_raw_bits(0x0);
		cout << "bfloat<5,1> : " << to_binary(a) << " : " << a << endl;
		float f = float(a);
		cout << "float      : " << f << endl;
		double d = double(a);
		cout << "double     : " << d << endl;
	}
	{
		bfloat<5, 1> a;
		a.set_raw_bits(0x10);
		cout << "bfloat<5,1> : " << to_binary(a) << " : " << a << endl;
		float f = float(a);
		cout << "float      : " << f << endl;
		double d = double(a);
		cout << "double     : " << d << endl;
	}

	{
		bfloat<6, 1> a;
		a.set_raw_bits(0x0);
		cout << "bfloat<6,1> : " << to_binary(a) << " : " << a << endl;
		float f = float(a);
		cout << "float      : " << f << endl;
		double d = double(a);
		cout << "double     : " << d << endl;
	}
	{
		bfloat<6, 1> a;
		a.set_raw_bits(0x20);
		cout << "bfloat<6,1> : " << to_binary(a) << " : " << a << endl;
		float f = float(a);
		cout << "float      : " << f << endl;
		double d = double(a);
		cout << "double     : " << d << endl;
	}
}

// conditional compile flags
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	print_cmd_line(argc, argv);

	int nrOfFailedTestCases = 0;
	std::string tag = "double conversion: ";

#if MANUAL_TESTING

	// to track conversion in more detail
	std::cout << std::setprecision(15);
	std::cerr << std::setprecision(15);

	{
		bfloat<6, 2> a;
		a.constexprParameters();
		double testValue = 0.0625000074505806;
		a = testValue;
		double da = double(a);
		std::cout << to_binary(a) << " : " << a << " : " << da << " : " << setprecision(8) << testValue << endl;
	}

	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<4, 1, uint8_t>, double >(false), tag, "bfloat<4,1,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<5, 1, uint8_t>, double >(false), tag, "bfloat<5,1,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<5, 2, uint8_t>, double >(false), tag, "bfloat<5,2,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<6, 2, uint8_t>, double >(true), tag, "bfloat<6,2,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<7, 2, uint8_t>, double >(false), tag, "bfloat<7,2,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<8, 2, uint8_t>, double >(false), tag, "bfloat<8,2,uint8_t>");

	std::cout << "failed tests: " << nrOfFailedTestCases << endl;
	nrOfFailedTestCases = 0; // in manual testing we ignore failures for the regression system

#if STRESS_TESTING

	// manual exhaustive test

#endif

#else  // !MANUAL_TESTING
	bool bReportIndividualTestCases = false;
	cout << "BFLOAT conversion from double validation" << endl;

	// es = 1
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<4, 1>, double >(bReportIndividualTestCases), tag, "bfloat<4,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<5, 1>, double >(bReportIndividualTestCases), tag, "bfloat<5,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<6, 1>, double >(bReportIndividualTestCases), tag, "bfloat<6,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<7, 1>, double >(bReportIndividualTestCases), tag, "bfloat<7,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<8, 1>, double >(bReportIndividualTestCases), tag, "bfloat<8,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<9, 1>, double >(bReportIndividualTestCases), tag, "bfloat<9,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<10, 1>, double >(bReportIndividualTestCases), tag, "bfloat<10,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<12, 1>, double >(bReportIndividualTestCases), tag, "bfloat<12,1>");


	// es = 2
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<5, 2>, double >(bReportIndividualTestCases), tag, "bfloat<5,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<6, 2>, double >(bReportIndividualTestCases), tag, "bfloat<6,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<7, 2>, double >(bReportIndividualTestCases), tag, "bfloat<7,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<8, 2>, double >(bReportIndividualTestCases), tag, "bfloat<8,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<10, 2>, double >(bReportIndividualTestCases), tag, "bfloat<10,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<12, 2>, double >(bReportIndividualTestCases), tag, "bfloat<12,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<14, 2>, double >(bReportIndividualTestCases), tag, "bfloat<14,2>");


	// es = 3
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<6, 3>, double >(bReportIndividualTestCases), tag, "bfloat<6,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<7, 3>, double >(bReportIndividualTestCases), tag, "bfloat<7,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<8, 3>, double >(bReportIndividualTestCases), tag, "bfloat<8,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<10, 3>, double >(bReportIndividualTestCases), tag, "bfloat<10,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<12, 3>, double >(bReportIndividualTestCases), tag, "bfloat<12,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<14, 3>, double >(bReportIndividualTestCases), tag, "bfloat<14,3>");


	// es = 4
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<7, 4>, double >(bReportIndividualTestCases), tag, "bfloat<7,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<8, 4>, double >(bReportIndividualTestCases), tag, "bfloat<8,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<10, 4>, double >(bReportIndividualTestCases), tag, "bfloat<10,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<12, 4>, double >(bReportIndividualTestCases), tag, "bfloat<12,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<14, 4>, double >(bReportIndividualTestCases), tag, "bfloat<14,4>");


	// es = 5
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<8, 5>, double >(bReportIndividualTestCases), tag, "bfloat<8,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<10, 5>, double >(bReportIndividualTestCases), tag, "bfloat<10,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<12, 5>, double >(bReportIndividualTestCases), tag, "bfloat<12,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<14, 5>, double >(bReportIndividualTestCases), tag, "bfloat<14,5>");

#ifdef LATER
	// es = 6
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<9, 6>, double >(bReportIndividualTestCases), tag, "bfloat<9,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<10, 6>, double >(bReportIndividualTestCases), tag, "bfloat<10,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<12, 6>, double >(bReportIndividualTestCases), tag, "bfloat<12,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<14, 6>, double >(bReportIndividualTestCases), tag, "bfloat<14,6>");


	// es = 7
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<10, 7>, double >(bReportIndividualTestCases), tag, "bfloat<10,7>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<12, 7>, double >(bReportIndividualTestCases), tag, "bfloat<12,7>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<14, 7>, double >(bReportIndividualTestCases), tag, "bfloat<14,7>");


	// es = 8
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<11, 8>, double >(bReportIndividualTestCases), tag, "bfloat<11,8>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<12, 8>, double >(bReportIndividualTestCases), tag, "bfloat<12,8>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatConversion< bfloat<14, 8>, double >(bReportIndividualTestCases), tag, "bfloat<14,8>");

#endif // LATER

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
