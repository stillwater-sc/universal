// randoms.cpp: test suite runner for larger posits that are too big for an exhaustive enumeration
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// Configure the posit template environment
// first: enable general or specialized posit configurations
//#define POSIT_FAST_SPECIALIZATION
// second: enable/disable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 0
// third: enable tracing 
// when you define ALGORITHM_VERBOSE_OUTPUT executing an ADD the code will print intermediate results
//#define ALGORITHM_VERBOSE_OUTPUT
//#define ALGORITHM_TRACE_ADD
#include <universal/number/posit1/posit1.hpp>
#include <universal/verification/posit_test_suite.hpp>
#include <universal/verification/posit_test_suite_randoms.hpp>

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "posit randoms verification";
	std::string test_tag = "randoms";
	bool reportTestCases = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<64, 2>(reportTestCases, OPCODE_ADD, 1000), "posit<64,2>", "addition");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	{
		using TestType = posit<16, 2>;
		std::string typeTag = type_tag(TestType());
		nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_ADD, 1000), typeTag, "addition");
		nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_SUB, 1000), typeTag, "subtraction");
		nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_MUL, 1000), typeTag, "multiplication");
		nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_DIV, 1000), typeTag, "division");
	}
#endif

#if REGRESSION_LEVEL_2
	{
		using TestType = posit<32, 2>;
		std::string typeTag = type_tag(TestType());
		nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_ADD, 1000), typeTag, "addition");
		nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_SUB, 1000), typeTag, "subtraction");
		nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_MUL, 1000), typeTag, "multiplication");
		nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_DIV, 1000), typeTag, "division");
	}
#endif

#if REGRESSION_LEVEL_3
	{
		using TestType = posit<64, 2>;
		std::string typeTag = type_tag(TestType());
		nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_ADD, 1000), typeTag, "addition");
		nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_SUB, 1000), typeTag, "subtraction");
		nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_MUL, 1000), typeTag, "multiplication");
		nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_DIV, 1000), typeTag, "division");
	}
#endif

#if REGRESSION_LEVEL_4
	{
		using TestType = posit<128, 2>;
		std::string typeTag = type_tag(TestType());
		nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_ADD, 1000), typeTag, "addition");
		nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_SUB, 1000), typeTag, "subtraction");
		nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_MUL, 1000), typeTag, "multiplication");
		nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_DIV, 1000), typeTag, "division");
	}
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
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
