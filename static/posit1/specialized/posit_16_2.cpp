// posit_16_2.cpp: test suite runner for specialized posit<16,2>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the posit template environment
// first: enable fast specialized posit<16,2>
//#define POSIT_FAST_SPECIALIZATION
#define POSIT_FAST_POSIT_16_2 1
// second: enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit1/posit1.hpp>
#include <universal/verification/posit_test_suite.hpp>
#include <universal/verification/posit_test_suite_mathlib.hpp>
#include <universal/verification/posit_test_suite_randoms.hpp>

// Standard posit with nbits = 16 have es = 2 exponent bit.

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
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

	// configure a posit<16,2>
	constexpr unsigned nbits = NBITS_IS_16;
	constexpr unsigned es    = ES_IS_2;

#if POSIT_FAST_POSIT_16_2
	std::string test_suite = "Fast specialization posit<16,2>";
#else
	std::string test_suite = "Standard posit<16,2>";
#endif

	std::string test_tag    = "arithmetic type tests";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using TestType = posit<nbits, es>;
	using EnvelopeType = posit<nbits + 1, es>;
	TestType p;
	std::string typeTag = type_tag(p);
	std::cout << dynamic_range(p) << "\n\n";


#if MANUAL_TESTING

	using TestType = posit<16, 2>;
	/*
		1.3877787807814456755e-17 /= -0.004917144775390625     != -8.8817841970012523234e-16 golden reference is -3.5527136788005009294e-15
		0b0.000000000000001..     /= 0b1.001.00.0100001001     != 0b1.00000000000001.1.     golden reference is 0b1.0000000000001.00.
	*/
	// TestArithmeticBinaryOperation<TestType>(1.3877787807814456755e-17, -0.004917144775390625, TestCaseOperator::DIV);

	p = 0.06251519627f;
	ReportValue(p);

	nrOfFailedTestCases += ReportTestResult(VerifyConversion<TestType, EnvelopeType, float>(true), typeTag, "float conversion   (native)  "); return 0;
	nrOfFailedTestCases += ReportTestResult(VerifyConversion<TestType, EnvelopeType, double>(reportTestCases), typeTag, "double conversion  (native)  ");

	{
		posit<16, 2> a, b;
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
	}

	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_IPA, 100), typeTag, "+=             (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_IPS, 100), typeTag, "-=             (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_IPM, 100), typeTag, "*=             (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_IPD, 100), typeTag, "/=             (native)  ");

	goto epilog;  // skip the exhaustive tests

	std::cout << "Exhaustive tests" << std::endl;
	nrOfFailedTestCases += ReportTestResult(VerifyDivision      <TestType>(reportTestCases), typeTag, "div            (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<TestType>(reportTestCases), typeTag, "mul            (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction   <TestType>(reportTestCases), typeTag, "sub            (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition      <TestType>(reportTestCases), typeTag, "add            (native)  ");

epilog:
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	// special cases
	std::cout << "Special case tests\n";
	std::string test = "Initialize to zero: ";
	p = 0;
	nrOfFailedTestCases += ReportCheck(typeTag, test, p.iszero());
	test = "Initialize to NAN";
	p = NAN;
	nrOfFailedTestCases += ReportCheck(typeTag, test, p.isnar());
	test = "Initialize to INFINITY";
	p = INFINITY;
	nrOfFailedTestCases += ReportCheck(typeTag, test, p.isnar());
	test = "sign is true";
	p = -1.0f;
	nrOfFailedTestCases += ReportCheck(typeTag, test, p.sign());
	test = "is negative";
	nrOfFailedTestCases += ReportCheck(typeTag, test, p.isneg());
	test = "sign is false";
	p = +1.0f;
	nrOfFailedTestCases += ReportCheck(typeTag, test, !p.sign());
	test = "is positive";
	nrOfFailedTestCases += ReportCheck(typeTag, test, p.ispos());

	// conversion tests
	std::cout << "Assignment/conversion tests\n";
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerConversion           <TestType>(reportTestCases), typeTag, "integer assign (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion                  <TestType, float>(reportTestCases), typeTag, "float assign   (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyConversion                  <TestType, double>(reportTestCases), typeTag, "double assign  (native)  ");

	{
		unsigned RND_TEST_CASES = 10000;
		std::cout << "Arithmetic tests " << RND_TEST_CASES << " randoms each\n";
		nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_ADD, RND_TEST_CASES), typeTag, "addition      ");
		nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_SUB, RND_TEST_CASES), typeTag, "subtraction   ");
		nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_MUL, RND_TEST_CASES), typeTag, "multiplication");
		nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_DIV, RND_TEST_CASES), typeTag, "division      ");
	}
