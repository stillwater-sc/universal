// multifile.cpp: compilation test to check arithmetic type usage in application environments
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>
#include <limits>

#include <universal/native/ieee754.hpp>
#include <universal/number/integer/integer.hpp>
#include <universal/number/einteger/einteger.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/number/dbns/dbns.hpp>
#include <universal/number/dd/dd.hpp>
#include <universal/number/qd/qd.hpp>

#include <universal/verification/test_reporters.hpp>

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

// forward references
using Integer      = sw::universal::integer<8, uint8_t, sw::universal::IntegerNumberType::IntegerNumber>;
using Fixpnt       = sw::universal::fixpnt<8, 4, sw::universal::Saturate, uint8_t>;
using Cfloat       = sw::universal::half;
using Posit        = sw::universal::posit<8,2>;
using Lns          = sw::universal::lns<8, 2>;
using Lns2b        = sw::universal::dbns<8, 6>;
using DoubleDouble = sw::universal::dd;
using QuadDouble   = sw::universal::qd;

Integer      integerPolynomial(const std::vector<int>& coef, const Integer& x);
Fixpnt       fixpntPolynomial(const std::vector<float>& coef, const Fixpnt& x);
Cfloat       cfloatPolynomial(const std::vector<float>& coef, const Cfloat& x);
Posit        positPolynomial(const std::vector<float>& coef, const Posit& x);
Lns          lnsPolynomial(const std::vector<float>& coef, const Lns& x);
Lns2b        dbnsPolynomial(const std::vector<float>& coef, const Lns2b& x);
DoubleDouble ddPolynomial(const std::vector<double>& coef, const DoubleDouble& x);
QuadDouble   qdPolynomial(const std::vector<double>& coef, const QuadDouble& x);

template<typename NumberType>
void EvaluatePolynomial(const std::vector<float>& coefficients, const NumberType& x) {
	std::vector<int> intCoefficients;
	for (auto e : coefficients) intCoefficients.push_back(static_cast<int>(e));
	std::vector<double> doubleCoefficients;
	for (auto e : coefficients) doubleCoefficients.push_back(double(e));

	std::cout << "x            : " << x << '\n';
	std::cout << "integer      : " << integerPolynomial(intCoefficients, Integer(x)) << '\n';
	std::cout << "fixpnt       : " << fixpntPolynomial(coefficients, Fixpnt(x)) << '\n';
	std::cout << "cfloat       : " << cfloatPolynomial(coefficients, Cfloat(x)) << '\n';
	std::cout << "posit        : " << positPolynomial(coefficients, Posit(x)) << '\n';
	std::cout << "lns          : " << lnsPolynomial(coefficients, Lns(x)) << '\n';
	std::cout << "dbns         : " << dbnsPolynomial(coefficients, Lns2b(x)) << '\n';
	std::cout << "double-double: " << ddPolynomial(doubleCoefficients, DoubleDouble(x)) << '\n';
	std::cout << "quad-double  : " << qdPolynomial(doubleCoefficients, QuadDouble(x)) << '\n';
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "multifile application environment";
	std::string test_tag    = "multifile application";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// polynomial(x) = a + bx + cx^2 + dx^3 + ex^4 + fx^5;
	std::vector<float> coefficients;
	coefficients.push_back(1);
	coefficients.push_back(-1);
	coefficients.push_back(1);
	coefficients.push_back(-1);
	coefficients.push_back(1);
	coefficients.push_back(-1);

	// Goal is to show the impact of rounding of different real number systems
	float a(1.0f);
	for (unsigned i = 0; i < 20; ++i) {
		EvaluatePolynomial(coefficients, a);
		a *= 0.5f;
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
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
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
