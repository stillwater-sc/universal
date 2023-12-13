// posit_16_2.cpp: test suite runner for specialized posit<16,2>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
//
// Configure the posit template environment
// first: enable fast specialized posit<16,2>
//#define POSIT_FAST_SPECIALIZATION
#define POSIT_FAST_POSIT_16_2 1
// second: enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>
#include <universal/verification/posit_test_suite.hpp>
#include <universal/verification/posit_test_randoms.hpp>
#include <universal/verification/posit_math_test_suite.hpp>

// Standard posit with nbits = 16 have es = 2 exponent bit.

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

	// configure a posit<16,2>
	constexpr unsigned nbits = NBITS_IS_16;
	constexpr unsigned es    = ES_IS_2;

	int nrOfFailedTestCases = 0;
	bool bReportIndividualTestCases = false;
	size_t RND_TEST_CASES = 10000;

#if POSIT_FAST_POSIT_16_2
	std::cout << "Fast specialization posit<16,2> configuration tests\n";
#else
	std::cout << "Standard posit<16,2> configuration tests\n";
#endif

	using Scalar = posit<nbits, es>;
	Scalar p;
	std::cout << dynamic_range(p) << "\n\n";
	std::string tag = type_tag(p);

#if MANUAL_TESTING
	float fa, fb;
	posit<nbits, es> a, b, c;
	fa = 2;	fb = 1;
	a = fa; b = fb; c = a; c += b;
	std::cout << to_binary(a) << " + " << to_binary(b) << " = " << to_binary(a + b) << "(" << (fa + fb) << ") " << to_binary(c) << "(" << c << ")" << '\n';
	fa = 2;	fb = -1;
	a = fa; b = fb; c = a; c += b;
	std::cout << to_binary(a) << " + " << to_binary(b) << " = " << to_binary(a + b) << "(" << (fa + fb) << ") " << to_binary(c) << "(" << c << ")" << '\n';
	fa = -2;	fb = 1;
	a = fa; b = fb; c = a; c += b;
	std::cout << to_binary(a) << " + " << to_binary(b) << " = " << to_binary(a + b) << "(" << (fa + fb) << ") " << to_binary(c) << "(" << c << ")" << '\n';
	fa = -2;	fb = -1;
	a = fa; b = fb; c = a; c += b;
	std::cout << to_binary(a) << " + " << to_binary(b) << " = " << to_binary(a + b) << "(" << (fa + fb) << ") " << to_binary(c) << "(" << c << ")" << '\n';

	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_IPA, 100), tag, "+=             (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_IPS, 100), tag, "-=             (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_IPM, 100), tag, "*=             (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_IPD, 100), tag, "/=             (native)  ");


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

	std::cout << nrOfFailedTestCases << " number of failures\n";

	nrOfFailedTestCases = 0;  // ignore failures in manual testing
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
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_ADD, RND_TEST_CASES), tag, "addition      ");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_SUB, RND_TEST_CASES), tag, "subtraction   ");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_MUL, RND_TEST_CASES), tag, "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_DIV, RND_TEST_CASES), tag, "division      ");

#endif