#endif

#if REGRESSION_LEVEL_2
	// logic tests
	std::cout << "Logic operator tests\n";
	nrOfFailedTestCases += ReportTestResult( VerifyLogicEqual             <TestType>(reportTestCases), typeTag, "    ==         (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyLogicNotEqual          <TestType>(reportTestCases), typeTag, "    !=         (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyLogicLessThan          <TestType>(reportTestCases), typeTag, "    <          (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyLogicLessOrEqualThan   <TestType>(reportTestCases), typeTag, "    <=         (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyLogicGreaterThan       <TestType>(reportTestCases), typeTag, "    >          (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyLogicGreaterOrEqualThan<TestType>(reportTestCases), typeTag, "    >=         (native)  ");
#endif

#if REGRESSION_LEVEL_3
	// arithmetic tests
	// State space is too large for exhaustive testing, so we use randoms to try to catch any silly regressions
	{
		unsigned RND_TEST_CASES = 1024 * 1024;
		std::cout << "Arithmetic tests " << RND_TEST_CASES << " randoms each\n";
		nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_ADD, RND_TEST_CASES), typeTag, "addition       (native)  ");
		nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_IPA, RND_TEST_CASES), typeTag, "+=             (native)  ");
		nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_SUB, RND_TEST_CASES), typeTag, "subtraction    (native)  ");
		nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_IPS, RND_TEST_CASES), typeTag, "-=             (native)  ");
		nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_MUL, RND_TEST_CASES), typeTag, "multiplication (native)  ");
		nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_IPM, RND_TEST_CASES), typeTag, "*=             (native)  ");
		nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_DIV, RND_TEST_CASES), typeTag, "division       (native)  ");
		nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<TestType>(reportTestCases, OPCODE_IPD, RND_TEST_CASES), typeTag, "/=             (native)  ");
	}
#endif

#if REGRESSION_LEVEL_4
	// elementary function tests
	std::cout << "Elementary function tests\n";
	nrOfFailedTestCases += ReportTestResult( VerifySqrt                        <TestType>(reportTestCases), typeTag, "sqrt           (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyExp                         <TestType>(reportTestCases), typeTag, "exp                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyExp2                        <TestType>(reportTestCases), typeTag, "exp2                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyLog                         <TestType>(reportTestCases), typeTag, "log                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyLog2                        <TestType>(reportTestCases), typeTag, "log2                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyLog10                       <TestType>(reportTestCases), typeTag, "log10                    ");
	nrOfFailedTestCases += ReportTestResult( VerifySine                        <TestType>(reportTestCases), typeTag, "sin                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyCosine                      <TestType>(reportTestCases), typeTag, "cos                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyTangent                     <TestType>(reportTestCases), typeTag, "tan                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyAsin                        <TestType>(reportTestCases), typeTag, "asin                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyAcos                        <TestType>(reportTestCases), typeTag, "acos                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyAtan                        <TestType>(reportTestCases), typeTag, "atan                     ");
	nrOfFailedTestCases += ReportTestResult( VerifySinh                        <TestType>(reportTestCases), typeTag, "sinh                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyCosh                        <TestType>(reportTestCases), typeTag, "cosh                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyTanh                        <TestType>(reportTestCases), typeTag, "tanh                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyAsinh                       <TestType>(reportTestCases), typeTag, "asinh                    ");
	nrOfFailedTestCases += ReportTestResult( VerifyAcosh                       <TestType>(reportTestCases), typeTag, "acosh                    ");
	nrOfFailedTestCases += ReportTestResult( VerifyAtanh                       <TestType>(reportTestCases), typeTag, "atanh                    ");

	nrOfFailedTestCases += ReportTestResult( VerifyPowerFunction               <TestType>(reportTestCases), typeTag, "pow                      ");
#endif


#ifdef EXHAUSTIVE
	// arithmetic tests
	std::cout << "Arithmetic tests\n";
	nrOfFailedTestCases += ReportTestResult(VerifyAddition         <TestType>(reportTestCases), typeTag, "add            (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction      <TestType>(reportTestCases), typeTag, "subtract       (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication   <TestType>(reportTestCases), typeTag, "multiply       (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision         <TestType>(reportTestCases), typeTag, "divide         (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyNegation         <TestType>(reportTestCases), typeTag, "negate         (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation    <TestType>(reportTestCases), typeTag, "reciprocate    (native)  ");
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
