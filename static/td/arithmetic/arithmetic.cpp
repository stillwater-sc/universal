// arithmetic.cpp: test suite runner of arithmetic operations on triple-double (td) floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <algorithm>
#include <universal/number/td/td.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/test_suite_arithmetic.hpp>
#include <universal/verification/test_suite_randoms.hpp>
#include <universal/verification/cfloat_test_suite.hpp>

constexpr unsigned gLabelWidth = 15;
constexpr unsigned gPrecision = 25;

void TraceTwoSum(double addend) {
	using namespace sw::universal;
	double a, b, s, r;
	a = 1.0;
	b = addend;
	s = two_sum(a, b, r);

	ReportValue(a, "a", gLabelWidth, gPrecision);
	ReportValue(b, "b", gLabelWidth, gPrecision);
	ReportValue(s, "s", gLabelWidth, gPrecision);
	ReportValue(r, "r", gLabelWidth, gPrecision);
}

void TraceTwoDiff(double differend) {
	using namespace sw::universal;
	double a, b, s, r;
	a = 1.0;
	b = differend;
	s = two_diff(a, b, r);

	ReportValue(a, "a", gLabelWidth, gPrecision);
	ReportValue(b, "b", gLabelWidth, gPrecision);
	ReportValue(s, "s", gLabelWidth, gPrecision);
	ReportValue(r, "r", gLabelWidth, gPrecision);
}

void TraceTwoProd(double base, double multiplicant) {
	using namespace sw::universal;
	double a, b, p, r;
	a = base;
	b = multiplicant;
	p = two_prod(a, b, r);

	ReportValue(a, "a", gLabelWidth, gPrecision);
	ReportValue(b, "b", gLabelWidth, gPrecision);
	ReportValue(p, "p", gLabelWidth, gPrecision);
	ReportValue(r, "r", gLabelWidth, gPrecision);
}

void TestArithmeticOp(const sw::universal::td& a, sw::universal::RandomsOp op, const sw::universal::td& b) {
	using namespace sw::universal;
	bool binaryOp = true;
	td c;
	switch (op) {
	case RandomsOp::OPCODE_ADD:
		c = a + b;
		break;
	case RandomsOp::OPCODE_SUB:
		c = a - b;
		break;
	case RandomsOp::OPCODE_MUL:
		c = a * b;
		break;
	case RandomsOp::OPCODE_DIV:
		c = a / b;
		break;
	case RandomsOp::OPCODE_SQRT:
		c = sqrt(a);
		binaryOp = false;
		break;
	case RandomsOp::OPCODE_NOP:
	case RandomsOp::OPCODE_ASSIGN:
	case RandomsOp::OPCODE_IPA:  // In Place Add
	case RandomsOp::OPCODE_IPS:  // In Place Sub
	case RandomsOp::OPCODE_IPM:  // In Place Mul
	case RandomsOp::OPCODE_IPD:  // In Place Div
	case RandomsOp::OPCODE_EXP:
	case RandomsOp::OPCODE_EXP2:
	case RandomsOp::OPCODE_LOG:
	case RandomsOp::OPCODE_LOG2:
	case RandomsOp::OPCODE_LOG10:
	case RandomsOp::OPCODE_SIN:
	case RandomsOp::OPCODE_COS:
	case RandomsOp::OPCODE_TAN:
	case RandomsOp::OPCODE_ASIN:
	case RandomsOp::OPCODE_ACOS:
	case RandomsOp::OPCODE_ATAN:
	case RandomsOp::OPCODE_SINH:
	case RandomsOp::OPCODE_COSH:
	case RandomsOp::OPCODE_TANH:
	case RandomsOp::OPCODE_ASINH:
	case RandomsOp::OPCODE_ACOSH:
	case RandomsOp::OPCODE_ATANH:
	case RandomsOp::OPCODE_POW:
	case RandomsOp::OPCODE_HYPOT:
	case RandomsOp::OPCODE_RAN:
		std::cerr << "invalid operator: test ignored\n";
		break;
	default:
		std::cerr << "unknown operator: test ignored\n";
		break;
	}
	ReportValue(a, "a", gLabelWidth, gPrecision);
	if (binaryOp) ReportValue(b, "b", gLabelWidth, gPrecision);
	ReportValue(c, "c", gLabelWidth, gPrecision);
}


namespace sw {
	namespace universal {
		void TestReciprocalIdentity(sw::universal::td const& a) {

			td oneOverA = reciprocal(a);

			td one(1.0);
			td error = one - a * oneOverA;
			ReportValue(a, "a", gLabelWidth, gPrecision);
			ReportValue(oneOverA, "1/a", gLabelWidth, gPrecision);
			ReportValue(error, "error", gLabelWidth, gPrecision);
		}

		void TestDivisionalIdentity(sw::universal::td const& a) {

			td oneOverA = 1.0 / a;

			td one(1.0);
			td error = one - a * oneOverA;
			ReportValue(a, "a", gLabelWidth, gPrecision);
			ReportValue(oneOverA, "1/a", gLabelWidth, gPrecision);
			ReportValue(error, "error", gLabelWidth, gPrecision);
		}

		void TestRandomReciprocalIdentities(int nrRandoms = 10) {
			std::default_random_engine generator;
			std::uniform_real_distribution< double > distr(-1048576.0, 1048576.0);

			for (int i = 0; i < nrRandoms; ++i) {
				td a = distr(generator);
				TestReciprocalIdentity(a);
			}
		}

