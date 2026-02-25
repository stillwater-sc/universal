// api.cpp: application programming interface demonstration of fixed-size, arbitrary precision double base number systems
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/dbns/dbns.hpp>
#include <universal/number/dbns/table.hpp>
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

template<typename Real>
inline constexpr Real bases(const Real&, const Real& base) {
	std::cout << "         base : " << base << '\n';
	return base;
}

template<typename Real, typename BaseHead, typename ...OtherBases>
inline constexpr Real bases(const Real& x, const BaseHead& bHead, const OtherBases& ...otherBases) {
	std::cout << "bases<>  base : " << bHead << '\n';
	return bases(x, static_cast<Real>(otherBases)...);
}

template<typename Real, unsigned nrBases>
class lnsBases {
public:

private:
	Real base[nrBases];
};

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "dbns API demonstration";
	std::string test_tag = "api";
	bool reportTestCases = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#ifdef EXTRA
	{
		// experiment with variadic template arguments to specify multi-base lns configurations
		float x{ 1.0f };
		bases(x, 2, 3, 5.5f, 7.1, 9);
	}

	{
		dbns<8, 3> l(1);
		std::cout << to_binary(l) << " : " << l << " : " << color_print(l) << '\n';
		l.debugConstexprParameters();

		l.setbits(0xf5);
		std::cout << to_binary(l) << " : " << l << " : " << color_print(l) << '\n';
		std::cout << to_binary(l.extractExponent(0), 4) << " : " << to_binary(l.extractExponent(1), 4) << '\n';

		std::cout << dynamic_range(l) << '\n';
	}
#endif

	{
		std::cout << "+-------- important behavioral traits   --------+\n";
		ReportTrivialityOfType<dbns<8, 3>>();
	}

	// default behavior
	{
		std::cout << "+---------    default dbns bahavior   --------+\n";
		using Real = dbns<8, 3>;
		Real a(0.5f), b(1.0f), c{ 0 };

		c = a + b;
		ReportBinaryOperation(a, "+", b, c);
		c = a - b;
		ReportBinaryOperation(a, "-", b, c);
		c = a * b;
		ReportBinaryOperation(a, "*", b, c);
		c = a / b;
		ReportBinaryOperation(a, "/", b, c);
	}

	{
		std::cout << "+---------    dynamic ranges of 8-bit dbns<> configurations   --------+\n";
		//		std::cout << symmetry_range(dbns<8, 0>()) << '\n';
		std::cout << symmetry_range(dbns<8, 1>()) << '\n';
		std::cout << symmetry_range(dbns<8, 2>()) << '\n';
		std::cout << symmetry_range(dbns<8, 3>()) << '\n';
		std::cout << symmetry_range(dbns<8, 4>()) << '\n';
		std::cout << symmetry_range(dbns<8, 5>()) << '\n';
		std::cout << symmetry_range(dbns<8, 6>()) << '\n';
	}

	// configuration
	{
		std::cout << "+---------    arithmetic operators with explicit alignment bahavior   --------+\n";
		//		using dbns16 = dbns<16, 5, std::uint16_t>;
		//		ArithmeticOperators<dbns16>(1.0f, 1.0f);

		//		using dbns24 = dbns<24, 5, std::uint32_t>;
		//		ArithmeticOperators<dbns24>(1.0f, 1.0f);
	}

	{
		std::cout << "+---------    Dynamic ranges of dbns<> configurations   --------+\n";
		std::cout << dynamic_range(dbns< 4,  2>()) << '\n';
		std::cout << dynamic_range(dbns< 8,  3>()) << '\n';
		std::cout << dynamic_range(dbns<12,  6>()) << '\n';
		// double-base number systems with bases {0.5,3} 
		// grow too quickly to represent with doubles
		// as shown with the following two configurations.
		std::cout << dynamic_range(dbns<16,  8>()) << '\n';
		std::cout << dynamic_range(dbns<20, 12>()) << '\n';
	}

	{
		std::cout << "+---------    constexpr and specific values   --------+\n";
		constexpr size_t nbits = 10;
		constexpr size_t rbits = 3;
		using Real = dbns<nbits, rbits>;  // BlockType = uint8_t, behavior = Saturating

		//		CONSTEXPRESSION Real a{}; // zero constexpr
		//		std::cout << type_tag<Real>(a) << '\n';  // TODO: type_tag doesn't work for dbns

				// TODO: needs a constexpr version of log2() function
		//		CONSTEXPRESSION Real b(1.0f);  // constexpr of a native type conversion
		//		std::cout << to_binary(b) << " : " << b << '\n';

		CONSTEXPRESSION Real c(SpecificValue::minpos);  // constexpr of a special value in the encoding
		std::cout << to_binary(c) << " : " << c << " == minpos" << '\n';

		CONSTEXPRESSION Real d(SpecificValue::maxpos);  // constexpr of a special value in the encoding
		std::cout << to_binary(d) << " : " << d << " == maxpos" << '\n';
	}

	{
		std::cout << "+---------    extreme values   --------+\n";
		constexpr size_t nbits = 10;
		constexpr size_t rbits = 3;
		using Real = dbns<nbits, rbits>;  // BlockType = uint8_t, behavior = Saturating

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
		using dbns = sw::universal::dbns<16, 8, uint16_t>;
		dbns a = dbns(0.0f);
		dbns b = -dbns(0.0);
		// if (a != b) std::cout << "you can't compare indeterminate NaN\n";
		if (a.isnan() && b.isnan()) std::cout << "PASS: both +dbns(0) and -dbns(0) are indeterminate\n";
		std::cout << "+dbns(0.0f): " << dbns(0.0f) << "\n";
		std::cout << "-dbns(0.0f): " << -dbns(0.0f) << "\n";
	}
	
	{
		std::cout << "+---------    extract exponents   --------+\n";
		{
			dbns<8, 3> l;  // 1 limb
			l.setbits(0x11);
			ReportValue(l);
			std::cout << "first  exponent : " << l.extractExponent(0) << '\n';
			std::cout << "second exponent : " << l.extractExponent(1) << '\n';
		}
		{
			dbns<16, 9> l; // two limbs
			l.setbits(0x1fff);
			ReportValue(l);
			std::cout << "first  exponent : " << l.extractExponent(0) << '\n';
			std::cout << "second exponent : " << l.extractExponent(1) << '\n';
		}
	}

	{
		std::cout << "+---------    comparison to classic floats   --------+\n";
		using dbns = dbns<16, 8, std::uint16_t>;
		using Real = cfloat<16, 5, std::uint16_t>;
		dbns a;
		Real b;
		static_assert(std::is_trivially_constructible<dbns>(), "dbns<> is not trivially constructible");
		a = 1;
//		std::cout << std::setw(80) << type_tag(a) << " : " << to_binary(a, true) << " : " << color_print(a, true) << " : " << float(a) << '\n';
		b = 1;
		std::cout << std::setw(80) << type_tag(b) << " : " << to_binary(b, true) << " : " << color_print(b, true) << " : " << float(b) << '\n';
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
