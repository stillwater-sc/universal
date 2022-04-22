// multifile.cpp: compilation test to check arithmetic type usage in application environments
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>
#include <limits>
#include <universal/verification/test_reporters.hpp>

// minimum set of include files to reflect source code dependencies
#include <universal/number/integer/integer.hpp>
#include <universal/number/adaptiveint/adaptiveint.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>


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

// forward references
using Integer = sw::universal::integer<8, uint8_t, sw::universal::IntegerNumberType::IntegerNumber>;
using Fixpnt  = sw::universal::fixpnt<8, 4, sw::universal::Saturating, uint8_t>;
using Cfloat  = sw::universal::half;
using Posit   = sw::universal::posit<8,2>;

Integer integerPolynomial(const std::vector<int>& coef, const Integer& x);
Fixpnt  fixpntPolynomial(const std::vector<int>& coef, const Fixpnt& x);
Cfloat  cfloatPolynomial(const std::vector<int>& coef, const Cfloat& x);
Posit   positPolynomial(const std::vector<int>& coef, const Posit& x);

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "multifile application environment";
	std::string test_tag = "multifile application";
	bool reportTestCases = true;
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';

#if MANUAL_TESTING

	// polynomial(x) = a + bx + cx^2 + dx^3 + ex^4 + fx^5;
	std::vector<int> coefficients;
	coefficients.push_back(1);
	coefficients.push_back(-1);
	coefficients.push_back(1);
	coefficients.push_back(-1);
	coefficients.push_back(1);
	coefficients.push_back(-1);

	float a(1.0f);
	
	std::cout << "integer      : " << integerPolynomial(coefficients, Integer(a)) << '\n';
	std::cout << "fixpnt       : " << fixpntPolynomial(coefficients, Fixpnt(a)) << '\n';
	std::cout << "cfloat       : " << cfloatPolynomial(coefficients, Cfloat(a)) << '\n';
	std::cout << "posit        : " << positPolynomial(coefficients, Posit(a)) << '\n';


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