		void TestRandomDivisionalIdentities(int nrRandoms = 10) {
			std::default_random_engine generator;
			std::uniform_real_distribution< double > distr(-1048576.0, 1048576.0);

			for (int i = 0; i < nrRandoms; ++i) {
				td a = distr(generator);
				TestDivisionalIdentity(a);
			}
		}
	}
}


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

	std::string test_suite         = "triple-double arithmetic validation";
	std::string test_tag           = "triple-double arithmetic";
	bool reportTestCases           = false;
	int nrOfFailedTestCases        = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	auto defaultPrecision = std::cout.precision();

	// triple-double addition
	std::cout << "two sum\n";
	TraceTwoSum(ulp(pow(0.5, 10.0)));
	TraceTwoSum(-ulp(pow(0.5, 10.0)));

	// triple-double subtraction
	std::cout << "\ntwo diff\n";
	TraceTwoDiff(ulp(pow(0.5, 10.0)));
	TraceTwoDiff(-ulp(pow(0.5, 10.0)));

	// triple-double multiplication
	std::cout << "\ntwo prod\n";
	double ulp1 = ulp(pow(1.0, 1.0));
	TraceTwoProd(1.0, ulp1);
	TraceTwoProd(ulp1, ulp1);
	double base = 4.4501477170144023e-308; // smallest normal
	double multiplicant = 1.0 / static_cast<double>(1ull << 54);
	TraceTwoProd(base, multiplicant);

	base = 1.7976931348623157e+308;
	multiplicant = 1.7976931348623157e+308;
	TraceTwoProd(base, multiplicant);

	duble min_normal, max_normal;
	min_normal.setbits(0x001F'FFFF'FFFF'FFFFull);
	ReportValue(min_normal, "min-normal", gLabelWidth, gPrecision);
	max_normal.setbits(0x7FEF'FFFF'FFFF'FFFFull);
	ReportValue(max_normal, "max-normal", gLabelWidth, gPrecision);


	td a, b, c;

	a = 1.0;
	b = ulp(std::pow(0.5, 10));
	TestArithmeticOp(a, RandomsOp::OPCODE_ADD, b);
	TestArithmeticOp(a, RandomsOp::OPCODE_SUB, b);
	TestArithmeticOp(a, RandomsOp::OPCODE_MUL, b);
	TestArithmeticOp(a, RandomsOp::OPCODE_DIV, b);

	ReportValue(1.0 / b.high(), "one over", gLabelWidth, gPrecision);

	std::cout << "\n\n\n";
	TestReciprocalIdentity(td(1.0));
	TestReciprocalIdentity(td(0.5));
	TestReciprocalIdentity(td(10.0));

	std::cout << "\n\nfused multiply add\n";
	a = 1.0; b = 1.0, c = 0.0;
	c = fma(a, b, c);
	ReportValue(c, "fma(1.0, 1.0, 0.0)");
	a = 0.0; b = 1.0, c = 1.0;
	c = fma(a, b, c);
	ReportValue(c, "fma(0.0, 1.0, 1.0)");
	a = 1.0; b = 1.0, c = 1023.0;
	c = fma(a, b, c);
	ReportValue(c, "fma(1.0, 1.0, 1023.0)");

	std::cout << "\n\nquick product pairs\n";
	a = 0.5; b = 2.0;
	c = a * b;
	ReportValue(c, "0.5 * 2.0");
	a = 0.0625; b = 16.0;
	c = a * b;
	ReportValue(c, "0.0625 * 16.0");
	a = 10.0; b = 0.1;
	c = a * b;
	ReportValue(c, "10.0 * 0.1");

	std::cout << "\n\nquick divisional pairs\n";
	a = 1.0; b = 2.0;
	c = a / b;
	ReportValue(c, "1.0 / 2.0");
	a = 0.5; b = 2.0;
	c = a / b;
	ReportValue(c, "0.5 / 2.0");
	a = 2.0; b = 16.0;
	c = a / b;
	ReportValue(c, "2.0 / 16.0");
	a = 1.0; b = 2.0;
	c = a / b;
	ReportValue(c, "1.0 / 2.0");
	a = 10.0; b = 0.1;
	c = a / b;
	ReportValue(c, "10.0 / 0.1");

	std::cout << "Test reciprocal identities\n";
	TestRandomReciprocalIdentities(1);
	std::cout << "Test divisional identities\n";
	TestRandomDivisionalIdentities(1);

	std::cout << std::setprecision(defaultPrecision);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else  // !MANUAL_TESTING

#if REGRESSION_LEVEL_1

	constexpr unsigned nrOfRandoms = 1000;
	std::stringstream adds;
	adds << test_tag << " " << nrOfRandoms << " random adds";
	std::string description = adds.str();
	nrOfFailedTestCases += ReportTestResult(
		VerifyBinaryOperatorThroughRandoms<td>(reportTestCases, RandomsOp::OPCODE_ADD, nrOfRandoms),
		description,
		test_tag
	);
	std::stringstream subs;
	subs << test_tag << " " << nrOfRandoms << " random subs";
	description = subs.str();
	nrOfFailedTestCases += ReportTestResult(
		VerifyBinaryOperatorThroughRandoms<td>(reportTestCases, RandomsOp::OPCODE_SUB, nrOfRandoms),
		description,
		test_tag
	);
	std::stringstream muls;
	muls << test_tag << " " << nrOfRandoms << " random muls";
	description = muls.str();
	nrOfFailedTestCases += ReportTestResult(
		VerifyBinaryOperatorThroughRandoms<td>(reportTestCases, RandomsOp::OPCODE_MUL, nrOfRandoms),
		description,
		test_tag
	);
	std::stringstream divs;
	divs << test_tag << " " << nrOfRandoms << " random divs";
	description = divs.str();
	nrOfFailedTestCases += ReportTestResult(
		VerifyBinaryOperatorThroughRandoms<td>(reportTestCases, RandomsOp::OPCODE_DIV, nrOfRandoms),
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
