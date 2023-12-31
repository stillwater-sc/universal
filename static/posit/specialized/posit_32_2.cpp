// posit_32_2.cpp: test suite runner for fast specialized posit<32,2>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the posit template environment
// first: enable fast specialized posit<32,2>
//#define POSIT_FAST_SPECIALIZATION   // turns on all fast specializations
#define POSIT_FAST_POSIT_32_2 1
// second: enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>
#include <universal/number/posit/posit_parse.hpp>
#include <universal/verification/posit_test_suite.hpp>
#include <universal/verification/posit_test_randoms.hpp>
#include <universal/verification/test_case.hpp>

// Standard posit with nbits = 32 have es = 2 exponent bits.

namespace sw {
	namespace universal {
		void TestWithValue(double fa, double fb) {
			double fc;
			sw::universal::posit<32, 2> a, b, c;

			fc = fa + fb;
			a = fa;
			b = fb;
			c = a + b;
			ReportBinaryOperation(a, "+", b, c);
			std::cout << color_print(a) << " + " << color_print(b) << " = " << color_print(c) << '\n';
			c = fc;
			ReportBinaryOperation(a, "+", b, c);
			std::cout << color_print(a) << " + " << color_print(b) << " = " << color_print(c) << '\n';
		}

		void TestWithPattern(posit<32, 2>& a, posit<32, 2>& b) {
			posit<32, 2> c = a + b;
			ReportBinaryOperation(a, "+", b, c);
			std::cout << color_print(a) << " + " << color_print(b) << " = " << color_print(c) << '\n';
			double fa = double(a);
			double fb = double(b);
			double fc = fa + fb;
			ReportBinaryOperation(fa, "+", fb, fc);
			posit<32, 2> cref = fc;
			ReportBinaryOperation(a, "+", b, cref);
			std::cout << color_print(a) << " + " << color_print(b) << " = " << color_print(c) << '\n';
			if (c != cref) std::cout << "FAIL\n"; else std::cout << "PASS\n";
		}
} }

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

	// configure a posit<32,2>
	constexpr size_t nbits = 32;
	constexpr size_t es    =  2;

#if POSIT_FAST_POSIT_32_2
	std::string test_suite = "Fast specialization posit<32,2>";
#else
	std::string test_suite = "Standard posit<32,2>";
#endif

	std::string test_tag    = "number system test";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	unsigned RND_TEST_CASES = 65536;

	using Scalar = posit<nbits, es>;
	Scalar p;
	std::string tag = type_tag(p);
	std::cout << dynamic_range(p) << "\n\n";

#if MANUAL_TESTING

	/*
	-413900.75                + -0.23673234228044748306   != -6622473                  golden reference is -413901
	0b1.111110.10.10010100001100110011000 + 0b1.01.01.111001001101001111101101001 != 0b1.1111110.10.1001010000110100001001 golden reference is 0b1.111110.10.10010100001100110100000
	FAIL
	0.11507077468559145927    + 248.02450752258300781     != 3997.8502197265625        golden reference is 248.13957786560058594
	0b0.01.00.110101110101010001110011111 + 0b0.110.11.11110000000011001000110001 != 0b0.1110.11.1111001110111011001101010 golden reference is 0b0.110.11.11110000010001110111011011

	//double fa, fb;
	//fa = -413900.75;
	//fb = -0.23673234228044748306;
	//TestWithValue(fa, fb);

	{
		// FAIL
		// 0b0.10.00.000000000000000000000000001 + 0b0.0000000000000001.00.0000000000000 = 0b0.11111111111111110.01.000000000000
		// 1 + 8.67362e-19 = 2.30584e+18  should be 1+ULP
		//0b0000'0000'0000'0000'1000'0000'0000'0000
		posit<32, 2> a, b, c;
		a.setbits(0x4000'0001); // 1 + ULP
		b.setbits(0x00008000); // 8.67362e-19
		c = a + b;
		ReportBinaryOperation(a, "+", b, c);
	}


	{
		posit<32, 2> a, b;
		a.setbits(0x4000'0001); // 1 + ULP
		double useed = 16;
		for (int i = -15; i < 16; ++i) {
			b = pow(useed, double(i));
			TestWithPattern(a, b);
		}
	}
	 */

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

	RND_TEST_CASES = 5000;
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_ADD, RND_TEST_CASES), tag, "addition      ");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_SUB, RND_TEST_CASES), tag, "subtraction   ");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_MUL, RND_TEST_CASES), tag, "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_DIV, RND_TEST_CASES), tag, "division      ");

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

	RND_TEST_CASES = 5000;
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_ADD, RND_TEST_CASES), tag, "addition      ");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_SUB, RND_TEST_CASES), tag, "subtraction   ");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_MUL, RND_TEST_CASES), tag, "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_DIV, RND_TEST_CASES), tag, "division      ");
#endif

