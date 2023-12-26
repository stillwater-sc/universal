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

// Standard posit with nbits = 32 have es = 2 exponent bits.

template<size_t nbits, size_t es>
void CheckAddition() {
	using namespace sw::universal;

	posit<nbits, es> pa, pb, pc;
	int fails = 0;
	for (int a = 0; a < 256; ++a) {
		pa.setbits(a);
		for (int b = 0; b < 256; ++b) {
			pb.setbits(b);
			pc = pa + pb;

			double da, db, dref;
			da = double(pa);
			db = double(pb);
			dref = da + db;

			posit<nbits, es> pref = dref;
			if (pref != pc) {
				std::cout << "FAIL: " << posit_format(pa) << " + " << posit_format(pb) << " produced " << posit_format(pc) << " instead of " << posit_format(pref) << '\n';
				++fails;
				break;
			}
		}
	}
	if (fails) {
		printf("addition        FAIL\n");
	}
	else {
		printf("addition        PASS\n");
	}
}

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

	// configure a posit<32,2>
	constexpr size_t nbits = 32;
	constexpr size_t es    =  2;

#if POSIT_FAST_POSIT_32_2
	std::string test_suite = "Fast specialization posit<32,2>";
#else
	std::string test_suite = "Standard posit<32,2>";
#endif

	std::string test_tag    = "arithmetic type tests";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	unsigned RND_TEST_CASES = 5000;

	using Scalar = posit<nbits, es>;
	Scalar p;
	std::string tag = type_tag(p);
	std::cout << dynamic_range(p) << "\n\n";

#if MANUAL_TESTING

	posit<nbits, es> a, b, c;
	a.setbits(0x0aa99eea);
	b.setbits(0xf97fcf40);
	std::cout << hex_format(a) << " + " << hex_format(b) << " = " << hex_format(a + b) << '\n';
	c = a + b;
	std::cout << a << " + " << b << " = " << c << '\n';
	std::cout << color_print(a) << " + " << color_print(b) << " = " << color_print(c) << '\n';
	c = a;
	c += b;
	std::cout << a << " + " << b << " = " << c << '\n';
	std::cout << color_print(a) << " + " << color_print(b) << " = " << color_print(c) << '\n';

#if 1
	std::string testVector[] = {
		"32.2x0a2f641dp",
		"32.2x06e8eb35p",
		"32.2xf97fcf40p",
		"32.2x03812f3fp",
		"32.2xf57e2aa8p",
		"32.2xf88b7e2fp",
		"32.2x04cd9168p",
		"32.2xfa843f6bp",
		"32.2x05a36e2ep",
		"32.2xf4e89c21p",
		"32.2x05080d4cp",
		"32.2x05a36e2ep"
	};

	std::string golden[] = {
		"32.2x0a2f641dp",
		"32.2x0aa99eeap",
		"32.2x0a4992bap",
		"32.2x0a51a5aep",
		"32.2xfa7e82b0p",
		"32.2xf82b1edbp",
		"32.2xf864d108p",
		"32.2xf805e0e3p",
		"32.2xf86ebc6ep",
		"32.2xf41ffa58p",
		"32.2xf440fc02p",
		"32.2xf47569c8p"
	};

	posit<nbits, es> accu1(0), accu2(0);
	int i = 0;
	for (auto v : testVector) {
		if (!parse(v, p)) {
			std::cerr << "unable to parse -" << v << "- into a posit value\n";
		}
		else {
			std::cout << hex_format(accu1) << " + " << hex_format(p) << '\n';
			accu1 = accu1 + p;
			accu2 += p;
			std::cout << hex_format(accu1) << " vs " << golden[i++] << " " << accu2 << '\n';
		}
	}
#endif

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
	RND_TEST_CASES = 1024 * 1024;
	std::cout << "Arithmetic tests " << RND_TEST_CASES << " randoms each\n";
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_ADD, RND_TEST_CASES), tag, "addition        (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_SUB, RND_TEST_CASES), tag, "subtraction     (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_MUL, RND_TEST_CASES), tag, "multiplication  (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_DIV, RND_TEST_CASES), tag, "division        (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_ADD, RND_TEST_CASES), tag, "+=              (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_SUB, RND_TEST_CASES), tag, "-=              (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_MUL, RND_TEST_CASES), tag, "*=              (native)  ");
	nrOfFailedTestCases += ReportTestResult( VerifyBinaryOperatorThroughRandoms<nbits, es>(reportTestCases, OPCODE_DIV, RND_TEST_CASES), tag, "/=              (native)  ");
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

