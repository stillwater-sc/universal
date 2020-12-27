// multiplication.cpp: verification tests for block triple number division
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <typeinfo>

// minimum set of include files to reflect source code dependencies
#include <universal/blockbin/blocktriple.hpp>
#include <universal/verification/binaryop_status.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult

template<size_t ebits, size_t fbits, typename BlockType = uint8_t>
std::string to_binary(const sw::universal::blocktriple<ebits, fbits, BlockType>& a) {
	std::stringstream ss;
	return ss.str();
}

// The verification suite uses long doubles as reference.
// This implies that verification routines will behave differently on Visual Studio and GCC/Clang.
// The CI regression suite runs with GCC, so we are testing with extended precision effectively.
//
// enumerate all multiplication cases for a blocktriple<ebits,fbits,BlockType> configuration
template<size_t ebits, size_t fbits, typename BlockType = uint8_t>
int VerifyMultiplication(const std::string& tag, bool bReportIndividualTestCases) {
	constexpr size_t NR_VALUES = (size_t(1) << fbits);
	using namespace std;
	using namespace sw::universal;

	cout << endl;
	cout << "blocktriple<" << ebits << ',' << fbits << ',' << typeid(BlockType).name() << '>' << endl;

	//bool bReportOverflowCondition = false;
	int nrOfFailedTests = 0;

	return nrOfFailedTests;
}

// conditional compile flags
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	std::string tag = "blocktriple division: ";

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 8, uint8_t>(tag, bReportIndividualTestCases), "blocktriple<4>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 8, uint8_t>(tag, bReportIndividualTestCases), "blocktriple<8>", "division");


#if STRESS_TESTING

#endif

#else

	cout << "blocktriple division validation" << endl;

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 4, uint8_t>(tag, bReportIndividualTestCases), "blocktriple<8, 4,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 5, uint8_t>(tag, bReportIndividualTestCases), "blocktriple<8, 5,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 6, uint8_t>(tag, bReportIndividualTestCases), "blocktriple<8, 6,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 7, uint8_t>(tag, bReportIndividualTestCases), "blocktriple<8, 7,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 8, uint8_t>(tag, bReportIndividualTestCases), "blocktriple<8, 8,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 9, uint8_t>(tag, bReportIndividualTestCases), "blocktriple<8, 9,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 10, uint8_t>(tag, bReportIndividualTestCases), "blocktriple<8, 10,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 12, uint8_t>(tag, bReportIndividualTestCases), "blocktriple<8, 12,uint8_t>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 9, uint16_t>(tag, bReportIndividualTestCases), "blocktriple<8, 9,uint16_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 11, uint16_t>(tag, bReportIndividualTestCases), "blocktriple<8, 11,uint16_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 13, uint16_t>(tag, bReportIndividualTestCases), "blocktriple<8, 13,uint16_t>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 12, uint32_t>(tag, bReportIndividualTestCases), "blocktriple<8, 12,uint32_t>", "division");

#if STRESS_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 16, uint8_t>(tag, bReportIndividualTestCases), "blocktriple<8, 16,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<8, 16, uint16_t>(tag, bReportIndividualTestCases), "blocktriple<8, 16,uint16_t>", "division");


#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
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
