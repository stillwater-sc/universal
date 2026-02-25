// sqrt_algorithm_reference.cpp: test suite runner for native floating-point square root algorithm
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
//#define ALGORITHM_VERBOSE_OUTPUT 1
//#define ALGORITHM_TRACE_SQRT 1
#include <universal/number/algorithm/newtons_iteration.hpp>
// #define CFLOAT_NATIVE_SQRT 1
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/test_suite_randoms.hpp>
#include <universal/verification/cfloat_test_suite.hpp>

template<typename Real = float>
void CheckNewtonsIterationAcrossNormals() {
	std::cout << "Iterate into max normals\n";
	// std::sqrt(negative) returns a -NaN(ind)
	auto precision = std::cout.precision();
	unsigned COLUMN_WIDTH = std::numeric_limits<Real>::max_digits10 + 3;
	std::cout << std::setprecision(std::numeric_limits<Real>::max_digits10);
	Real base = sqrt(std::numeric_limits<Real>::max());
	std::cout << "starting base : " << base << '\n';
	for (int i = 0; i < 4; i++) {
		Real square = base * base;
		Real root = sw::universal::newtons_iteration(square);
		std::cout << "square "     << std::setw(COLUMN_WIDTH) << square
			      << " root "      << std::setw(COLUMN_WIDTH) << root
			      << " reference " << std::setw(COLUMN_WIDTH) << base
			      << " diff "      << std::setw(COLUMN_WIDTH) << (std::abs(root - base)) << '\n';
		base *= 2.0f;
	}
	std::cout << std::setprecision(precision);
}

template<typename Real = float>
void CheckNewtonsIterationAcrossSubnormals() {
	std::cout << "Iterate into subnormals\n";
	// std::sqrt(negative) returns a -NaN(ind)
	auto precision = std::cout.precision();
	unsigned COLUMN_WIDTH = std::numeric_limits<Real>::max_digits10 + 3;
	std::cout << std::setprecision(std::numeric_limits<Real>::max_digits10);
	bool printHeader = true;
	Real base = sqrt(std::numeric_limits<Real>::min());
	std::cout << "starting base : " << base << '\n';
	for (int i = 0; i < 4; i++) {
		Real square = base * base;
		Real root = sw::universal::newtons_iteration(square);
		if (printHeader && !std::isnormal(square)) {
			std::cout << "Subnormal range\n";
			printHeader = false;
		}
		std::cout	<< "square "     << std::setw(COLUMN_WIDTH) << square 
					<< " root "	     << std::setw(COLUMN_WIDTH) << root
					<< " reference " << std::setw(COLUMN_WIDTH) << base
					<< " diff "      << std::setw(COLUMN_WIDTH) << (std::abs(root - base)) << '\n';
		base *= 0.5f;
	}
	std::cout << std::setprecision(precision);
}

template<typename Real>
void CheckNewtonsIteration(Real value) {
	auto precision = std::cout.precision();
	std::cout << std::setprecision(std::numeric_limits<Real>::max_digits10);

	Real reference = sqrt(value);
	Real root = sw::universal::newtons_iteration(value);

	bool printHeader = true;
	if (printHeader && !std::isnormal(value)) {
		std::cout << "Subnormal range\n";
		printHeader = false;
	}
	std::cout << "sqrt( " << value << ")\n";
	std::cout << "Standard Library   : " << reference << '\n';
	std::cout << "Newton's Iteration : " << root << '\n';
	std::cout << "Absolute Error     : " << std::abs(root - reference) << '\n';

	std::cout << std::setprecision(precision);
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

	std::string test_suite  = "float square root experiment";
	std::string test_tag    = "sqrt";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	CheckNewtonsIterationAcrossNormals();
	CheckNewtonsIterationAcrossSubnormals();
	CheckNewtonsIteration(2.0f);

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

#endif // REGRESSION_LEVEL_4

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Unexpected runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