#if REGRESSION_LEVEL_2
	// logic tests
	std::cout << "Logic operator tests\n";
	nrOfFailedTestCases += ReportTestResult( VerifyPositLogicEqual             <nbits, es>(), tag, "    ==          (native) ");
	nrOfFailedTestCases += ReportTestResult( VerifyPositLogicNotEqual          <nbits, es>(), tag, "    !=          (native) ");
	nrOfFailedTestCases += ReportTestResult( VerifyPositLogicLessThan          <nbits, es>(), tag, "    <           (native) ");
	nrOfFailedTestCases += ReportTestResult( VerifyPositLogicLessOrEqualThan   <nbits, es>(), tag, "    <=          (native) ");
	nrOfFailedTestCases += ReportTestResult( VerifyPositLogicGreaterThan       <nbits, es>(), tag, "    >           (native) ");
	nrOfFailedTestCases += ReportTestResult( VerifyPositLogicGreaterOrEqualThan<nbits, es>(), tag, "    >=          (native) ");

	// conversion tests
	// internally this generators are clamped as the state space 2^33 is too big
	std::cout << "Assignment/conversion tests\n";
	nrOfFailedTestCases += ReportTestResult( VerifyIntegerConversion           <nbits, es>(reportTestCases), tag, "sint32 assign   (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyUintConversion              <nbits, es>(reportTestCases), tag, "uint32 assign   (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyConversion                  <nbits, es>(reportTestCases), tag, "float assign    (native)  ");
//	nrOfFailedTestCases += ReportTestResult( VerifyConversionThroughRandoms <nbits, es>(tag, true, 100), tag, "float assign   ");
#endif

#if REGRESSION_LEVEL_3
	// arithmetic tests
	std::cout << "Arithmetic tests " << RND_TEST_CASES << " randoms each\n";
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_ADD, RND_TEST_CASES), tag, "addition        (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_SUB, RND_TEST_CASES), tag, "subtraction     (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_MUL, RND_TEST_CASES), tag, "multiplication  (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_DIV, RND_TEST_CASES), tag, "division        (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_IPA, RND_TEST_CASES), tag, "+=              (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_IPS, RND_TEST_CASES), tag, "-=              (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_IPM, RND_TEST_CASES), tag, "*=              (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_IPD, RND_TEST_CASES), tag, "/=              (native)  ");
#endif

#if REGRESSION_LEVEL_4
	// elementary function tests
	std::cout << "Elementary function tests\n";
	p.minpos();
	double dminpos = double(p);
	nrOfFailedTestCases += ReportTestResult( VerifyUnaryOperatorThroughRandoms<Scalar>(reportTestCases, OPCODE_SQRT,  RND_TEST_CASES, dminpos), tag, "sqrt            (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyUnaryOperatorThroughRandoms<Scalar>(reportTestCases, OPCODE_EXP,   RND_TEST_CASES, dminpos), tag, "exp                       ");
	nrOfFailedTestCases += ReportTestResult( VerifyUnaryOperatorThroughRandoms<Scalar>(reportTestCases, OPCODE_EXP2,  RND_TEST_CASES, dminpos), tag, "exp2                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyUnaryOperatorThroughRandoms<Scalar>(reportTestCases, OPCODE_LOG,   RND_TEST_CASES, dminpos), tag, "log                       ");
	nrOfFailedTestCases += ReportTestResult( VerifyUnaryOperatorThroughRandoms<Scalar>(reportTestCases, OPCODE_LOG2,  RND_TEST_CASES, dminpos), tag, "log2                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyUnaryOperatorThroughRandoms<Scalar>(reportTestCases, OPCODE_LOG10, RND_TEST_CASES, dminpos), tag, "log10                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyUnaryOperatorThroughRandoms<Scalar>(reportTestCases, OPCODE_SIN,   RND_TEST_CASES, dminpos), tag, "sin                       ");
	nrOfFailedTestCases += ReportTestResult( VerifyUnaryOperatorThroughRandoms<Scalar>(reportTestCases, OPCODE_COS,   RND_TEST_CASES, dminpos), tag, "cos                       ");
	nrOfFailedTestCases += ReportTestResult( VerifyUnaryOperatorThroughRandoms<Scalar>(reportTestCases, OPCODE_TAN,   RND_TEST_CASES, dminpos), tag, "tan                       ");
	nrOfFailedTestCases += ReportTestResult( VerifyUnaryOperatorThroughRandoms<Scalar>(reportTestCases, OPCODE_ASIN,  RND_TEST_CASES, dminpos), tag, "asin                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyUnaryOperatorThroughRandoms<Scalar>(reportTestCases, OPCODE_ACOS,  RND_TEST_CASES, dminpos), tag, "acos                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyUnaryOperatorThroughRandoms<Scalar>(reportTestCases, OPCODE_ATAN,  RND_TEST_CASES, dminpos), tag, "atan                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyUnaryOperatorThroughRandoms<Scalar>(reportTestCases, OPCODE_SINH,  RND_TEST_CASES, dminpos), tag, "sinh                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyUnaryOperatorThroughRandoms<Scalar>(reportTestCases, OPCODE_COSH,  RND_TEST_CASES, dminpos), tag, "cosh                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyUnaryOperatorThroughRandoms<Scalar>(reportTestCases, OPCODE_TANH,  RND_TEST_CASES, dminpos), tag, "tanh                      ");
	nrOfFailedTestCases += ReportTestResult( VerifyUnaryOperatorThroughRandoms<Scalar>(reportTestCases, OPCODE_ASINH, RND_TEST_CASES, dminpos), tag, "asinh                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyUnaryOperatorThroughRandoms<Scalar>(reportTestCases, OPCODE_ACOSH, RND_TEST_CASES, dminpos), tag, "acosh                     ");
	nrOfFailedTestCases += ReportTestResult( VerifyUnaryOperatorThroughRandoms<Scalar>(reportTestCases, OPCODE_ATANH, RND_TEST_CASES, dminpos), tag, "atanh                     ");
	// elementary functions with two operands
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_POW, RND_TEST_CASES),   tag, "pow                       ");

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

