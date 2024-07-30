// arithmetic.cpp: test suite runner for arithmetic on doubledouble (dd) floating-point
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

constexpr unsigned labelWidth = 15;
constexpr unsigned precision = 25;

double TwoSumTrace(double a, double b, double& r) {
	double s = a + b;
	double bb = s - a;
	//r = (a - (s - bb)) + (b - bb);
	double sbb = s - bb;
	double asbb = a - sbb;
	double bbb = b - bb;
	r = asbb + bbb;
	return s;
}

void TraceTwoSum(double addend) {
	using namespace sw::universal;
	double a, b, s, r;
	a = 1.0;
	b = addend;
	s = two_sum(a, b, r);

	ReportValue(a, "a", labelWidth, precision);
	ReportValue(b, "b", labelWidth, precision);
	ReportValue(s, "s", labelWidth, precision);
	ReportValue(r, "r", labelWidth, precision);
}

void TraceTwoDiff(double differend) {
	using namespace sw::universal;
	double a, b, s, r;
	a = 1.0;
	b = differend;
	s = two_diff(a, b, r);

	ReportValue(a, "a", labelWidth, precision);
	ReportValue(b, "b", labelWidth, precision);
	ReportValue(s, "s", labelWidth, precision);
	ReportValue(r, "r", labelWidth, precision);
}

void TraceTwoProd(double base, double multiplicant) {
	using namespace sw::universal;
	double a, b, p, r;
	a = base;
	b = multiplicant;
	p = two_prod(a, b, r);

	ReportValue(a, "a", labelWidth, precision);
	ReportValue(b, "b", labelWidth, precision);
	ReportValue(p, "p", labelWidth, precision);
	ReportValue(r, "r", labelWidth, precision);
}

void TestArithmeticOp(const sw::universal::dd& a, sw::universal::RandomsOp op, const sw::universal::dd& b) {
	using namespace sw::universal;
	bool binaryOp = true;
	dd c;
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
	default:
		std::cerr << "unknown operator: test ignored\n";
		break;
	}
	ReportValue(a, "a", labelWidth, precision);
	if (binaryOp) ReportValue(b, "b", labelWidth, precision);
	ReportValue(c, "c", labelWidth, precision);
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

	std::string test_suite         = "doubledouble arithmetic validation";
	std::string test_tag           = "doubledouble arithmetic";
	bool reportTestCases           = false;
	int nrOfFailedTestCases        = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// doubledouble addition
	std::cout << "two sum\n";
	TraceTwoSum(ulp(pow(0.5, 10.0)));
	TraceTwoSum(-ulp(pow(0.5, 10.0)));

	// doubledouble subtraction
	std::cout << "\ntwo diff\n";
	TraceTwoDiff(ulp(pow(0.5, 10.0)));
	TraceTwoDiff(-ulp(pow(0.5, 10.0)));

	// doubledouble multiplication
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
	ReportValue(min_normal, "min-normal", labelWidth, precision);
	max_normal.setbits(0x7FEF'FFFF'FFFF'FFFFull);
	ReportValue(max_normal, "max-normal", labelWidth, precision);


	dd a, b;

	a = 1.0;
	b = ulp(pow(0.5, 10));
	TestArithmeticOp(a, RandomsOp::OPCODE_ADD, b);
	TestArithmeticOp(a, RandomsOp::OPCODE_SUB, b);
	TestArithmeticOp(a, RandomsOp::OPCODE_MUL, b);
	TestArithmeticOp(a, RandomsOp::OPCODE_DIV, b);

	ReportValue(1.0 / b.high(), "one over", labelWidth, precision);

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
