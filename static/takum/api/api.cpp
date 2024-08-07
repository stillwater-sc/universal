// api.cpp: application programming interface demonstration of fixed-size, arbitrary precision takum number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/compiler.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
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

	std::string test_suite = "takum API demonstration";
	std::string test_tag = "api";
	bool reportTestCases = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// important behavioral traits
	ReportTrivialityOfType<takum<16, std::uint8_t>>();

	{
		fixpnt<16, 8> fp(1);
		std::cout << std::setw(22) << to_binary(fp) << " : " << fp << " : " << color_print(fp) << '\n';
		cfloat<16, 5> f(1);
		std::cout << std::setw(22) << to_binary(f) << " : " << f << " : " << color_print(f) << '\n';
		posit<16, 2> p(1);
		std::cout << std::setw(22) << to_binary(p) << " : " << p << " : " << color_print(p) << '\n';
		takum<16> l(1);
		std::cout << std::setw(22) << to_binary(l) << " : " << l << " : " << color_print(l) << '\n';
	}

	// default behavior

	{
		std::cout << "+---------    default takum bahavior   --------+\n";
		using Real = takum<16>;
		Real a(1.0f), b(1.0f);
		ArithmeticOperators<Real>(a, b);
	}

	// configuration

	{
		std::cout << "+---------    arithmetic operators with explicit alignment bahavior   --------+\n";
		using takum16 = takum<16, std::uint16_t>;
		ArithmeticOperators<takum16>(1.0f, 1.0f);

		using takum24 = takum<24, std::uint32_t>;
		ArithmeticOperators<takum24>(1.0f, 1.0f);
	}

	{
		std::cout << "+---------    Dynamic ranges of takum<> configurations   --------+\n";
		//		std::cout << dynamic_range(takum< 4>()) << '\n';  // this is not a valid configuration
		std::cout << dynamic_range<takum<8>>() << '\n';
		std::cout << dynamic_range<takum<12>>() << '\n';
		std::cout << dynamic_range<takum<16>>() << '\n';
		std::cout << dynamic_range<takum<20>>() << '\n';
	}

#if BIT_CAST_IS_CONSTEXPR
	{
		std::cout << "+---------    constexpr and specific values   --------+\n";
		constexpr size_t nbits = 10;
		using Real = takum<nbits>;  // BlockType = uint8_t

		BIT_CAST_CONSTEXPR Real a{}; // zero constexpr
		std::cout << type_tag(a) << '\n';

		// constexpr of a native floating-point type construction requires a constexpr log2 and floor library function
		// CONSTEXPRESSION Real b(1.0f);  // constexpr of a native type conversion
		// std::cout << to_binary(b) << " : " << b << '\n';

		BIT_CAST_CONSTEXPR Real c(SpecificValue::minpos);  // constexpr of a special value in the encoding
		constexpr float fminpos = float(c);
		float f = 1.0f;
		if (f < fminpos) {
			std::cout << f << " is smaller than takum minpos " << fminpos << '\n';
		}
		else {
			std::cout << f << " is larger than takum minpos " << fminpos << '\n';
		}
		std::cout << to_binary(c) << " : " << c << " == minpos" << '\n';

		BIT_CAST_CONSTEXPR Real d(SpecificValue::maxpos);  // constexpr of a special value in the encoding
		std::cout << to_binary(d) << " : " << d << " == maxpos" << '\n';
	}
#else   // ! BIT_CAST_IS_CONSTEXPR
	{
		std::cout << "+---------    constexpr and specific values   --------+\n";
		std::cout << "compiler does not support constexpr on native floating-point types\n";
		report_compiler();
	}
#endif  // BIT_CAST_IS_CONSTEXPR

	{
		std::cout << "+---------    extreme values   --------+\n";
		constexpr size_t nbits = 10;
		using Real = takum<nbits>;  // BlockType = uint8_t

		Real a, b, c;

		a = INFINITY;
		b = 2;
		c = a / b;
		std::cout << "scale(" << a << ") = " << a.scale() << '\n';
		std::cout << "scale(" << b << ") = " << b.scale() << '\n';
		ReportBinaryOperation(a, "/", b, c);
	}

	{
		std::cout << "+---------    state queries   ---------+\n";
		constexpr size_t nbits = 16;
		using Real = takum<nbits>;

		Real a{ 0 };
		a.debugConstexprParameters();
		if (!a.iszero()) std::cout << "PASS: zero\n"; else std::cout << "FAIL: zero\n";
		a.setbits(0x8000); // set to NaR
		if (!a.isnar()) std::cout << "PASS: NaR\n"; else std::cout << "FAIL: NaR\n";
	}

	{
		std::cout << "+---------    exceptions   ---------+\n";
		using Real = sw::universal::takum<16, uint16_t>;
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
