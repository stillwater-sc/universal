// api.cpp: application programming interface demonstration of fixed-size, arbitrary precision logarithmic number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/number/cfloat/cfloat.hpp>  // for comparison
#include <math/polynomial/horners.hpp>
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

	std::string test_suite  = "lns API demonstration";
	std::string test_tag    = "api";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// important behavioral traits
	ReportTrivialityOfType<lns<8, 2>>();

	{
		lns<5, 2> l(1);
		std::cout << to_binary(l) << " : " << l << " : " << color_print(l) << '\n';
	}

	// default behavior

	{
		std::cout << "+---------    default lns bahavior   --------+\n";
		using Real = lns<8, 3>;
		Real a(1.0f), b(1.0f), c;
		ArithmeticOperators<Real>(a, b);
		a = 1;  // integer assignment
		b = 1;
		c = a + b;
		ReportBinaryOperation(a, "+", b, c);
	}

	// configuration
	
	{
		std::cout << "+---------    arithmetic operators with explicit alignment bahavior   --------+\n";
		using lns16 = lns<16, 5, std::uint16_t>;
		ArithmeticOperators<lns16>(1.0f, 1.0f);

		using lns24 = lns<24, 5, std::uint32_t>;
		ArithmeticOperators<lns24>(1.0f, 1.0f);
	}

	{
		std::cout << "+---------    Dynamic ranges of lns<> configurations   --------+\n";
		std::cout << dynamic_range(lns< 4, 2>()) << '\n';
		std::cout << dynamic_range(lns< 8, 3>()) << '\n';
		std::cout << dynamic_range(lns<12, 4>()) << '\n';
		std::cout << dynamic_range(lns<16, 5>()) << '\n';
		std::cout << dynamic_range(lns<20, 6>()) << '\n';
	}

	{
		std::cout << "+---------    constexpr and specific values   --------+\n";
		constexpr size_t nbits = 10;
		constexpr size_t rbits = 3;
		using Real = lns<nbits, rbits>;  // BlockType = uint8_t, behavior = Saturating

		CONSTEXPRESSION Real a{}; // zero constexpr
		std::cout << type_tag(a) << '\n';

		// TODO: needs a constexpr version of log2() function
//		CONSTEXPRESSION Real b(1.0f);  // constexpr of a native type conversion
//		std::cout << to_binary(b) << " : " << b << '\n';

		// TODO: conversion functions need to be constexpr
//		CONSTEXPRESSION Real c(SpecificValue::minpos);  // constexpr of a special value in the encoding
//		constexpr float fminpos = float(c);
//		float f = 1.0f;
//		if (f < fminpos) {
//			std::cout << f << " is smaller than lns minpos\n";
//		}
//		else {
//			std::cout << f << " is larger than lns minpos\n";
//		}
//		std::cout << to_binary(c) << " : " << c << " == minpos" << '\n';

		CONSTEXPRESSION Real d(SpecificValue::maxpos);  // constexpr of a special value in the encoding
		std::cout << to_binary(d) << " : " << d << " == maxpos" << '\n';
	}

	{
		std::cout << "+---------    extreme values   --------+\n";
		constexpr size_t nbits = 10;
		constexpr size_t rbits = 3;
		using Real = lns<nbits, rbits>;  // BlockType = uint8_t, behavior = Saturating

		Real a, b, c;

		a = INFINITY;
		b = 2;
		c = a / b;
		std::cout << "scale(" << a << ") = " << a.scale() << '\n';
		std::cout << "scale(" << b << ") = " << b.scale() << '\n';
		ReportBinaryOperation(a, "/", b, c);
	}

	{
		std::cout << "+---------    exceptions   ---------+\n";
		using lns = sw::universal::lns<16, 8, uint16_t>;
		lns a = lns(0.0f);
		lns b = -lns(0.0);
		// if (a != b) std::cout << "you can't compare indeterminate NaN\n";
		if (a.isnan() && b.isnan()) std::cout << "PASS: both +lns(0) and -lns(0) are indeterminate\n";
		std::cout << "+lns(0.0f): " <<  lns(0.0f) << "\n";
		std::cout << "-lns(0.0f): " << -lns(0.0f) << "\n";
	}

	{
		std::cout << "+---------    dynamic ranges of 8-bit lns<> configurations   --------+\n";
		std::cout << dynamic_range(lns<8, 0>()) << '\n';
		std::cout << dynamic_range(lns<8, 1>()) << '\n';
		std::cout << dynamic_range(lns<8, 2>()) << '\n';
		std::cout << dynamic_range(lns<8, 3>()) << '\n';
		std::cout << dynamic_range(lns<8, 4>()) << '\n';
		std::cout << dynamic_range(lns<8, 5>()) << '\n';
		std::cout << dynamic_range(lns<8, 6>()) << '\n';
	}

	{
		std::cout << "+---------    comparison to classic floats   --------+\n";
		using LNS = lns<16, 8, std::uint16_t>;
		using Real = cfloat<16, 5, std::uint16_t>;
		LNS a;
		Real b;
		static_assert(std::is_trivially_constructible<LNS>(), "lns<> is not trivially constructible");
		a = 1;
		std::cout << std::setw(80) << type_tag(a) << " : " << to_binary(a, true) << " : " << color_print(a, true) << " : " << float(a) << '\n';
		b = 1;
		std::cout << std::setw(80) << type_tag(b) << " : " << to_binary(b, true) << " : " << color_print(b, true) << " : " << float(b) << '\n';
	}
	

	std::cout << "+------------ Horner's Rule ----------+\n";
	{
		using LNS = lns<16, 8, std::uint16_t>;
		std::vector<LNS> polynomial = {
			1.0, 1.0, 1.0, 1.0, 1.0, 1.0
		};

		std::cout << "polynomial(1.0) = " << sw::math::polynomial::polyeval(polynomial, 5, LNS(1.0f)) << '\n';
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
