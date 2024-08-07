// printing.cpp: experiments with printing floating-point numbers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/support/decimal.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>

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

template<typename Real>
void Boundary(Real v, Real& nLow, Real& nHigh) {
	std::cout << sw::universal::to_binary(v) << " : " << v << '\n';
	nLow = v;
	nHigh = v;
}

template<typename Real>
void NarrowInterval(Real v, int& e, Real& nLeft, Real& nRight) {
	Real nLow, nHigh;
	Boundary<Real>(v, nLow, nHigh);
	e = floor(log10(nHigh));
	nLeft = nLow * pow(Real(10.0), Real(-e));
	nRight = nHigh * pow(Real(10.0), Real(-e));
}

template<typename Real>
void NextDigit(Real& nBar, int& d) {
	d = static_cast<int>(nBar);
	nBar = (nBar - d) * Real(10.0);
}

template<typename Real>
void Digits(Real& nLeft, Real& nRight) {
	std::vector<int> digits;
	int dLeft, dRight;
	do {
		NextDigit(nLeft, dLeft);
		NextDigit(nRight, dRight);
		digits.push_back(dRight);
		std::cout << "(" << nLeft << ", " << nRight << ") - (" << dLeft << ", " << dRight << ")\n";
	} while (dLeft != dRight);
}

template<typename Real>
void Convert(Real v, sw::universal::support::decimal& digits) {
	int e{ 0 };
	Real nLeft, nRight;
	NarrowInterval(v, e, nLeft, nRight);
	std::cout << "(" << e << ", " << nLeft << ", " << nRight << ")\n";
	Digits(nLeft, nRight);
}
int main()
try {
	using namespace sw::universal;

	std::string test_suite =  "Experiments in printing floating-point numbers ";
	std::string test_tag = "print";
	std::cout << test_suite << '\n';
	int nrOfFailedTestCases = 0;

#if MANUAL_TESTING

	using Real = float;

	Real a = 6.54321e5;
	support::decimal d;
	Convert(a, d);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; 
#else

#if REGRESSION_LEVEL_1

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
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
