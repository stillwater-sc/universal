// reference_type_generic.cpp: the generic random-operand verifier picks the exact reference for posit (#2121)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <iostream>
#include <string>
#include <type_traits>
#include <universal/number/posit/posit.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/test_suite_randoms.hpp>   // the generic verifier only, not posit_test_suite_randoms.hpp

/*
   This file includes posit and the generic verifier, and nothing posit-specific from the
   verification directory. The posit exact reference must still be visible, or the default
   reference of posit<64,2> would silently be double, which cannot hold its results.

   It also covers the range rule: posit<32,6> has 24 digits, fewer than double, but reaches
   2^1920, so its default reference is the exact one too. And with the exact reference, a
   mismatch on an operand double sees as zero or inf is a failure, not skipped: posit<256,4>
   operands built from 64 random low bits are far below double's range.
*/

namespace sw { namespace universal {

static_assert(has_exact_rounding<posit<64, 2>>, "the posit exact reference is visible through the generic verifier");
static_assert(std::is_same_v<default_reference_t<posit<64, 2>>, exact_reference>);
static_assert(std::is_same_v<default_reference_t<posit<32, 6>>, exact_reference>);   // the range, not the digits
static_assert(std::is_same_v<default_reference_t<posit<32, 2>>, double>);
static_assert(!valid_reference_v<posit<64, 2>, double>);
static_assert(!valid_reference_v<posit<32, 6>, double>);
static_assert(std::is_same_v<default_reference_t<posit<32, 5>>, exact_reference>);   // products past double

}}  // namespace sw::universal

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

	std::string test_suite  = "generic verifier with the posit exact reference";
	std::string test_tag    = "reference type";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<64, 2>>(true, RandomsOp::OPCODE_DIV, 100), "posit<64,2>", "division");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<64, 2>>(reportTestCases, RandomsOp::OPCODE_DIV, 500), "posit<64,2>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<64, 4>>(reportTestCases, RandomsOp::OPCODE_MUL, 500), "posit<64,4>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<32, 6>>(reportTestCases, RandomsOp::OPCODE_ADD, 500), "posit<32,6>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<32, 6>>(reportTestCases, RandomsOp::OPCODE_DIV, 500), "posit<32,6>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<256, 4, std::uint64_t>>(reportTestCases, RandomsOp::OPCODE_MUL, 200), "posit<256,4>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyUnaryOperatorThroughRandoms<posit<64, 2>>(reportTestCases, RandomsOp::OPCODE_SQRT, 500), "posit<64,2>", "sqrt");
	// the double reference where it holds: the generic double path compiles for posit, which has no inf
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<32, 2>>(reportTestCases, RandomsOp::OPCODE_DIV, 500), "posit<32,2>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<posit<32, 5>>(reportTestCases, RandomsOp::OPCODE_MUL, 500), "posit<32,5>", "multiplication");
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
