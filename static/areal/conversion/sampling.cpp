// sampling.cpp: sampling comparison between different areal configurations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// Configure the areal template environment
// first: enable general or specialized configurations
#define AREAL_FAST_SPECIALIZATION
// second: enable/disable arithmetic exceptions
#define AREAL_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/areal/areal.hpp>
#include <universal/verification/test_suite_arithmetic.hpp>


template<size_t nbits, size_t es>
void GenerateArealComparisonTable(const std::string& tag) {
	using namespace sw::universal;
	constexpr size_t NR_VALUES = (size_t(1) << nbits);

	areal<nbits, es> a;
	std::string typeOfa = typeid(a).name();
	areal<nbits+1, es> b;
	std::string typeOfb = typeid(b).name();
	size_t columnWidth = 6 + std::max(typeOfa.length(), typeOfb.length());
	std::cout << tag << '\n' << std::setw(columnWidth) << typeOfb << "  |  " << std::setw(columnWidth) << typeOfa << '\n';

	// enumerate and compare the sampling of the real value line of the two types
	for (size_t i = 0; i < NR_VALUES; ++i) {
		a.setbits(i);
		b.setbits(2*i);
		std::cout << std::setw(columnWidth - 11ull) << pretty_print(b) << ' ' << std::setw(10) << b << "  |  " << pretty_print(a) << ' ' << std::setw(10) << a << '\n';

		b.setbits(2 * i + 1);
		std::cout << std::setw(columnWidth - 11ull) << pretty_print(b) << ' ' << std::setw(10) << b << "  |  " << '\n';
	}
}

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
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "areal value sampling verification";
	std::string test_tag    = "sampling";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// generate individual testcases to hand trace/debug

	// manual test
	GenerateArealComparisonTable<5, 2>(tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
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
catch (const std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
