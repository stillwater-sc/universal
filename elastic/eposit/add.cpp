// add.cpp: functional tests for addition on adaptive precision tapered floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//  SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>
#include <limits>

// minimum set of include files to reflect source code dependencies
#include <universal/number/eposit/eposit.hpp>
#include <universal/verification/test_suite.hpp>

// generate specific test case that you can trace with the trace conditions in mpreal.hpp
// for most bugs they are traceable with _trace_conversion and _trace_add
template<typename Ty>
void GenerateTestCase(Ty _a, Ty _b) {
	Ty ref;
	sw::universal::eposit a, b, aref, asum;
	a = _a;
	b = _b;
	asum = a + b;
	ref = _a + _b;
	aref = ref;

	auto precision = std::cout.precision();
	constexpr size_t ndigits = std::numeric_limits<Ty>::digits10;
	std::cout << std::setprecision(ndigits);
	std::cout << std::setw(ndigits) << _a << " + " << std::setw(ndigits) << _b << " = " << std::setw(ndigits) << ref << std::endl;
	std::cout << a << " + " << b << " = " << asum << " (reference: " << aref << ")   " ;
	std::cout << (aref == asum ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(precision);
}

// progressions
void Progressions(uint32_t digit) {
	using BlockType = uint32_t;
	sw::universal::eposit f;
	std::vector<BlockType> coef;

	constexpr size_t digitsInWord = 9;
	coef.clear();
	coef.push_back(digit);
	for (size_t i = 0; i < digitsInWord; ++i) {
		f.test(false, -1, coef);
		std::cout << "(+, exp = -1, coef = " << coef[0] << ") = " << f << '\n';
		coef[0] *= 10;
		coef[0] += digit;
	}
	coef.clear();
	coef.push_back(digit);
	for (size_t i = 0; i < digitsInWord; ++i) {
		f.test(false, 0, coef);
		std::cout << "(+, exp = 0, coef = " << coef[0] << ") = " << f << '\n';
		coef[0] *= 10;
		coef[0] += digit;
	}
	coef.clear();
	coef.push_back(digit);
	for (size_t i = 0; i < digitsInWord; ++i) {
		f.test(false, 1, coef);
		std::cout << "(+, exp = 1, coef = " << coef[0] << ") = " << f << '\n';
		coef[0] *= 10;
		coef[0] += digit;
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
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	std::string test_suite  = "adaptive posit addition";
	std::string test_tag    = "addition";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// generate individual testcases to hand trace/debug
	GenerateTestCase(INFINITY, INFINITY);

	eposit f;
	f = 0;
	std::cout << f << '\n';

	std::vector<uint32_t> coef;

	Progressions(1);
	Progressions(9);

	coef.clear();
	coef.push_back(0);
	f.test(false, 0, coef);
	for (int i = 0; i < 13; ++i) {
		coef[0] += 1;
		f.test(false, 0, coef);
		std::cout << "(+, exp = 0, coef = " << coef[0] << ") = " << f << '\n';
	}
	coef[0] = 999999999;
	f.test(false, 0, coef);
	std::cout << "(+, exp = 0, coef = " << coef[0] << ") = " << f << '\n';
	coef.push_back(0);
	for (int i = 0; i < 13; ++i) {
		coef[0] = 0;
		coef[1] += 1;
		f.test(false, 0, coef);
		std::cout << "(+, exp = 0, coef = " << coef[0] << ", " << coef[1] << ") = " << f << '\n';
		coef[0] = 999999999;
		f.test(false, 0, coef);
		std::cout << "(+, exp = 0, coef = " << coef[0] << ", " << coef[1] << ") = " << f << '\n';
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1

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
