// double_conversion.cpp: test suite runner for double conversions to areals
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#if defined(_MSC_VER)
#pragma warning(disable : 4514)  // unreferenced function is removed
#pragma warning(disable : 4710)  // function is not inlined
#pragma warning(disable : 5045)  // Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
#endif
#include <iostream>
#include <iomanip>
// Configure the areal template environment
// first: enable general or specialized configurations
#define AREAL_FAST_SPECIALIZATION
// second: enable/disable arithmetic exceptions
#define AREAL_THROW_ARITHMETIC_EXCEPTION 0
// third: enable trace conversion
#define TRACE_CONVERSION 0

#include <universal/number/areal/areal.hpp>
#include <universal/verification/test_suite_arithmetic.hpp>
#include <universal/verification/areal_test_suite.hpp>
#include <universal/number/areal/table.hpp> // only used for value table generation

// sign of 0 is flipped on MSVC Release builds
void CompilerBug() {
	using namespace sw::universal;
	{
		areal<5, 1> a;
		a.setbits(0x0);
		std::cout << "areal<5,1> : " << to_binary(a) << " : " << a << std::endl;
		float f = float(a);
		std::cout << "float      : " << f << std::endl;
		double d = double(a);
		std::cout << "double     : " << d << std::endl;
	}
	{
		areal<5, 1> a;
		a.setbits(0x10);
		std::cout << "areal<5,1> : " << to_binary(a) << " : " << a << std::endl;
		float f = float(a);
		std::cout << "float      : " << f << std::endl;
		double d = double(a);
		std::cout << "double     : " << d << std::endl;
	}

	{
		areal<6, 1> a;
		a.setbits(0x0);
		std::cout << "areal<6,1> : " << to_binary(a) << " : " << a << std::endl;
		float f = float(a);
		std::cout << "float      : " << f << std::endl;
		double d = double(a);
		std::cout << "double     : " << d << std::endl;
	}
	{
		areal<6, 1> a;
		a.setbits(0x20);
		std::cout << "areal<6,1> : " << to_binary(a) << " : " << a << std::endl;
		float f = float(a);
		std::cout << "float      : " << f << std::endl;
		double d = double(a);
		std::cout << "double     : " << d << std::endl;
	}
}

// conditional compile flags
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	if (argc > 0) {
		std::cout << argv[0] << std::endl;
	}

	int nrOfFailedTestCases = 0;
	std::string tag = "conversion: ";

#if MANUAL_TESTING

	bool bReportIndividualTestCases = false;

	// areal<> is organized as a set of exact samples and an interval to the next exact value
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

//	GenerateArealTable<9, 6>(cout, true);  // ok
//	GenerateArealTable<10, 7>(cout, true); // fails

	areal<10, 7> a;
	a.setbits(0x1F6);  // b01'1111'0110;
	cout << to_binary(a) << " : " << a << endl;
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<10, 7, uint8_t> >(tag, true), tag, "areal<10,7,uint8_t>");
//	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<10, 7, uint16_t> >(tag, true), tag, "areal<10,7,uint16_t>");

	std::cout << "failed tests: " << nrOfFailedTestCases << std::endl;

#if STRESS_TESTING

	// manual exhaustive test

#endif

#else  // !MANUAL_TESTING
	bool bReportIndividualTestCases = false;
	std::cout << "AREAL conversion from double validation\n";

	// es = 1
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<4, 1>, double >(tag, bReportIndividualTestCases), tag, "areal<4,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<5, 1>, double >(tag, bReportIndividualTestCases), tag, "areal<5,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<6, 1>, double >(tag, bReportIndividualTestCases), tag, "areal<6,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<7, 1>, double >(tag, bReportIndividualTestCases), tag, "areal<7,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<8, 1>, double >(tag, bReportIndividualTestCases), tag, "areal<8,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<9, 1>, double >(tag, bReportIndividualTestCases), tag, "areal<9,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<10, 1>, double >(tag, bReportIndividualTestCases), tag, "areal<10,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<12, 1>, double >(tag, bReportIndividualTestCases), tag, "areal<12,1>");


	// es = 2
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<5, 2>, double >(tag, bReportIndividualTestCases), tag, "areal<5,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<6, 2>, double >(tag, bReportIndividualTestCases), tag, "areal<6,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<7, 2>, double >(tag, bReportIndividualTestCases), tag, "areal<7,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<8, 2>, double >(tag, bReportIndividualTestCases), tag, "areal<8,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<10, 2>, double >(tag, bReportIndividualTestCases), tag, "areal<10,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<12, 2>, double >(tag, bReportIndividualTestCases), tag, "areal<12,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<14, 2>, double >(tag, bReportIndividualTestCases), tag, "areal<14,2>");


	// es = 3
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<6, 3>, double >(tag, bReportIndividualTestCases), tag, "areal<6,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<7, 3>, double >(tag, bReportIndividualTestCases), tag, "areal<7,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<8, 3>, double >(tag, bReportIndividualTestCases), tag, "areal<8,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<10, 3>, double >(tag, bReportIndividualTestCases), tag, "areal<10,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<12, 3>, double >(tag, bReportIndividualTestCases), tag, "areal<12,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<14, 3>, double >(tag, bReportIndividualTestCases), tag, "areal<14,3>");


	// es = 4
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<7, 4>, double >(tag, bReportIndividualTestCases), tag, "areal<7,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<8, 4>, double >(tag, bReportIndividualTestCases), tag, "areal<8,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<10, 4>, double >(tag, bReportIndividualTestCases), tag, "areal<10,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<12, 4>, double >(tag, bReportIndividualTestCases), tag, "areal<12,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<14, 4>, double >(tag, bReportIndividualTestCases), tag, "areal<14,4>");


	// es = 5
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<8, 5>, double >(tag, bReportIndividualTestCases), tag, "areal<8,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<10, 5>, double >(tag, bReportIndividualTestCases), tag, "areal<10,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<12, 5>, double >(tag, bReportIndividualTestCases), tag, "areal<12,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<14, 5>, double >(tag, bReportIndividualTestCases), tag, "areal<14,5>");


	// es = 6
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<9, 6>, double >(tag, bReportIndividualTestCases), tag, "areal<9,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<10, 6>, double >(tag, bReportIndividualTestCases), tag, "areal<10,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<12, 6>, double >(tag, bReportIndividualTestCases), tag, "areal<12,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<14, 6>, double >(tag, bReportIndividualTestCases), tag, "areal<14,6>");


	// es = 7
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<10, 7>, double >(tag, true), tag, "areal<10,7>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<12, 7>, double >(tag, bReportIndividualTestCases), tag, "areal<12,7>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<14, 7>, double >(tag, bReportIndividualTestCases), tag, "areal<14,7>");

#if LATER
	// es = 8
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<11, 8>, double >(tag, bReportIndividualTestCases), tag, "areal<11,8>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<12, 8>, double >(tag, bReportIndividualTestCases), tag, "areal<12,8>");
	nrOfFailedTestCases = ReportTestResult(VerifyArealIntervalConversion< areal<14, 8>, double >(tag, bReportIndividualTestCases), tag, "areal<14,8>");

#endif // LATER

#if STRESS_TESTING

#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
