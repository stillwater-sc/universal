// api.cpp: application programming interface demonstration of fixed-size, arbitrary precision takum number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/takum/takum.hpp>
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

	std::string test_suite  = "takum API demonstration";
	std::string test_tag    = "api";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// important behavioral traits
	ReportTrivialityOfType<takum<16, 2>>();

	{
		takum<16, 2> l(1);
		std::cout << to_binary(l) << " : " << l << " : " << color_print(l) << '\n';
	}

	// default behavior

	{
		std::cout << "+---------    default takum bahavior   --------+\n";
		using Real = takum<8, 2>;
		Real a(1.0f), b(1.0f);
		ArithmeticOperators<Real>(a, b);
	}

	// configuration
	
	{
		std::cout << "+---------    arithmetic operators with explicit alignment bahavior   --------+\n";
		using takum16 = takum<16, 2, std::uint16_t>;
		ArithmeticOperators<takum16>(1.0f, 1.0f);

		using takum24 = lns<24, 2, std::uint32_t>;
		ArithmeticOperators<takum24>(1.0f, 1.0f);
	}

	{
		std::cout << "+---------    Dynamic ranges of takum<> configurations   --------+\n";
		std::cout << dynamic_range(takum< 4, 2>()) << '\n';
		std::cout << dynamic_range(takum< 8, 3>()) << '\n';
		std::cout << dynamic_range(takum<12, 4>()) << '\n';
		std::cout << dynamic_range(takum<16, 5>()) << '\n';
		std::cout << dynamic_range(takum<20, 6>()) << '\n';
	}

	{
		std::cout << "+---------    constexpr and specific values   --------+\n";
		constexpr size_t nbits = 10;
		constexpr size_t ebits = 3;
		using Real = takum<nbits, ebits>;  // BlockType = uint8_t

		CONSTEXPRESSION Real a{}; // zero constexpr
		std::cout << type_tag(a) << '\n';

		CONSTEXPRESSION Real b(1.0f);  // constexpr of a native type conversion
		std::cout << to_binary(b) << " : " << b << '\n';

		CONSTEXPRESSION Real c(SpecificValue::minpos);  // constexpr of a special value in the encoding
		std::cout << to_binary(c) << " : " << c << " == minpos" << '\n';

		CONSTEXPRESSION Real d(SpecificValue::maxpos);  // constexpr of a special value in the encoding
		std::cout << to_binary(d) << " : " << d << " == maxpos" << '\n';
	}

	{
		std::cout << "+---------    extreme values   --------+\n";
		constexpr size_t nbits = 10;
		constexpr size_t ebits = 3;
		using Real = takum<nbits, ebits>;  // BlockType = uint8_t, behavior = Saturating

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
		using Real = sw::universal::takum<16, 8, uint16_t>;
		Real a; // set to NaR
		Real b = Real(0.0);
		if (a != b) std::cout << "you can't compare indeterminate NaR\n";
		if (a.isnar() && b.isnar()) std::cout << "PASS: both takums are indeterminate\n";
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
