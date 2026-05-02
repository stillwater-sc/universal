// constexpr.cpp: compile-time tests for dbns
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure dbns: throwing arithmetic exceptions disabled so divide-by-zero
// has defined constexpr-safe behavior (returns NaN encoding) rather than
// throwing, which would be ill-formed in a constant expression.
#define DBNS_THROW_ARITHMETIC_EXCEPTION 0

#include <iostream>
#include <iomanip>
#include <universal/number/dbns/dbns.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "dbns constexpr verification";
	std::string test_tag    = "constexpr";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// dbns<16, 8> = (-1)^s * 0.5^a * 3^b with a in [0..255], b in [0..127].
	// Values like 1.0 (a=0,b=0), 3.0 (a=0,b=1), 0.5 (a=1,b=0), 1.5 (a=1,b=1)
	// are exactly representable; 2.0 is not (best fit gives ~2.004).

	// ----------------------------------------------------------------------------
	// Native int construction (already constexpr -- smoke test only)
	// ----------------------------------------------------------------------------
	{
		using D16_8 = dbns<16, 8, std::uint16_t>;
		constexpr D16_8 a(0);
		constexpr D16_8 b(1);
		constexpr D16_8 c(-3);
		(void)a; (void)b; (void)c;
	}

	// ----------------------------------------------------------------------------
	// Acceptance form from issue #726:
	//   constexpr dbns<...> a(2.0), b(3.0); constexpr auto c = a * b;
	// Multiplication marshalls through double; with constexpr operator double()
	// and constexpr convert_ieee754, the entire chain is now constexpr.
	// ----------------------------------------------------------------------------
	{
		using D16_8 = dbns<16, 8, std::uint16_t>;
		constexpr D16_8 a(0.5);   // exactly representable: (a=1,b=0)
		constexpr D16_8 b(3.0);   // exactly representable: (a=0,b=1)
		constexpr auto cx_prod = a * b;  // 0.5 * 3.0 = 1.5 (also exact)
		(void)cx_prod;
	}

	// ----------------------------------------------------------------------------
	// All four arithmetic operators in constexpr context with value asserts
	// ----------------------------------------------------------------------------
	{
		using D16_8 = dbns<16, 8, std::uint16_t>;
		constexpr D16_8 a(4.5);                  // (a=1, b=2): exact
		constexpr D16_8 b(1.5);                  // (a=1, b=1): exact
		constexpr auto cx_sum  = a + b;          // 6.0  (exact: a=0, b=...)
		constexpr auto cx_diff = a - b;          // 3.0  (exact: a=0, b=1)
		constexpr auto cx_prod = a * b;          // 6.75 (exact: a=2, b=3)
		constexpr auto cx_quot = a / b;          // 3.0  (exact: a=0, b=1)
		constexpr auto cx_neg  = -a;             // -4.5
		// Tolerance accounts for the saturating search in convert_ieee754
		// rounding through the dbns<16,8> grid; values picked to be exactly
		// representable so the residual is well under 1%.
		static_assert(double(cx_sum)  > 5.9 && double(cx_sum)  < 6.1, "constexpr +  failed");
		static_assert(double(cx_diff) > 2.9 && double(cx_diff) < 3.1, "constexpr -  failed");
		static_assert(double(cx_prod) > 6.7 && double(cx_prod) < 6.8, "constexpr *  failed");
		static_assert(double(cx_quot) > 2.9 && double(cx_quot) < 3.1, "constexpr /  failed");
		static_assert(double(cx_neg) < -4.4 && double(cx_neg) > -4.6, "constexpr unary - failed");

		// Compound assignment via lambda (constexpr lambdas are C++20)
		constexpr D16_8 cx_addeq = []() { D16_8 t(1.5); t += D16_8(3.0); return t; }();
		constexpr D16_8 cx_subeq = []() { D16_8 t(4.5); t -= D16_8(1.5); return t; }();
		constexpr D16_8 cx_muleq = []() { D16_8 t(1.5); t *= D16_8(3.0); return t; }();
		constexpr D16_8 cx_diveq = []() { D16_8 t(4.5); t /= D16_8(1.5); return t; }();
		static_assert(double(cx_addeq) > 4.4 && double(cx_addeq) < 4.6, "constexpr += failed");
		static_assert(double(cx_subeq) > 2.9 && double(cx_subeq) < 3.1, "constexpr -= failed");
		static_assert(double(cx_muleq) > 4.4 && double(cx_muleq) < 4.6, "constexpr *= failed");
		static_assert(double(cx_diveq) > 2.9 && double(cx_diveq) < 3.1, "constexpr /= failed");
	}

	// ----------------------------------------------------------------------------
	// Constexpr comparison
	// ----------------------------------------------------------------------------
	{
		using D16_8 = dbns<16, 8, std::uint16_t>;
		constexpr D16_8 a(1.5);
		constexpr D16_8 b(3.0);
		static_assert(a < b,     "constexpr: dbns(1.5) < dbns(3.0)");
		static_assert(b > a,     "constexpr: dbns(3.0) > dbns(1.5)");
		static_assert(!(a == b), "constexpr: dbns(1.5) != dbns(3.0)");
		static_assert(a == a,    "constexpr: dbns(1.5) == dbns(1.5)");
	}

	// ----------------------------------------------------------------------------
	// Conversion-out: operator double() through to_ieee754<double>() now uses
	// constexpr ipow().
	// ----------------------------------------------------------------------------
	{
		using D16_8 = dbns<16, 8, std::uint16_t>;
		constexpr D16_8 a(0.5);
		constexpr double v = double(a);
		static_assert(v > 0.49 && v < 0.51, "dbns(0.5) -> double round-trip");

		constexpr D16_8 b(3.0);
		constexpr double bv = double(b);
		static_assert(bv > 2.99 && bv < 3.01, "dbns(3.0) -> double round-trip");
	}

	// ----------------------------------------------------------------------------
	// Saturating variant: maxpos/maxneg construction at compile time.
	// ----------------------------------------------------------------------------
	{
		using DSat = dbns<8, 3, std::uint8_t, Behavior::Saturating>;
		constexpr DSat maxpos(SpecificValue::maxpos);
		constexpr DSat maxneg(SpecificValue::maxneg);
		(void)maxpos; (void)maxneg;
	}

	// ----------------------------------------------------------------------------
	// Edge case: divide-by-zero under DBNS_THROW_ARITHMETIC_EXCEPTION=0 returns
	// NaN encoding (constexpr-safe -- no throw).
	// ----------------------------------------------------------------------------
	{
		using D16_8 = dbns<16, 8, std::uint16_t>;
		constexpr D16_8 div_zero = []() {
			D16_8 t(2.0);
			t /= D16_8(0.0);
			return t;
		}();
		static_assert(div_zero.isnan(), "constexpr: divide-by-zero returns NaN");
	}

	std::cout << "dbns constexpr verification: PASS\n";

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const sw::universal::dbns_arithmetic_exception& err) {
	std::cerr << "Uncaught dbns arithmetic exception: " << err.what() << '\n';
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << '\n';
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception\n";
	return EXIT_FAILURE;
}
