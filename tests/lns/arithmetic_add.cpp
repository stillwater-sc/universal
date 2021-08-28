// arithmetic_add.cpp: test suite runner for addition on arbitrary logarithmic number system
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// minimum set of include files to reflect source code dependencies
#include <universal/number/lns/lns_impl.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/verification/test_case.hpp>

template<size_t nbits> 
int ValidateAddition(const std::string& tag, bool bReportIndividualTestCases) {
	int nrOfFailedTestCases = 0;

	return nrOfFailedTestCases;
}

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;

#if MANUAL_TESTING

	// generate individual testcases to hand trace/debug
	TestCase< lns<16, uint8_t>, double>(TestCaseOperator::ADD, INFINITY, INFINITY);
	TestCase< lns<8, uint8_t>, float>(TestCaseOperator::ADD, 0.5f, -0.5f);

	// manual exhaustive test
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8>("Manual Testing", true), "lns<8>", "addition");

	nrOfFailedTestCases = 0;
#else
	std::cout << "Arbitrary LNS addition validation\n";

	bool bReportIndividualTestCases = false;
	std::string tag = "Addition failed: ";

	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8>(tag, bReportIndividualTestCases), "lns<8>", "addition");

#if STRESS_TESTING

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
