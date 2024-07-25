// add.cpp: test runner for addition on adaptive precision binary floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//  SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>
#include <limits>

// minimum set of include files to reflect source code dependencies
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult

// generate specific test case that you can trace with the trace conditions in mpreal.hpp
// for most bugs they are traceable with _trace_conversion and _trace_add
template<typename Ty>
void GenerateTestCase(Ty _a, Ty _b) {
	Ty ref;
	sw::universal::efloat a, b, aref, asum;
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
	sw::universal::efloat f;
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

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;

	std::string tag = "adaptive precision linear float addition failed: ";

#if MANUAL_TESTING
//	bool bReportIndividualTestCases = false;

	// generate individual testcases to hand trace/debug
	GenerateTestCase(INFINITY, INFINITY);

	efloat f;
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

#else

	cout << "adaptive precision linear float addition validation" << endl;


#if STRESS_TESTING

#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
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
