// arithmetic_add.cpp: functional tests for valid addition
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>

// Configure the valid template environment
// first: enable  general or specialized posit configurations
//#define VALID_FAST_SPECIALIZATION
// second: enable/disable posit arithmetic exceptions
#define VALID_THROW_ARITHMETIC_EXCEPTION 0
// third: enable tracing 
// when you define VALID_VERBOSE_OUTPUT executing an ADD the code will print intermediate results
//#define VALID_VERBOSE_OUTPUT
#define VALID_TRACE_SUB

#include <universal/number/valid/valid.hpp>
#include <universal/number/valid/manipulators.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/verification/test_reporters.hpp>


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

	std::string test_suite  = "valid addition validation";
	std::string test_tag    = "addition";
//	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug
	constexpr size_t nbits = 16;
	constexpr size_t es = 1;
	valid<nbits, es> v1, v2;

	v1.clear();
	std::cout << v1 << '\n';

	v2.setinclusive();
	std::cout << v2 << '\n';

	v1 = 1;
	std::cout << v1 << '\n';

	posit<nbits, es> lb(1.25f), ub(1.375f);
	v2.setlb(lb, false);
	v2.setub(ub, true);
	std::cout << v2 << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<3, 0>(tag, reportTestCases), "valid<3,0>", "addition");

	nrOfFailedTestCases += ReportTestResult(ValidateAddition<4, 0>(tag, reportTestCases), "valid<4,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<4, 1>(tag, reportTestCases), "valid<4,1>", "addition");

	nrOfFailedTestCases += ReportTestResult(ValidateAddition<5, 0>(tag, reportTestCases), "valid<5,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<5, 1>(tag, reportTestCases), "valid<5,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<5, 2>(tag, reportTestCases), "valid<5,2>", "addition");

	nrOfFailedTestCases += ReportTestResult(ValidateAddition<6, 0>(tag, reportTestCases), "valid<6,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<6, 1>(tag, reportTestCases), "valid<6,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<6, 2>(tag, reportTestCases), "valid<6,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<6, 3>(tag, reportTestCases), "valid<6,3>", "addition");

	nrOfFailedTestCases += ReportTestResult(ValidateAddition<7, 0>(tag, reportTestCases), "valid<7,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<7, 1>(tag, reportTestCases), "valid<7,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<7, 2>(tag, reportTestCases), "valid<7,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<7, 3>(tag, reportTestCases), "valid<7,3>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<7, 4>(tag, reportTestCases), "valid<7,4>", "addition");

	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 0>(tag, reportTestCases), "valid<8,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 1>(tag, reportTestCases), "valid<8,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 2>(tag, reportTestCases), "valid<8,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 3>(tag, reportTestCases), "valid<8,3>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 4>(tag, reportTestCases), "valid<8,4>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<8, 5>(tag, reportTestCases), "valid<8,5>", "addition");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<16, 1>(tag, reportTestCases, OPCODE_ADD, 1000), "valid<16,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<24, 1>(tag, reportTestCases, OPCODE_ADD, 1000), "valid<24,1>", "addition");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<32, 1>(tag, reportTestCases, OPCODE_ADD, 1000), "valid<32,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<32, 2>(tag, reportTestCases, OPCODE_ADD, 1000), "valid<32,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<48, 2>(tag, reportTestCases, OPCODE_ADD, 1000), "valid<48,2>", "addition");
#endif

#if REGRESSION_LEVEL_4
	// nbits=64 requires long double compiler support
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<64, 2>(tag, reportTestCases, OPCODE_ADD, 1000), "valid<64,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<64, 3>(tag, reportTestCases, OPCODE_ADD, 1000), "valid<64,3>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateThroughRandoms<64, 4>(tag, reportTestCases, OPCODE_ADD, 1000), "valid<64,4>", "addition");


	nrOfFailedTestCases += ReportTestResult(ValidateAddition<10, 1>(tag, reportTestCases), "valid<10,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<12, 1>(tag, reportTestCases), "valid<12,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<14, 1>(tag, reportTestCases), "valid<14,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(ValidateAddition<16, 1>(tag, reportTestCases), "valid<16,1>", "addition");
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
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
