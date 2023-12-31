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
#define POSIT_FAST_POSIT_16_1 1
#define POSIT_FAST_POSIT_16_2 1
// second: enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>
#include <universal/verification/posit_test_randoms.hpp>
#include <universal/verification/posit_math_test_suite.hpp>

#ifdef EXHAUSTIVE
#include <universal/verification/posit_test_suite.hpp>
#endif

void TestWithValues(double av, double bv) {
	using namespace sw::universal;
	posit<16, 2> a, b, c;
	a = av;
	b = bv;
	c = a / b;
	ReportBinaryOperation(a, "/", b, c);
	double da = double(a);
	double db = double(b);
	double dc = da / db;
	//	ReportBinaryOperation(da, "/", db, dc);
	posit<16, 2> ref(dc);
	ReportBinaryOperation(a, "/", b, ref);
	if (c != ref) std::cout << "FAIL\n";
}

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

	unsigned RND_TEST_CASES = 10000;

	using Scalar = posit<nbits, es>;
	Scalar p;
	std::cout << dynamic_range(p) << "\n\n";
	std::string tag = type_tag(p);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_IPA, 100), tag, "+=             (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_IPS, 100), tag, "-=             (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_IPM, 100), tag, "*=             (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<nbits, es>(true, OPCODE_IPD, 100), tag, "/=             (native)  ");

	TestWithValues(1.1368683772161602974e-13, 8.5265128291212022305e-14);
	goto epilog;

	std::cout << "Exhaustive tests" << std::endl;
	nrOfFailedTestCases += ReportTestResult(VerifyDivision      <nbits, es>(reportTestCases), tag, "div            (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication<nbits, es>(reportTestCases), tag, "mul            (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction   <nbits, es>(reportTestCases), tag, "sub            (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition      <nbits, es>(reportTestCases), tag, "add            (native)  ");

	{
		posit<16, 2> a, b, c;
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

epilog:
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
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_ADD, RND_TEST_CASES), tag, "addition      ");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_SUB, RND_TEST_CASES), tag, "subtraction   ");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_MUL, RND_TEST_CASES), tag, "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_DIV, RND_TEST_CASES), tag, "division      ");

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
	nrOfFailedTestCases += ReportTestResult( VerifyIntegerConversion           <nbits, es>(reportTestCases), tag, "integer assign (native)  ");
	// FAIL =              0.25003 did not convert to             0.250061 instead it yielded                  0.25  raw 0b0.01.0.000000000000
	// FAIL = 0.99994 did not convert to             0.999878 instead it yielded                     1  raw 0b0.10.0.000000000000
	// posit<16, 1> float assign(native)   FAIL 2 failed test cases
	// nrOfFailedTestCases += ReportTestResult( VerifyConversion                  <nbits, es>(true), tag, "float assign   (native)  ");

	RND_TEST_CASES = 1024 * 1024;
	// arithmetic tests
	// State space is too large for exhaustive testing, so we use randoms to try to catch any silly regressions
	std::cout << "Arithmetic tests " << RND_TEST_CASES << " randoms each\n";
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_ADD, RND_TEST_CASES), tag, "addition       (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_IPA, RND_TEST_CASES), tag, "+=             (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_SUB, RND_TEST_CASES), tag, "subtraction    (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_IPS, RND_TEST_CASES), tag, "-=             (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_MUL, RND_TEST_CASES), tag, "multiplication (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_IPM, RND_TEST_CASES), tag, "*=             (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_DIV, RND_TEST_CASES), tag, "division       (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_IPD, RND_TEST_CASES), tag, "/=             (native)  ");
#endif

#if REGRESSION_LEVEL_4
	// elementary function tests
	std::cout << "Elementary function tests\n";
	nrOfFailedTestCases += ReportTestResult( VerifySqrt                        <nbits, es>(reportTestCases), tag, "sqrt           (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyExp                         <nbits, es>(reportTestCases), tag, "exp                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyExp2                        <nbits, es>(reportTestCases), tag, "exp2                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyLog                         <nbits, es>(reportTestCases), tag, "log                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyLog2                        <nbits, es>(reportTestCases), tag, "log2                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyLog10                       <nbits, es>(reportTestCases), tag, "log10                    ");
	nrOfFailedTestCases += ReportTestResult( VerifySine                        <nbits, es>(reportTestCases), tag, "sin                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyCosine                      <nbits, es>(reportTestCases), tag, "cos                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyTangent                     <nbits, es>(reportTestCases), tag, "tan                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyAsin                        <nbits, es>(reportTestCases), tag, "asin                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyAcos                        <nbits, es>(reportTestCases), tag, "acos                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyAtan                        <nbits, es>(reportTestCases), tag, "atan                     ");
	nrOfFailedTestCases += ReportTestResult( VerifySinh                        <nbits, es>(reportTestCases), tag, "sinh                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyCosh                        <nbits, es>(reportTestCases), tag, "cosh                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyTanh                        <nbits, es>(reportTestCases), tag, "tanh                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyAsinh                       <nbits, es>(reportTestCases), tag, "asinh                    ");
	nrOfFailedTestCases += ReportTestResult( VerifyAcosh                       <nbits, es>(reportTestCases), tag, "acosh                    ");
	nrOfFailedTestCases += ReportTestResult( VerifyAtanh                       <nbits, es>(reportTestCases), tag, "atanh                    ");

	nrOfFailedTestCases += ReportTestResult( VerifyPowerFunction               <nbits, es>(reportTestCases), tag, "pow                      ");
#endif


#ifdef EXHAUSTIVE
	// arithmetic tests
	std::cout << "Arithmetic tests\n";
	nrOfFailedTestCases += ReportTestResult(VerifyAddition         <nbits, es>(reportTestCases), tag, "add            (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction      <nbits, es>(reportTestCases), tag, "subtract       (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication   <nbits, es>(reportTestCases), tag, "multiply       (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision         <nbits, es>(reportTestCases), tag, "divide         (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyNegation         <nbits, es>(reportTestCases), tag, "negate         (native)  ");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation    <nbits, es>(reportTestCases), tag, "reciprocate    (native)  ");
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
