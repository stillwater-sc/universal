// arithmetic.cpp: test suite runner for arithmetic on bfloat16s
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/dd/dd.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/test_suite_arithmetic.hpp>
#include <universal/verification/test_suite_randoms.hpp>
#include <universal/verification/cfloat_test_suite.hpp>

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

	std::string test_suite         = "doubledouble arithmetic validation";
	std::string test_tag           = "doubledouble arithmetic";
	bool reportTestCases           = false;
	int nrOfFailedTestCases        = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// generate individual testcases to hand trace/debug
	TestCase< bfloat16, float>(TestCaseOperator::ADD, 1.0f, 1.0f);
	TestCase< cfloat<16, 8, uint16_t, true, true, false>, double>(TestCaseOperator::ADD, INFINITY, INFINITY);


	nrOfFailedTestCases += ReportTestResult(
		VerifyCfloatAddition< cfloat<8, 2, uint8_t, true, true, false> >(reportTestCases),
		"cfloat<8,2,uint8_t,t,t,f>", "addition"
	);

	nrOfFailedTestCases += ReportTestResult(
		VerifyBinaryOperatorThroughRandoms<bfloat16>(reportTestCases, RandomsOp::OPCODE_ADD, 1000),
		"bfloat16", "addition"
	);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else  // !MANUAL_TESTING

#if REGRESSION_LEVEL_1

	constexpr unsigned nrOfRandoms = 1000;
	std::stringstream adds;
	adds << test_tag << " " << nrOfRandoms << " random adds";
	std::string description = adds.str();
	nrOfFailedTestCases += ReportTestResult(
		VerifyBinaryOperatorThroughRandoms<dd>(reportTestCases, RandomsOp::OPCODE_ADD, nrOfRandoms),
		description, 
		test_tag
	); 
	std::stringstream subs;
	subs << test_tag << " " << nrOfRandoms << " random subs";
	description = subs.str();
	nrOfFailedTestCases += ReportTestResult(
		VerifyBinaryOperatorThroughRandoms<dd>(reportTestCases, RandomsOp::OPCODE_SUB, nrOfRandoms),
		description, 
		test_tag
	); 
	std::stringstream muls;
	muls << test_tag << " " << nrOfRandoms << " random muls";
	description = muls.str();
	nrOfFailedTestCases += ReportTestResult(
		VerifyBinaryOperatorThroughRandoms<dd>(reportTestCases, RandomsOp::OPCODE_MUL, nrOfRandoms),
		description, 
		test_tag
	); 
	std::stringstream divs;
	divs << test_tag << " " << nrOfRandoms << " random divs";
	description = divs.str();
	nrOfFailedTestCases += ReportTestResult(
		VerifyBinaryOperatorThroughRandoms<dd>(reportTestCases, RandomsOp::OPCODE_DIV, nrOfRandoms),
		description, 
		test_tag
	); 

#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
