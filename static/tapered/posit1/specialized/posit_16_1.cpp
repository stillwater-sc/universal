// posit_16_1.cpp: test suite runner for specialized posit<16,1>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the posit template environment
// first: enable fast specialized posit<16,1>
//#define POSIT_FAST_SPECIALIZATION
#define POSIT_FAST_POSIT_16_1 1
// second: enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit1/posit1.hpp>
#include <universal/verification/posit_test_suite.hpp>
#include <universal/verification/posit_test_suite_mathlib.hpp>
#include <universal/verification/posit_test_suite_randoms.hpp>

// Standard posit with nbits = 16 have es = 1 exponent bit.

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

	// configure a posit<16,1>
	constexpr size_t nbits = 16;
	constexpr size_t es    =  1;

#if POSIT_FAST_POSIT_16_1
	std::string test_suite = "Fast specialization posit<16,1>";
#else
	std::string test_suite = "Standard posit<16,1>";
#endif

	std::string test_tag    = "arithmetic type tests";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	unsigned RND_TEST_CASES = 1024 * 1024;

	using TestType = posit<nbits, es>;
	TestType p;
	std::cout << dynamic_range(p) << "\n\n";
	std::string tag = type_tag(p);

#if MANUAL_TESTING

//	float fa, fb;
	posit<TestType> a, b, c;

	a.setnar(); b.setnar();
	testLogicOperators(a, b);
	a = +1; b = +1; --b;
	testLogicOperators(a, b);
	a = +1; b = +1; ++b;
	testLogicOperators(a, b);
	a = -1; b = -1; --b;
	testLogicOperators(a, b);
	a = -1; b = -1; ++b;
	testLogicOperators(a, b);

	a.setbits(0xfffd); b.setbits(0xfffe);
	testLogicOperators(a, b);

	uint16_t v1 = 0x7fff;
	uint16_t v2 = 0x8001;
	std::cout << v1 << " vs " << int16_t(v1) << '\n';
	std::cout << v2 << " vs " << int16_t(v2) << '\n';
	a.setbits(v1); b.setbits(v2);
	testLogicOperators(a, b);
	testLogicOperators(b, a);

	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_IPA, 100), tag, "+=             (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_IPS, 100), tag, "-=             (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_IPM, 100), tag, "*=             (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_IPD, 100), tag, "/=             (native)  ");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	// special cases
	std::cout << "Special case tests\n";
	std::string test = "Initialize to zero: ";
	p = 0;
	nrOfFailedTestCases += ReportCheck(tag, test, p.iszero());
	test = "Initialize to NAN";
	p = NAN;
	nrOfFailedTestCases += ReportCheck(tag, test, p.isnar());
	test = "Initialize to INFINITY";
	p = INFINITY;
	nrOfFailedTestCases += ReportCheck(tag, test, p.isnar());
	test = "sign is true";
	p = -1.0f;
	nrOfFailedTestCases += ReportCheck(tag, test, p.sign());
	test = "is negative";
	nrOfFailedTestCases += ReportCheck(tag, test, p.isneg());
	test = "sign is false";
	p = +1.0f;
	nrOfFailedTestCases += ReportCheck(tag, test, !p.sign());
	test = "is positive";
	nrOfFailedTestCases += ReportCheck(tag, test, p.ispos());

	RND_TEST_CASES = 1024;
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_ADD, RND_TEST_CASES), tag, "addition      ");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_SUB, RND_TEST_CASES), tag, "subtraction   ");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_MUL, RND_TEST_CASES), tag, "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_DIV, RND_TEST_CASES), tag, "division      ");

#endif

#if REGRESSION_LEVEL_2
	// logic tests
	std::cout << "Logic operator tests\n";
	nrOfFailedTestCases += ReportTestResult( VerifyLogicEqual             <TestType>(reportTestCases), tag, "    ==         (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyLogicNotEqual          <TestType>(reportTestCases), tag, "    !=         (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyLogicLessThan          <TestType>(reportTestCases), tag, "    <          (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyLogicLessOrEqualThan   <TestType>(reportTestCases), tag, "    <=         (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyLogicGreaterThan       <TestType>(reportTestCases), tag, "    >          (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyLogicGreaterOrEqualThan<TestType>(reportTestCases), tag, "    >=         (native)  ");
#endif

#if REGRESSION_LEVEL_3
	// conversion tests
	std::cout << "Assignment/conversion tests\n";
	nrOfFailedTestCases += ReportTestResult( VerifyIntegerConversion           <TestType>(reportTestCases), tag, "integer assign (native)  ");
	// FAIL =              0.25003 did not convert to             0.250061 instead it yielded                  0.25  raw 0b0.01.0.000000000000
	// FAIL = 0.99994 did not convert to             0.999878 instead it yielded                     1  raw 0b0.10.0.000000000000
	// posit<16, 1> float assign(native)   FAIL 2 failed test cases
	// nrOfFailedTestCases += ReportTestResult( VerifyConversion                  <TestType>(true), tag, "float assign   (native)  ");

	RND_TEST_CASES = 4 * 1024 * 1024;
	// arithmetic tests
	// State space is too large for exhaustive testing, so we use randoms to try to catch any silly regressions
	std::cout << "Arithmetic tests " << RND_TEST_CASES << " randoms each\n";
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_ADD, RND_TEST_CASES), tag, "addition       (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_IPA, RND_TEST_CASES), tag, "+=             (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_SUB, RND_TEST_CASES), tag, "subtraction    (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_IPS, RND_TEST_CASES), tag, "-=             (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_MUL, RND_TEST_CASES), tag, "multiplication (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_IPM, RND_TEST_CASES), tag, "*=             (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_DIV, RND_TEST_CASES), tag, "division       (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_IPD, RND_TEST_CASES), tag, "/=             (native)  ");
#endif

#if REGRESSION_LEVEL_4
	// elementary function tests
	std::cout << "Elementary function tests\n";
	nrOfFailedTestCases += ReportTestResult( VerifySqrt                        <TestType>(reportTestCases), tag, "sqrt           (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyExp                         <TestType>(reportTestCases), tag, "exp                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyExp2                        <TestType>(reportTestCases), tag, "exp2                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyLog                         <TestType>(reportTestCases), tag, "log                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyLog2                        <TestType>(reportTestCases), tag, "log2                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyLog10                       <TestType>(reportTestCases), tag, "log10                    ");
	nrOfFailedTestCases += ReportTestResult( VerifySine                        <TestType>(reportTestCases), tag, "sin                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyCosine                      <TestType>(reportTestCases), tag, "cos                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyTangent                     <TestType>(reportTestCases), tag, "tan                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyAsin                        <TestType>(reportTestCases), tag, "asin                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyAcos                        <TestType>(reportTestCases), tag, "acos                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyAtan                        <TestType>(reportTestCases), tag, "atan                     ");
	nrOfFailedTestCases += ReportTestResult( VerifySinh                        <TestType>(reportTestCases), tag, "sinh                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyCosh                        <TestType>(reportTestCases), tag, "cosh                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyTanh                        <TestType>(reportTestCases), tag, "tanh                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyAsinh                       <TestType>(reportTestCases), tag, "asinh                    ");
	nrOfFailedTestCases += ReportTestResult( VerifyAcosh                       <TestType>(reportTestCases), tag, "acosh                    ");
	nrOfFailedTestCases += ReportTestResult( VerifyAtanh                       <TestType>(reportTestCases), tag, "atanh                    ");

	nrOfFailedTestCases += ReportTestResult( VerifyPowerFunction               <TestType>(reportTestCases), tag, "pow                      ");
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
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
