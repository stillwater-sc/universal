// lns_comparison.cpp: comparision between double base and classic logarithmic number systems
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/number/dbns/dbns.hpp>
#include <universal/number/cfloat/cfloat.hpp>  // bit field comparisons
#include <universal/verification/test_suite.hpp>

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

	std::string test_suite  = "1-base vs 2-base lns comparison";
	std::string test_tag    = "api";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	{
		using Ty = dbns<8, 3, std::uint8_t, Behavior::Saturating>;
		
		Ty l(1);
		std::cout << to_binary(l) << " : " << l << " : " << color_print(l) << '\n';
		l.debugConstexprParameters();

		l.setbits(0xf5);
		std::cout << to_binary(l) << " : " << l << " : " << color_print(l) << '\n';
		std::cout << to_binary(l.extractExponent(0), true, 4) << " : " << to_binary(l.extractExponent(1), true, 4) << '\n';

		std::cout << dynamic_range(l) << '\n';
	}

	{
		std::cout << "\n+---------    dynamic ranges of 8-bit lns<> configurations   --------+\n";
		std::cout << symmetry_range(lns<8, 1>()) << '\n';
		std::cout << symmetry_range(lns<8, 2>()) << '\n';
		std::cout << symmetry_range(lns<8, 3>()) << '\n';
		std::cout << symmetry_range(lns<8, 4>()) << '\n';
		std::cout << symmetry_range(lns<8, 5>()) << '\n';
		std::cout << symmetry_range(lns<8, 6>()) << '\n';
	}

	{
		std::cout << "\n+---------    dynamic ranges of 8-bit dbns<> configurations   --------+\n";
		std::cout << symmetry_range(dbns<8, 1>()) << '\n';
		std::cout << symmetry_range(dbns<8, 2>()) << '\n';
		std::cout << symmetry_range(dbns<8, 3>()) << '\n';
		std::cout << symmetry_range(dbns<8, 4>()) << '\n';
		std::cout << symmetry_range(dbns<8, 5>()) << '\n';
		std::cout << symmetry_range(dbns<8, 6>()) << '\n';
	}

	{
		std::cout << "\n+---------    dynamic ranges of 8-bit cfloat<> configurations (with sub and supernormals)   --------+\n";
		std::cout << symmetry_range(cfloat<8, 1, std::uint8_t, true, true, false>()) << '\n';
		std::cout << symmetry_range(cfloat<8, 2, std::uint8_t, true, true, false>()) << '\n';
		std::cout << symmetry_range(cfloat<8, 3, std::uint8_t, true, true, false>()) << '\n';
		std::cout << symmetry_range(cfloat<8, 4, std::uint8_t, true, true, false>()) << '\n';
		std::cout << symmetry_range(cfloat<8, 5, std::uint8_t, true, true, false>()) << '\n';
		std::cout << symmetry_range(cfloat<8, 6, std::uint8_t, true, true, false>()) << '\n';
	}

	{
		std::cout << "\n+---------    specific type range function\n";
		lns<7, 3> a{};
		dbns<7, 3> b{};
		std::cout << lns_range(a) << '\n';
		std::cout << dbns_range(b) << '\n';
	}

	{
		std::cout << "\n+---------    cross-lns sign() functions\n";
		lns<7, 3> a;
		dbns<7, 3> b;

		a.setbits(0x7f);
		std::cout << std::setw(45) << type_tag(a) << " : " << to_binary(a) << " : " << a << " : " << (sign(a) ? "sign = 1" : "sign = 0") << '\n';

		b.setbits(0x7f);
		std::cout << std::setw(45) << type_tag(b) << " : " << to_binary(b) << " : " << b << " : " << (sign(b) ? "sign = 1" : "sign = 0") << '\n';
	}

	{
		std::cout << "\n+---------    comparison to classic floats   --------+\n";
		using dbns = dbns<16, 8, std::uint16_t>;
		using Real = cfloat<16, 5, std::uint16_t>;
		dbns a;
		Real b;
		static_assert(std::is_trivially_constructible<dbns>(), "dbns<> is not trivially constructible");
		a = 1;
		std::cout << std::setw(80) << type_tag(a) << " : " << to_binary(a, true) << " : " << color_print(a, true) << " : " << float(a) << '\n';
		b = 1;
		std::cout << std::setw(80) << type_tag(b) << " : " << to_binary(b, true) << " : " << color_print(b, true) << " : " << float(b) << '\n';
	}
	
	{
		std::cout << "\nComparitive Number traits\n";
		compareNumberTraits< lns<10, 6>, dbns<10, 6> >(std::cout);
		threeWayCompareNumberTraits< float, lns<10, 6>, dbns<10, 6> >(std::cout);
		threeWayCompareNumberTraits< fp8, bfloat_t, dbns<8, 4> >(std::cout);
	}

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
