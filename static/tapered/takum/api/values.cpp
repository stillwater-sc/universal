// values.cpp: generate bit patterns and values for a 16-bit takum
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/compiler.hpp>
#include <universal/number/takum/takum.hpp>
#include <universal/verification/test_suite.hpp>

void valueBringup() {
	using namespace sw::universal;

	ReportValue(std::exp2(0), "exp2(0)");
	ReportValue(std::exp2(1), "exp2(1)");
	ReportValue(std::exp2(2), "exp2(2)");
	ReportValue(std::exp2(4), "exp2(4)");
	ReportValue(std::exp2(254), "exp2(254)");

	using Takum = takum<16, uint16_t>;
	Takum a{};
	a.setbits(0x0001);
	std::cout << to_binary(a) << " : ";
	double f = double(a);
	std::cout << f << '\n';
	a.setbits(0x07F8);
	std::cout << to_binary(a) << " : ";
	f = double(a);
	std::cout << f << '\n';
	a.setbits(0x7FF8);
	std::cout << to_binary(a) << " : ";
	f = double(a);
	std::cout << f << '\n';
}

template<unsigned nbits>
void GenerateTakumValues() {
	static_assert(nbits <= 16, "takum size too big for reasonable table generation");
	using namespace sw::universal;

	using Real = takum<nbits, uint32_t>;
	Real a{ 0 };

	const unsigned NR_VALUES = (1ull << nbits);
	for (unsigned i = 0; i < NR_VALUES; ++i) {
		a.setbits(i);
		ReportValue(a, "takum");
		// if you also want to color print
		// std::cout << to_binary(a) << " : " << a << " : " << color_print(a) << '\n';
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
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "takum value generation";
	std::string test_tag    = "values";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// important behavioral traits
	// ReportTrivialityOfType<takum<16, std::uint8_t>>();

#if MANUAL_TESTING
	{
		takum<12, uint16_t> a;
		a.setbits(0x800);
		if (a.isnar()) std::cout << "nar\n"; else std::cout << "not nar\n";

		std::cout << to_binary(a) << " : " << a << " : " << color_print(a) << '\n';
	}
#else

	GenerateTakumValues<16>();

#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
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
