// arithmetic.cpp: test suite runner of arithmetic operations on quad-double (qd) floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <algorithm>
#include <universal/number/qd/qd.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/test_suite_arithmetic.hpp>
#include <universal/verification/test_suite_randoms.hpp>
#include <universal/verification/cfloat_test_suite.hpp>

namespace sw {
	namespace universal {

		constexpr int LABELWIDTH = 15;
		constexpr int PRECISION = 25;

		void TestArithmeticOp(const sw::universal::qd& a, sw::universal::RandomsOp op, const sw::universal::qd& b) {
			using namespace sw::universal;
			bool binaryOp = true;
			qd c;
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
			case RandomsOp::OPCODE_IPA:  // In Place Aqd
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
			ReportValue(a, "a", LABELWIDTH, PRECISION);
			if (binaryOp) ReportValue(b, "b", LABELWIDTH, PRECISION);
			ReportValue(c, "c", LABELWIDTH, PRECISION);
		}



		void TestReciprocalIdentity(sw::universal::qd const& a) {

			qd oneOverA = reciprocal(a);

			qd one(1.0);
			qd error = one - a * oneOverA;
			ReportValue(a, "a", LABELWIDTH, PRECISION);
			ReportValue(oneOverA, "1/a", LABELWIDTH, PRECISION);
			ReportValue(error, "error", LABELWIDTH, PRECISION);
		}

		void TestDivisionalIdentity(sw::universal::qd const& a) {

			qd oneOverA = 1.0 / a;

			qd one(1.0);
			qd error = one - a * oneOverA;
			ReportValue(a, "a", LABELWIDTH, PRECISION);
			ReportValue(oneOverA, "1/a", LABELWIDTH, PRECISION);
			ReportValue(error, "error", LABELWIDTH, PRECISION);
		}

		void TestRandomReciprocalIdentities(int nrRandoms = 10) {
			std::default_random_engine generator;
			std::uniform_real_distribution< double > distr(-1048576.0, 1048576.0);

			for (int i = 0; i < nrRandoms; ++i) {
				qd a = distr(generator);
				TestReciprocalIdentity(a);
			}
		}

		void TestRandomDivisionalIdentities(int nrRandoms = 10) {
			std::default_random_engine generator;
			std::uniform_real_distribution< double > distr(-1048576.0, 1048576.0);

			for (int i = 0; i < nrRandoms; ++i) {
				qd a = distr(generator);
				TestDivisionalIdentity(a);
			}
		}

		void AdditionSubtraction() {
			double a0 = 1.0;
			double a1 = ulp(a0) / 2.0;
			double a2 = ulp(a1) / 2.0;
			double a3 = ulp(a2) / 2.0;

			ReportValue(a0, "a0 = 1.0");
			ReportValue(a1, "a1 = ulp(a0) / 2.0");
			ReportValue(a2, "a2 = ulp(a1) / 2.0");
			ReportValue(a3, "a3 = ulp(a2) / 2.0");
			renorm(a0, a1, a2, a3);  // double check this is a normalized quad-double configuration
			ReportValue(a0, "a0 = 1.0");
			ReportValue(a1, "a1 = ulp(a0) / 2.0");
			ReportValue(a2, "a2 = ulp(a1) / 2.0");
			ReportValue(a3, "a3 = ulp(a2) / 2.0");

			double b0 = 1.0;
			double b1 = ulp(b0) / 2.0;
			double b2 = ulp(b1) / 2.0;
			double b3 = ulp(b2) / 2.0;

			qd a(a0, a1, a2, a3);
			qd b(b0, b1, b2, b3);

			qd accurate_sum = a.accurate_addition(a, b);
			ReportValue(accurate_sum[0], "accurate_sum[0]");
			ReportValue(accurate_sum[1], "accurate_sum[1]");
			ReportValue(accurate_sum[2], "accurate_sum[2]");
			ReportValue(accurate_sum[3], "accurate_sum[3]");

			qd approximate_sum = a.approximate_addition(a, b);
			ReportValue(approximate_sum[0], "approximate_sum[0]");
			ReportValue(approximate_sum[1], "approximate_sum[1]");
			ReportValue(approximate_sum[2], "approximate_sum[2]");
			ReportValue(approximate_sum[3], "approximate_sum[3]");

			std::cout << to_quad(accurate_sum) << '\n';
			std::cout << to_binary(accurate_sum, true) << '\n';

			qd mina = -a;
			qd doublea = a + a;
			qd zero = a + mina;
			std::cout << to_quad(a) << '\n';
			std::cout << to_quad(mina) << '\n';
			std::cout << to_quad(doublea) << '\n';
			std::cout << to_quad(zero) << '\n';
			qd zero2 = a - a;
			std::cout << to_quad(zero2) << '\n';
			qd zero3 = -a + a;
			std::cout << to_quad(zero3) << '\n';
		}
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

	std::string test_suite         = "quad-double arithmetic validation";
	std::string test_tag           = "quad-double arithmetic";
	bool reportTestCases           = false;
	int nrOfFailedTestCases        = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	AdditionSubtraction();

	{
		qd a{ 1.0 }, b{ 2.0 }, c{};

		a *= 2.0;
		c = a * b;

		std::cout << to_binary(c) << '\n';
		std::cout << "product : " << c << '\n';
	}

	{
		double a0 = 1.0;
		double a1 = ulp(a0) / 2.0;
		double a2 = ulp(a1) / 2.0;
		double a3 = ulp(a2) / 2.0;

		qd a(a0, a1, a2, a3);
		std::cout << to_binary(a) << '\n';
		a *= 2;
		qd c{ a };
		std::cout << to_binary(c) << '\n';
		std::cout << "product : " << c << '\n';
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else  // !MANUAL_TESTING

#if REGRESSION_LEVEL_1

	constexpr unsigned nrOfRandoms = 1000;
	std::stringstream aqds;
	aqds << test_tag << " " << nrOfRandoms << " random aqds";
	std::string description = aqds.str();
	nrOfFailedTestCases += ReportTestResult(
		VerifyBinaryOperatorThroughRandoms<qd>(reportTestCases, RandomsOp::OPCODE_ADD, nrOfRandoms),
		description, 
		test_tag
	); 
	std::stringstream subs;
	subs << test_tag << " " << nrOfRandoms << " random subs";
	description = subs.str();
	nrOfFailedTestCases += ReportTestResult(
		VerifyBinaryOperatorThroughRandoms<qd>(reportTestCases, RandomsOp::OPCODE_SUB, nrOfRandoms),
		description, 
		test_tag
	); 
	std::stringstream muls;
	muls << test_tag << " " << nrOfRandoms << " random muls";
	description = muls.str();
	nrOfFailedTestCases += ReportTestResult(
		VerifyBinaryOperatorThroughRandoms<qd>(reportTestCases, RandomsOp::OPCODE_MUL, nrOfRandoms),
		description, 
		test_tag
	); 
	std::stringstream divs;
	divs << test_tag << " " << nrOfRandoms << " random divs";
	description = divs.str();
	nrOfFailedTestCases += ReportTestResult(
		VerifyBinaryOperatorThroughRandoms<qd>(reportTestCases, RandomsOp::OPCODE_DIV, nrOfRandoms),
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
