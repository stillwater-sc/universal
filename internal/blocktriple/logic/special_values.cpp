// special_values.cpp: blocktriple comparisons with infinities, NaN and signed zeros
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The QA sweep of #2029 (tracking issue #2052) found that blocktriple's operator< treated any
// infinity as -inf (+inf < 1 was true, -inf < +inf false), and that operator== ignored NaN
// (nan == nan was true). The comparisons now follow IEEE-754: NaN is unordered, +0 == -0,
// and -inf < every finite value < +inf. The reference is the same comparison on double.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <cmath>
#include <limits>
#include <universal/internal/blocktriple/blocktriple.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

template<typename BlockTriple>
int VerifySpecialValueComparisons(bool reportTestCases) {
	const double inf = std::numeric_limits<double>::infinity();
	const double nan = std::numeric_limits<double>::quiet_NaN();
	const double values[] = { -inf, -3.5, -1.0, -0.0, 0.0, 0.25, 1.0, 3.5, inf, nan };
	int nrOfFailedTests = 0;
	for (double x : values) {
		for (double y : values) {
			BlockTriple a(x), b(y);
			const bool got[6]      = { a == b, a != b, a < b, a <= b, a > b, a >= b };
			const bool expected[6] = { x == y, x != y, x < y, x <= y, x > y, x >= y };
			const char* names[6]   = { "==", "!=", "<", "<=", ">", ">=" };
			for (int k = 0; k < 6; ++k) {
				if (got[k] != expected[k]) {
					++nrOfFailedTests;
					if (reportTestCases) std::cerr << "FAIL: " << x << ' ' << names[k] << ' ' << y << " = " << got[k] << " expected " << expected[k] << '\n';
				}
			}
		}
	}
	return nrOfFailedTests;
}

// an operator result is left unnormalized (a MUL product 2.25 is 10.01b at radix 2*fbits with
// scale 0), a native value is 1.001b with scale 1: the order must not depend on that layout
int VerifyLayoutIndependentOrder(bool reportTestCases) {
	using namespace sw::universal;
	int nrOfFailedTests = 0;
	auto check = [&](bool got, bool expected, const char* label) {
		if (got != expected) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: " << label << '\n';
		}
	};
	{
		using Mul = blocktriple<4, BlockTripleOperator::MUL, uint8_t>;
		Mul a(1.5), b(1.5), product;
		product.mul(a, b);                         // 2.25, with the overflow bit set
		Mul two(2.0), exact(2.25), three(3.0), minus(-2.25);
		check(product == exact, true, "MUL 1.5*1.5 == 2.25");
		check(product != exact, false, "MUL 1.5*1.5 != 2.25");
		check(product < two, false, "MUL 1.5*1.5 < 2.0");
		check(two < product, true, "MUL 2.0 < 1.5*1.5");
		check(product < three, true, "MUL 1.5*1.5 < 3.0");
		check(product >= exact, true, "MUL 1.5*1.5 >= 2.25");
		check(minus < product, true, "MUL -2.25 < 1.5*1.5");
		Mul c(-1.5), negproduct;
		negproduct.mul(c, b);                      // -2.25
		check(negproduct == minus, true, "MUL -1.5*1.5 == -2.25");
		check(negproduct < Mul(-2.0), true, "MUL -1.5*1.5 < -2.0");
		check(Mul(-3.0) < negproduct, true, "MUL -3.0 < -1.5*1.5");
	}
	{
		using Div = blocktriple<8, BlockTripleOperator::DIV, uint32_t>;
		Div a(3.0), b(1.5), quotient;
		quotient.div(a, b);                        // 2.0
		check(quotient == Div(2.0), true, "DIV 3/1.5 == 2.0");
		check(quotient < Div(2.0), false, "DIV 3/1.5 < 2.0");
		check(Div(1.75) < quotient, true, "DIV 1.75 < 3/1.5");
	}
	return nrOfFailedTests;
}

} // namespace

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
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "blocktriple special value comparisons";
	std::string test_tag    = "comparison";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	nrOfFailedTestCases += VerifySpecialValueComparisons<blocktriple<8, BlockTripleOperator::REP, uint8_t>>(true);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifySpecialValueComparisons<blocktriple<8, BlockTripleOperator::REP, uint8_t>>(reportTestCases), "blocktriple<8,REP,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySpecialValueComparisons<blocktriple<23, BlockTripleOperator::REP, uint32_t>>(reportTestCases), "blocktriple<23,REP,uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySpecialValueComparisons<blocktriple<10, BlockTripleOperator::ADD, uint16_t>>(reportTestCases), "blocktriple<10,ADD,uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySpecialValueComparisons<blocktriple<10, BlockTripleOperator::MUL, uint32_t>>(reportTestCases), "blocktriple<10,MUL,uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySpecialValueComparisons<blocktriple<52, BlockTripleOperator::REP, uint64_t>>(reportTestCases), "blocktriple<52,REP,uint64_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLayoutIndependentOrder(reportTestCases), "blocktriple MUL/DIV results", "layout-independent order");
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
	std::cerr << msg << std::endl;
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
