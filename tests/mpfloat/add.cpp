// add.cpp: functional tests for addition on multi-precison linear floating point
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>
#include <limits>

// minimum set of include files to reflect source code dependencies
#include <universal/mpfloat/mpfloat.hpp>
// test helpers, such as, ReportTestResults
#include "../utils/test_helpers.hpp"
//#include "mpfloat_test_helpers.hpp"

// generate specific test case that you can trace with the trace conditions in mpreal.hpp
// for most bugs they are traceable with _trace_conversion and _trace_add
template<typename Ty>
void GenerateTestCase(Ty a, Ty b) {
	Ty ref;
	sw::unum::mpfloat mpa, mpb, mpref, mpsum;
	mpa = a;
	mpb = b;
	ref = a + b;
	mpref = ref;
	mpsum = mpa + mpb;
	constexpr size_t ndigits = std::numeric_limits<Ty>::digits10;
	std::cout << std::setprecision(ndigits);
	std::cout << std::setw(ndigits) << a << " + " << std::setw(ndigits) << b << " = " << std::setw(ndigits) << ref << std::endl;
	std::cout << mpa << " + " << mpb << " = " << mpsum << " (reference: " << mpref << ")   " ;
	std::cout << (mpref == mpsum ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
}

// progressions
void Progressions(uint32_t digit) {
	using namespace std;
	using BlockType = uint32_t;
	sw::unum::mpfloat mpa;
	vector<BlockType> coef;

	constexpr size_t digitsInWord = 9;
	coef.clear();
	coef.push_back(digit);
	for (size_t i = 0; i < digitsInWord; ++i) {
		mpa.test(false, -1, coef);
		cout << "(+, exp = -1, coef = " << coef[0] << ") = " << mpa << endl;
		coef[0] *= 10;
		coef[0] += digit;
	}
	coef.clear();
	coef.push_back(digit);
	for (size_t i = 0; i < digitsInWord; ++i) {
		mpa.test(false, 0, coef);
		cout << "(+, exp = 0, coef = " << coef[0] << ") = " << mpa << endl;
		coef[0] *= 10;
		coef[0] += digit;
	}
	coef.clear();
	coef.push_back(digit);
	for (size_t i = 0; i < digitsInWord; ++i) {
		mpa.test(false, 1, coef);
		cout << "(+, exp = 1, coef = " << coef[0] << ") = " << mpa << endl;
		coef[0] *= 10;
		coef[0] += digit;
	}
}

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	int nrOfFailedTestCases = 0;

	std::string tag = "multi-precision float addition failed: ";

#if MANUAL_TESTING
//	bool bReportIndividualTestCases = false;

	// generate individual testcases to hand trace/debug
	GenerateTestCase(INFINITY, INFINITY);

	mpfloat mpa;
	mpa = 0;
	cout << mpa << endl;

	vector<uint32_t> coef;

	Progressions(1);
	Progressions(9);

	coef.clear();
	coef.push_back(0);
	mpa.test(false, 0, coef);
	for (int i = 0; i < 13; ++i) {
		coef[0] += 1;
		mpa.test(false, 0, coef);
		cout << "(+, exp = 0, coef = " << coef[0] << ") = " << mpa << endl;
	}
	coef[0] = 999999999;
	mpa.test(false, 0, coef);
	cout << "(+, exp = 0, coef = " << coef[0] << ") = " << mpa << endl;
	coef.push_back(0);
	for (int i = 0; i < 13; ++i) {
		coef[0] = 0;
		coef[1] += 1;
		mpa.test(false, 0, coef);
		cout << "(+, exp = 0, coef = " << coef[0] << ", " << coef[1] << ") = " << mpa << endl;
		coef[0] = 999999999;
		mpa.test(false, 0, coef);
		cout << "(+, exp = 0, coef = " << coef[0] << ", " << coef[1] << ") = " << mpa << endl;

	}

#else

	cout << "multi-precision float addition validation" << endl;


#if STRESS_TESTING

#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
