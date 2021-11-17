// multiplication.cpp: test suite runner for multiplication of arbitrary logarithmic number system
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// minimum set of include files to reflect source code dependencies
#include <universal/number/lns/lns.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/verification/test_reporters.hpp>
#include <universal/verification/test_case.hpp>

template<size_t nbits> 
int ValidateMultiplication(const std::string& tag, bool bReportIndividualTestCases) {
	int nrOfFailedTestCases = 0;

	return nrOfFailedTestCases;
}

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	std::string test_suite = "lns multiplication validation";
	std::string test_tag = "multiplication";
	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

#if MANUAL_TESTING

	// generate individual testcases to hand trace/debug
	TestCase<lns<16, uint8_t>, double>(TestCaseOperator::MUL, INFINITY, INFINITY);
	TestCase<lns<8, uint8_t>, float>(TestCaseOperator::MUL, 0.5f, -0.5f);

	constexpr double e = 2.71828182845904523536;
	lns<16> a, b, c;
	a = 0.5; std::cout << a << '\n';
	a = e; std::cout << a << '\n';
	b = 1.0 / e;
	c = a * b;
	std::cout << c.to_long_double() << '\n';

	// manual exhaustive test
	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<8>("Manual Testing", true), "lns<8>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else
	std::cout << "Arbitrary LNS multiplication validation\n";

	bool bReportIndividualTestCases = false;
	std::string tag = "multiplication failed: ";

	nrOfFailedTestCases += ReportTestResult(ValidateMultiplication<8>(tag, bReportIndividualTestCases), "lns<8>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
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