#if REGRESSION_LEVEL_2
	// logic tests
	std::cout << "Logic operator tests\n";
	nrOfFailedTestCases += ReportTestResult( VerifyPositLogicEqual             <nbits, es>(), tag, "    ==         (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyPositLogicNotEqual          <nbits, es>(), tag, "    !=         (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyPositLogicLessThan          <nbits, es>(), tag, "    <          (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyPositLogicLessOrEqualThan   <nbits, es>(), tag, "    <=         (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyPositLogicGreaterThan       <nbits, es>(), tag, "    >          (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyPositLogicGreaterOrEqualThan<nbits, es>(), tag, "    >=         (native)  ");
#endif

#if REGRESSION_LEVEL_3
	// conversion tests
	std::cout << "Assignment/conversion tests\n";
	nrOfFailedTestCases += ReportTestResult( VerifyIntegerConversion           <nbits, es>(bReportIndividualTestCases), tag, "integer assign (native)  ");
	// FAIL =              0.25003 did not convert to             0.250061 instead it yielded                  0.25  raw 0b0.01.0.000000000000
	// FAIL = 0.99994 did not convert to             0.999878 instead it yielded                     1  raw 0b0.10.0.000000000000
	// posit<16, 1> float assign(native)   FAIL 2 failed test cases
	// nrOfFailedTestCases += ReportTestResult( VerifyConversion                  <nbits, es>(true), tag, "float assign   (native)  ");

	RND_TEST_CASES = 1024 * 1024;
	// arithmetic tests
	// State space is too large for exhaustive testing, so we use randoms to try to catch any silly regressions
	std::cout << "Arithmetic tests " << RND_TEST_CASES << " randoms each\n";
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_ADD, RND_TEST_CASES), tag, "addition       (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_IPA, RND_TEST_CASES), tag, "+=             (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_SUB, RND_TEST_CASES), tag, "subtraction    (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_IPS, RND_TEST_CASES), tag, "-=             (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_MUL, RND_TEST_CASES), tag, "multiplication (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_IPM, RND_TEST_CASES), tag, "*=             (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_DIV, RND_TEST_CASES), tag, "division       (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(bReportIndividualTestCases, OPCODE_IPD, RND_TEST_CASES), tag, "/=             (native)  ");
#endif

#if REGRESSION_LEVEL_4
	// elementary function tests
	std::cout << "Elementary function tests\n";
	nrOfFailedTestCases += ReportTestResult( VerifySqrt                        <nbits, es>(bReportIndividualTestCases), tag, "sqrt           (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyExp                         <nbits, es>(bReportIndividualTestCases), tag, "exp                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyExp2                        <nbits, es>(bReportIndividualTestCases), tag, "exp2                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyLog                         <nbits, es>(bReportIndividualTestCases), tag, "log                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyLog2                        <nbits, es>(bReportIndividualTestCases), tag, "log2                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyLog10                       <nbits, es>(bReportIndividualTestCases), tag, "log10                    ");
	nrOfFailedTestCases += ReportTestResult( VerifySine                        <nbits, es>(bReportIndividualTestCases), tag, "sin                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyCosine                      <nbits, es>(bReportIndividualTestCases), tag, "cos                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyTangent                     <nbits, es>(bReportIndividualTestCases), tag, "tan                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyAsin                        <nbits, es>(bReportIndividualTestCases), tag, "asin                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyAcos                        <nbits, es>(bReportIndividualTestCases), tag, "acos                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyAtan                        <nbits, es>(bReportIndividualTestCases), tag, "atan                     ");
	nrOfFailedTestCases += ReportTestResult( VerifySinh                        <nbits, es>(bReportIndividualTestCases), tag, "sinh                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyCosh                        <nbits, es>(bReportIndividualTestCases), tag, "cosh                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyTanh                        <nbits, es>(bReportIndividualTestCases), tag, "tanh                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyAsinh                       <nbits, es>(bReportIndividualTestCases), tag, "asinh                    ");
	nrOfFailedTestCases += ReportTestResult( VerifyAcosh                       <nbits, es>(bReportIndividualTestCases), tag, "acosh                    ");
	nrOfFailedTestCases += ReportTestResult( VerifyAtanh                       <nbits, es>(bReportIndividualTestCases), tag, "atanh                    ");

	nrOfFailedTestCases += ReportTestResult( VerifyPowerFunction               <nbits, es>(bReportIndividualTestCases), tag, "pow                      ");
#endif

#ifdef EXHAUSTIVE
	// arithmetic tests
	std::cout << "Arithmetic tests\n";
	nrOfFailedTestCases += ReportTestResult(VerifyAddition         <nbits, es>(bReportIndividualTestCases), tag, "add            (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction      <nbits, es>(bReportIndividualTestCases), tag, "subtract       (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication   <nbits, es>(bReportIndividualTestCases), tag, "multiply       (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision         <nbits, es>(bReportIndividualTestCases), tag, "divide         (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyNegation         <nbits, es>(bReportIndividualTestCases), tag, "negate         (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation    <nbits, es>(bReportIndividualTestCases), tag, "reciprocate    (native)  ");
#endif

#endif // MANUAL_TESTING
	std::cout << std::flush;

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
