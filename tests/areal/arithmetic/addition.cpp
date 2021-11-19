// addition.cpp: test suite runner for addition on arbitrary reals
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#if defined(_MSC_VER)
#pragma warning(disable : 4514)  // unreferenced function is removed
#pragma warning(disable : 4710)  // function is not inlined
#pragma warning(disable : 5045)  // Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
#endif

#include <universal/number/areal/areal.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_case.hpp>
#include <universal/verification/test_suite_arithmetic.hpp>

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	if (argc > 1) {
		for (int i = 0; i < argc; ++i) {
			std::cout << argv[i] << ' ';
		}
		std::cout << std::endl;
	}
	int nrOfFailedTestCases = 0;

#if MANUAL_TESTING

	// generate individual testcases to hand trace/debug
	TestCase<areal<16, 8, uint8_t>, double>(TestCaseOperator::ADD, INFINITY, INFINITY);
	TestCase<areal<8, 4, uint8_t>, float>(TestCaseOperator::ADD, 0.5f, -0.5f);

	// manual exhaustive test
	//nrOfFailedTestCases += ReportTestResult(VerifyAddition< areal<8, 2, uint8_t> >("Manual Testing", true), "areal<8,2,uint8_t>", "addition");

	
	nrOfFailedTestCases = 0;

#else
	cout << "Arbitrary Real addition validation" << endl;

	bool bReportIndividualTestCases = false;
	std::string tag = "Addition failed: ";

	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 2>(tag, bReportIndividualTestCases), "areal<8,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 4>(tag, bReportIndividualTestCases), "areal<8,4>", "addition");

#if STRESS_TESTING

	nrOfFailedTestCases += ReportTestResult(ValidateAddition<10, 4>(tag, bReportIndividualTestCases), "areal<10,4>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<16, 8>(tag, bReportIndividualTestCases), "areal<16,8>", "addition");
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
