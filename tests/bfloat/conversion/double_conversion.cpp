// double_conversion.cpp: test suite runner for double conversions to bfloats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
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
#include <universal/verification/test_suite_arithmetic.hpp>
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

	if (argc > 0) {
		std::cout << argv[0] << std::endl;
	}

	int nrOfFailedTestCases = 0;
	std::string tag = "conversion: ";

#if MANUAL_TESTING

	bool bReportIndividualTestCases = false;

	// bfloat<> is organized as a set of exact samples and an interval to the next exact value
	//
	// vprev    exact value          ######-0     ubit = false     some value [vprev,vprev]
	//          interval value       ######-1     ubit = true      (vprev, v)
	// v        exact value          ######-0     ubit = false     some value [v,v]
	//          interval value       ######-1     ubit = true      (v, vnext)
	// vnext    exact value          ######-0     ubit = false     some value [vnext,vnext]
	//          interval value       ######-1     ubit = true      (vnext, vnextnext)
	//
	// the assignment test can thus be constructed by enumerating the exact values
	// and taking a -diff to obtain the interval value of vprev, 
	// and taking a +diff to obtain the interval value of v

//	GenerateBfloatTable<9, 6>(cout, true);  // ok
//	GenerateBfloatTable<10, 7>(cout, true); // fails

	bfloat<10, 7> a;
	a.set_raw_bits(0x1F6);  // b01'1111'0110;
	cout << to_binary(a) << " : " << a << endl;
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<10, 7, uint8_t> >(tag, true), tag, "bfloat<10,7,uint8_t>");
//	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<10, 7, uint16_t> >(tag, true), tag, "bfloat<10,7,uint16_t>");

	cout << "failed tests: " << nrOfFailedTestCases << endl;

#if STRESS_TESTING

	// manual exhaustive test

#endif

#else  // !MANUAL_TESTING
	bool bReportIndividualTestCases = false;
	cout << "BFLOAT conversion from double validation" << endl;

	// es = 1
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<4, 1>, double >(tag, bReportIndividualTestCases), tag, "bfloat<4,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<5, 1>, double >(tag, bReportIndividualTestCases), tag, "bfloat<5,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<6, 1>, double >(tag, bReportIndividualTestCases), tag, "bfloat<6,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<7, 1>, double >(tag, bReportIndividualTestCases), tag, "bfloat<7,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<8, 1>, double >(tag, bReportIndividualTestCases), tag, "bfloat<8,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<9, 1>, double >(tag, bReportIndividualTestCases), tag, "bfloat<9,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<10, 1>, double >(tag, bReportIndividualTestCases), tag, "bfloat<10,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<12, 1>, double >(tag, bReportIndividualTestCases), tag, "bfloat<12,1>");


	// es = 2
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<5, 2>, double >(tag, bReportIndividualTestCases), tag, "bfloat<5,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<6, 2>, double >(tag, bReportIndividualTestCases), tag, "bfloat<6,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<7, 2>, double >(tag, bReportIndividualTestCases), tag, "bfloat<7,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<8, 2>, double >(tag, bReportIndividualTestCases), tag, "bfloat<8,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<10, 2>, double >(tag, bReportIndividualTestCases), tag, "bfloat<10,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<12, 2>, double >(tag, bReportIndividualTestCases), tag, "bfloat<12,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<14, 2>, double >(tag, bReportIndividualTestCases), tag, "bfloat<14,2>");


	// es = 3
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<6, 3>, double >(tag, bReportIndividualTestCases), tag, "bfloat<6,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<7, 3>, double >(tag, bReportIndividualTestCases), tag, "bfloat<7,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<8, 3>, double >(tag, bReportIndividualTestCases), tag, "bfloat<8,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<10, 3>, double >(tag, bReportIndividualTestCases), tag, "bfloat<10,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<12, 3>, double >(tag, bReportIndividualTestCases), tag, "bfloat<12,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<14, 3>, double >(tag, bReportIndividualTestCases), tag, "bfloat<14,3>");


	// es = 4
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<7, 4>, double >(tag, bReportIndividualTestCases), tag, "bfloat<7,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<8, 4>, double >(tag, bReportIndividualTestCases), tag, "bfloat<8,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<10, 4>, double >(tag, bReportIndividualTestCases), tag, "bfloat<10,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<12, 4>, double >(tag, bReportIndividualTestCases), tag, "bfloat<12,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<14, 4>, double >(tag, bReportIndividualTestCases), tag, "bfloat<14,4>");


	// es = 5
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<8, 5>, double >(tag, bReportIndividualTestCases), tag, "bfloat<8,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<10, 5>, double >(tag, bReportIndividualTestCases), tag, "bfloat<10,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<12, 5>, double >(tag, bReportIndividualTestCases), tag, "bfloat<12,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<14, 5>, double >(tag, bReportIndividualTestCases), tag, "bfloat<14,5>");


	// es = 6
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<9, 6>, double >(tag, bReportIndividualTestCases), tag, "bfloat<9,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<10, 6>, double >(tag, bReportIndividualTestCases), tag, "bfloat<10,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<12, 6>, double >(tag, bReportIndividualTestCases), tag, "bfloat<12,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<14, 6>, double >(tag, bReportIndividualTestCases), tag, "bfloat<14,6>");


	// es = 7
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<10, 7>, double >(tag, true), tag, "bfloat<10,7>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<12, 7>, double >(tag, bReportIndividualTestCases), tag, "bfloat<12,7>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<14, 7>, double >(tag, bReportIndividualTestCases), tag, "bfloat<14,7>");

#if LATER
	// es = 8
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<11, 8>, double >(tag, bReportIndividualTestCases), tag, "bfloat<11,8>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<12, 8>, double >(tag, bReportIndividualTestCases), tag, "bfloat<12,8>");
	nrOfFailedTestCases = ReportTestResult(VerifyBfloatIntervalConversion< bfloat<14, 8>, double >(tag, bReportIndividualTestCases), tag, "bfloat<14,8>");

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
