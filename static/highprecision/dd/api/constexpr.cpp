// constexpr.cpp: compile-time tests for dd (double-double)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure dd: throwing arithmetic exceptions disabled so divide-by-zero
// has defined constexpr-safe behavior (returns +/-inf or NaN encoding) rather
// than throwing, which would be ill-formed in a constant expression.
#define DD_THROW_ARITHMETIC_EXCEPTION 0

#include <iostream>
#include <iomanip>
#include <universal/number/dd/dd.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "dd constexpr verification";
	std::string test_tag    = "constexpr";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// dd is a (1, 11, 106) floating-point triple stored as an unevaluated sum
	// of two IEEE-754 doubles. Values that fit exactly in a single double
	// (e.g. small integers, powers of 2) construct with lo == 0.0 and round-
	// trip through dd arithmetic without precision loss.

	// ----------------------------------------------------------------------------
	// Native int construction (already constexpr -- smoke test only)
	// ----------------------------------------------------------------------------
	{
		constexpr dd a(0);
		constexpr dd b(1);
		constexpr dd c(-3);
		(void)a; (void)b; (void)c;
	}

	// ----------------------------------------------------------------------------
	// Acceptance form from issue #727:
	//   constexpr dd a(2.0), b(3.0); constexpr auto c = a + b;
	// (and analogous for *, -, /, +=, <)
	// ----------------------------------------------------------------------------
	{
		constexpr dd a(2.0);
		constexpr dd b(3.0);
		constexpr auto cx_sum = a + b;  // 2 + 3 = 5
		static_assert(cx_sum == dd(5.0), "issue #727 acceptance: a + b");
	}

	// ----------------------------------------------------------------------------
	// All four binary arithmetic operators in constexpr context
	// Operands chosen so all results are exactly representable in binary64
	// (so dd's lo limb is 0 and equality with the literal-constructed dd holds).
	// ----------------------------------------------------------------------------
	{
		constexpr dd a(4.5);
		constexpr dd b(1.5);
		constexpr auto cx_sum  = a + b;          // 4.5 + 1.5 = 6.0
		constexpr auto cx_diff = a - b;          // 4.5 - 1.5 = 3.0
		constexpr auto cx_prod = a * b;          // 4.5 * 1.5 = 6.75
		constexpr auto cx_quot = a / b;          // 4.5 / 1.5 = 3.0
		constexpr auto cx_neg  = -a;             // -4.5
		static_assert(cx_sum  == dd(6.0),  "constexpr +  failed");
		static_assert(cx_diff == dd(3.0),  "constexpr -  failed");
		static_assert(cx_prod == dd(6.75), "constexpr *  failed");
		static_assert(cx_quot == dd(3.0),  "constexpr /  failed");
		static_assert(cx_neg  == dd(-4.5), "constexpr unary - failed");

		// Compound assignment via lambda (constexpr lambdas are C++20)
		constexpr dd cx_addeq = []() { dd t(1.5); t += dd(3.0); return t; }();
		constexpr dd cx_subeq = []() { dd t(4.5); t -= dd(1.5); return t; }();
		constexpr dd cx_muleq = []() { dd t(1.5); t *= dd(3.0); return t; }();
		constexpr dd cx_diveq = []() { dd t(4.5); t /= dd(1.5); return t; }();
		static_assert(cx_addeq == dd(4.5), "constexpr += failed");
		static_assert(cx_subeq == dd(3.0), "constexpr -= failed");
		static_assert(cx_muleq == dd(4.5), "constexpr *= failed");
		static_assert(cx_diveq == dd(3.0), "constexpr /= failed");

		// Compound assignment with double rhs
		constexpr dd cx_addeqd = []() { dd t(1.5); t += 3.0; return t; }();
		constexpr dd cx_subeqd = []() { dd t(4.5); t -= 1.5; return t; }();
		constexpr dd cx_muleqd = []() { dd t(1.5); t *= 3.0; return t; }();
		constexpr dd cx_diveqd = []() { dd t(4.5); t /= 1.5; return t; }();
		static_assert(cx_addeqd == dd(4.5), "constexpr += double failed");
		static_assert(cx_subeqd == dd(3.0), "constexpr -= double failed");
		static_assert(cx_muleqd == dd(4.5), "constexpr *= double failed");
		static_assert(cx_diveqd == dd(3.0), "constexpr /= double failed");
	}

	// ----------------------------------------------------------------------------
	// Constexpr comparison (issue acceptance: <)
	// ----------------------------------------------------------------------------
	{
		constexpr dd a(1.5);
		constexpr dd b(3.0);
		static_assert(a < b,     "constexpr: dd(1.5) < dd(3.0)");
		static_assert(b > a,     "constexpr: dd(3.0) > dd(1.5)");
		static_assert(a <= b,    "constexpr: dd(1.5) <= dd(3.0)");
		static_assert(b >= a,    "constexpr: dd(3.0) >= dd(1.5)");
		static_assert(!(a == b), "constexpr: dd(1.5) != dd(3.0)");
		static_assert(a != b,    "constexpr: dd(1.5) != dd(3.0) (operator!=)");
		static_assert(a == a,    "constexpr: dd(1.5) == dd(1.5)");

		// dd vs double comparisons
		static_assert(a < 3.0,      "constexpr: dd(1.5) < 3.0");
		static_assert(3.0 > a,      "constexpr: 3.0 > dd(1.5)");
		static_assert(a == 1.5,     "constexpr: dd(1.5) == 1.5");
		static_assert(1.5 == a,     "constexpr: 1.5 == dd(1.5)");
	}

	// ----------------------------------------------------------------------------
	// Conversion-out: operator double() and operator int() are now constexpr
	// ----------------------------------------------------------------------------
	{
		constexpr dd a(0.5);
		constexpr double d = double(a);
		static_assert(d == 0.5, "constexpr operator double()");

		constexpr dd b(42.0);
		constexpr int n = int(b);
		static_assert(n == 42, "constexpr operator int()");

		constexpr dd c(2.5);
		constexpr float f = float(c);
		static_assert(f == 2.5f, "constexpr operator float()");
	}

	// ----------------------------------------------------------------------------
	// SpecificValue construction (already constexpr -- smoke test)
	// ----------------------------------------------------------------------------
	{
		constexpr dd zero(SpecificValue::zero);
		constexpr dd inf_pos(SpecificValue::infpos);
		constexpr dd inf_neg(SpecificValue::infneg);
		static_assert(zero == dd(0.0), "constexpr SpecificValue::zero");
		static_assert(inf_pos > dd(0.0), "constexpr SpecificValue::infpos");
		static_assert(inf_neg < dd(0.0), "constexpr SpecificValue::infneg");
	}

	// ----------------------------------------------------------------------------
	// Mixed dd / double binary arithmetic in constexpr context
	// ----------------------------------------------------------------------------
	{
		constexpr dd a(2.0);
		constexpr auto cx_sum  = a + 3.0;     // 2 + 3 = 5
		constexpr auto cx_diff = a - 1.0;     // 2 - 1 = 1
		constexpr auto cx_prod = a * 4.0;     // 2 * 4 = 8
		constexpr auto cx_quot = a / 2.0;     // 2 / 2 = 1
		static_assert(cx_sum  == dd(5.0), "constexpr dd + double");
		static_assert(cx_diff == dd(1.0), "constexpr dd - double");
		static_assert(cx_prod == dd(8.0), "constexpr dd * double");
		static_assert(cx_quot == dd(1.0), "constexpr dd / double");

		constexpr auto cx_lhs_sum = 5.0 + dd(2.0);
		static_assert(cx_lhs_sum == dd(7.0), "constexpr double + dd");
	}

	std::cout << "dd constexpr verification: PASS\n";

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const sw::universal::dd_arithmetic_exception& err) {
	std::cerr << "Uncaught dd arithmetic exception: " << err.what() << '\n';
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
