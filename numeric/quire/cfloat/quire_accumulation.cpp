//  quire_accumulations.cpp : computational path experiments with quires
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/quire/quire.hpp>
#include <universal/verification/test_reporters.hpp>

#include <iostream>
#include <string>

template<typename Scalar>
int TestQuireAccumulation() {
	using namespace sw::universal;

	using QuireType = sw::universal::quire<Scalar>;
	std::cout << "Testing quire accumulation with type: " << type_tag(QuireType{}) << '\n';

	int nrOfFailedTestCases = 0;

	Scalar    minpos(SpecificValue::minpos), maxpos(SpecificValue::maxpos);
	QuireType q;

	std::cout << type_tag(quire<Scalar>{}) << '\n';
	std::cout << quire_properties<Scalar>() << '\n';

	return nrOfFailedTestCases;  // return number of failed test cases
}

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
// #undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 1
#	define REGRESSION_LEVEL_3 0
#	define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite          = "cfloat<> quire accumulation";
	std::string test_tag            = "cfloat<> quire";
	bool        reportTestCases     = false;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#	if MANUAL_TESTING

	TestQuireAccumulation<cfloat<8, 3, uint8_t, true, false, false>>();
	TestQuireAccumulation<cfloat<16, 5, uint16_t, true, false, false>>();
	TestQuireAccumulation<cfloat<32, 8, uint32_t, true, false, false>>();

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore errors
#else

#	if REGRESSION_LEVEL_1

#	endif

#	if REGRESSION_LEVEL_2

#	endif

#	if REGRESSION_LEVEL_3

#	endif

#	if REGRESSION_LEVEL_4

#	endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
} catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
} catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
