// constexpr.cpp: compile-time tests for dd_cascade (double-double via floatcascade<2>)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure dd_cascade: throwing arithmetic exceptions disabled so divide-by-zero
// has defined constexpr-safe behavior (returns +/-inf or NaN encoding) rather
// than throwing, which would be ill-formed in a constant expression.
#define DD_CASCADE_THROW_ARITHMETIC_EXCEPTION 0

#include <iostream>
#include <iomanip>
#include <universal/number/dd_cascade/dd_cascade.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "dd_cascade constexpr verification";
	std::string test_tag    = "constexpr";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// dd_cascade is a (1, 11, 106) floating-point triple stored as a
	// floatcascade<2> -- an unevaluated sum of two IEEE-754 doubles.

	// ----------------------------------------------------------------------------
	// Native int construction (already constexpr -- smoke test only)
	// ----------------------------------------------------------------------------
	{
		constexpr dd_cascade a(0);
		constexpr dd_cascade b(1);
		constexpr dd_cascade c(-3);
		(void)a; (void)b; (void)c;
	}

	// ----------------------------------------------------------------------------
	// Acceptance form from issue #728:
	//   constexpr dd_cascade a(2.0), b(3.0); constexpr auto c = a + b;
	// (and analogous for *, -, /, +=, <)
	// ----------------------------------------------------------------------------
	{
		constexpr dd_cascade a(2.0);
		constexpr dd_cascade b(3.0);
		constexpr auto cx_sum = a + b;  // 2 + 3 = 5
		static_assert(cx_sum == dd_cascade(5.0), "issue #728 acceptance: a + b");
	}

	// ----------------------------------------------------------------------------
	// All four binary arithmetic operators in constexpr context
	// Operands chosen so all results are exactly representable in binary64.
	// ----------------------------------------------------------------------------
	{
		constexpr dd_cascade a(4.5);
		constexpr dd_cascade b(1.5);
		constexpr auto cx_sum  = a + b;          // 4.5 + 1.5 = 6.0
		constexpr auto cx_diff = a - b;          // 4.5 - 1.5 = 3.0
		constexpr auto cx_prod = a * b;          // 4.5 * 1.5 = 6.75
		constexpr auto cx_quot = a / b;          // 4.5 / 1.5 = 3.0
		constexpr auto cx_neg  = -a;             // -4.5
		static_assert(cx_sum  == dd_cascade(6.0),  "constexpr +  failed");
		static_assert(cx_diff == dd_cascade(3.0),  "constexpr -  failed");
		static_assert(cx_prod == dd_cascade(6.75), "constexpr *  failed");
		static_assert(cx_quot == dd_cascade(3.0),  "constexpr /  failed");
		static_assert(cx_neg  == dd_cascade(-4.5), "constexpr unary - failed");

		// Compound assignment via lambda (constexpr lambdas are C++20)
		constexpr dd_cascade cx_addeq = []() { dd_cascade t(1.5); t += dd_cascade(3.0); return t; }();
		constexpr dd_cascade cx_subeq = []() { dd_cascade t(4.5); t -= dd_cascade(1.5); return t; }();
		constexpr dd_cascade cx_muleq = []() { dd_cascade t(1.5); t *= dd_cascade(3.0); return t; }();
		constexpr dd_cascade cx_diveq = []() { dd_cascade t(4.5); t /= dd_cascade(1.5); return t; }();
		static_assert(cx_addeq == dd_cascade(4.5), "constexpr += failed");
		static_assert(cx_subeq == dd_cascade(3.0), "constexpr -= failed");
		static_assert(cx_muleq == dd_cascade(4.5), "constexpr *= failed");
		static_assert(cx_diveq == dd_cascade(3.0), "constexpr /= failed");

		// Compound assignment with double rhs
		constexpr dd_cascade cx_addeqd = []() { dd_cascade t(1.5); t += 3.0; return t; }();
		constexpr dd_cascade cx_subeqd = []() { dd_cascade t(4.5); t -= 1.5; return t; }();
		constexpr dd_cascade cx_muleqd = []() { dd_cascade t(1.5); t *= 3.0; return t; }();
		constexpr dd_cascade cx_diveqd = []() { dd_cascade t(4.5); t /= 1.5; return t; }();
		static_assert(cx_addeqd == dd_cascade(4.5), "constexpr += double failed");
		static_assert(cx_subeqd == dd_cascade(3.0), "constexpr -= double failed");
		static_assert(cx_muleqd == dd_cascade(4.5), "constexpr *= double failed");
		static_assert(cx_diveqd == dd_cascade(3.0), "constexpr /= double failed");
	}

	// ----------------------------------------------------------------------------
	// Constexpr comparison (issue acceptance: <)
	// ----------------------------------------------------------------------------
	{
		constexpr dd_cascade a(1.5);
		constexpr dd_cascade b(3.0);
		static_assert(a < b,     "constexpr: dd_cascade(1.5) < dd_cascade(3.0)");
		static_assert(b > a,     "constexpr: dd_cascade(3.0) > dd_cascade(1.5)");
		static_assert(a <= b,    "constexpr: dd_cascade(1.5) <= dd_cascade(3.0)");
		static_assert(b >= a,    "constexpr: dd_cascade(3.0) >= dd_cascade(1.5)");
		static_assert(!(a == b), "constexpr: dd_cascade(1.5) != dd_cascade(3.0)");
		static_assert(a != b,    "constexpr: != operator");
		static_assert(a == a,    "constexpr: dd_cascade(1.5) == dd_cascade(1.5)");

		// dd_cascade vs double comparisons
		static_assert(a < 3.0,      "constexpr: dd_cascade(1.5) < 3.0");
		static_assert(3.0 > a,      "constexpr: 3.0 > dd_cascade(1.5)");
		static_assert(a == 1.5,     "constexpr: dd_cascade(1.5) == 1.5");
		static_assert(1.5 == a,     "constexpr: 1.5 == dd_cascade(1.5)");
	}

	// ----------------------------------------------------------------------------
	// Conversion-out: operator double() / int() / float() are now constexpr.
	// ----------------------------------------------------------------------------
	{
		constexpr dd_cascade a(0.5);
		constexpr double d = double(a);
		static_assert(d == 0.5, "constexpr operator double()");

		constexpr dd_cascade b(42.0);
		constexpr int n = int(b);
		static_assert(n == 42, "constexpr operator int()");

		constexpr dd_cascade c(2.5);
		constexpr float f = float(c);
		static_assert(f == 2.5f, "constexpr operator float()");

		// Large integer conversion: limb-separated truncation preserves
		// dd_cascade's full 106-bit precision on every platform regardless
		// of long-double width.  dd_cascade(2^53, 1.0) represents the exact
		// integer 2^53 + 1 = 9007199254740993.
		constexpr dd_cascade large(9007199254740992.0, 1.0);
		constexpr unsigned long long u = static_cast<unsigned long long>(large);
		static_assert(u == 9007199254740993ULL,
			"constexpr conversion preserves precision above 2^53");

		// Negative large: dd_cascade(-2^53, -1.0) = -(2^53 + 1)
		constexpr dd_cascade large_neg(-9007199254740992.0, -1.0);
		constexpr long long s = static_cast<long long>(large_neg);
		static_assert(s == -9007199254740993LL,
			"constexpr signed conversion preserves precision above -2^53");

		// Boundary case: hi is integer, lo subtracts a sub-ulp fraction.
		// dd_cascade(101.0, -2^-50) ~ 100.99..., truncates to 100.
		constexpr dd_cascade boundary_neg(101.0, -0x1.0p-50);
		constexpr int b_neg = static_cast<int>(boundary_neg);
		static_assert(b_neg == 100,
			"constexpr trunc handles fractional borrow across int boundary");

		// Boundary case: positive lo pushing hi+lo over an integer boundary.
		// dd_cascade(2.5, 0.6) = 3.1, truncates to 3.
		constexpr dd_cascade boundary_pos(2.5, 0.6);
		constexpr int b_pos = static_cast<int>(boundary_pos);
		static_assert(b_pos == 3,
			"constexpr trunc handles fractional carry across int boundary");

		// Negative dd_cascade to unsigned saturates to 0.
		constexpr dd_cascade negval(-5.0);
		constexpr unsigned int neg_u = static_cast<unsigned int>(negval);
		static_assert(neg_u == 0u,
			"constexpr unsigned conversion clamps negative dd_cascade to 0");

		// Out-of-range saturates to integer max.
		constexpr dd_cascade huge(1.0e20);
		constexpr int huge_i = static_cast<int>(huge);
		static_assert(huge_i == (std::numeric_limits<int>::max)(),
			"constexpr signed conversion saturates above int max");

		// hi at the rounded long long boundary, lo brings it back in range.
		// dd_cascade(2^63, -2.0) represents 2^63 - 2 = 9223372036854775806.
		constexpr dd_cascade boundary_max(0x1.0p63, -2.0);
		constexpr long long b_max = static_cast<long long>(boundary_max);
		static_assert(b_max == 9223372036854775806LL,
			"constexpr signed conversion at upper boundary uses lo to stay in range");

		// dd_cascade(2^64, -2.0) represents 2^64 - 2 (valid unsigned long long).
		constexpr dd_cascade boundary_umax(0x1.0p64, -2.0);
		constexpr unsigned long long b_umax = static_cast<unsigned long long>(boundary_umax);
		static_assert(b_umax == 18446744073709551614ULL,
			"constexpr unsigned conversion at upper boundary uses lo to stay in range");

		// Defensive saturation for unnormalized dd_cascade values: the raw
		// two-limb constructor accepts arbitrary lo, but the limb cast must
		// not invoke C++20 [conv.fpint] UB.  These tests exercise the lo
		// out-of-range guards.
		// hi finite, lo huge positive (unnormalized) -> saturates to max
		// instead of triggering UB on static_cast<long long>(lo).
		constexpr dd_cascade unnorm_pos(0.0, 1.0e30);
		constexpr long long unp = static_cast<long long>(unnorm_pos);
		static_assert(unp == (std::numeric_limits<long long>::max)(),
			"constexpr conversion saturates on unnormalized lo > max");

		// hi finite, lo huge negative -> saturates to min for signed, 0 for unsigned.
		constexpr dd_cascade unnorm_neg(0.0, -1.0e30);
		constexpr long long unn = static_cast<long long>(unnorm_neg);
		static_assert(unn == (std::numeric_limits<long long>::min)(),
			"constexpr signed conversion saturates on unnormalized lo < min");
		constexpr unsigned long long unn_u = static_cast<unsigned long long>(unnorm_neg);
		static_assert(unn_u == 0ULL,
			"constexpr unsigned conversion clamps unnormalized negative to 0");
	}

	// ----------------------------------------------------------------------------
	// SpecificValue construction (already constexpr -- smoke test)
	// ----------------------------------------------------------------------------
	{
		constexpr dd_cascade zero(SpecificValue::zero);
		constexpr dd_cascade inf_pos(SpecificValue::infpos);
		constexpr dd_cascade inf_neg(SpecificValue::infneg);
		static_assert(zero == dd_cascade(0.0), "constexpr SpecificValue::zero");
		static_assert(inf_pos > dd_cascade(0.0), "constexpr SpecificValue::infpos");
		static_assert(inf_neg < dd_cascade(0.0), "constexpr SpecificValue::infneg");
	}

	// ----------------------------------------------------------------------------
	// Mixed dd_cascade / double binary arithmetic in constexpr context
	// ----------------------------------------------------------------------------
	{
		constexpr dd_cascade a(2.0);
		constexpr auto cx_sum  = a + 3.0;     // 2 + 3 = 5
		constexpr auto cx_diff = a - 1.0;     // 2 - 1 = 1
		constexpr auto cx_prod = a * 4.0;     // 2 * 4 = 8
		constexpr auto cx_quot = a / 2.0;     // 2 / 2 = 1
		static_assert(cx_sum  == dd_cascade(5.0), "constexpr dd_cascade + double");
		static_assert(cx_diff == dd_cascade(1.0), "constexpr dd_cascade - double");
		static_assert(cx_prod == dd_cascade(8.0), "constexpr dd_cascade * double");
		static_assert(cx_quot == dd_cascade(1.0), "constexpr dd_cascade / double");

		constexpr auto cx_lhs_sum = 5.0 + dd_cascade(2.0);
		static_assert(cx_lhs_sum == dd_cascade(7.0), "constexpr double + dd_cascade");
	}

	// ----------------------------------------------------------------------------
	// Sign of negative zero in divide-by-zero (constexpr branch must use
	// IEEE-754 sign bit, not ordered comparison, to match runtime
	// std::copysign behavior -- per #727 lessons).
	// ----------------------------------------------------------------------------
	{
		// 1.0 / -0.0 should produce -infinity (sign of divisor flows through).
		constexpr auto cx_q = []() { dd_cascade t(1.0); t /= dd_cascade(-0.0); return t; }();
		static_assert(cx_q < dd_cascade(0.0), "constexpr 1.0 / -0.0 should be -inf");
		static_assert(cx_q.isinf(), "constexpr 1.0 / -0.0 should be inf");
	}

	// ----------------------------------------------------------------------------
	// IEEE-754 unordered comparison: nan >= x must be false (and similar).
	//
	// Tested at runtime instead of via static_assert because MSVC's constexpr
	// evaluator does not reliably implement IEEE-754 NaN semantics during
	// constant evaluation (see #727 commit 550790f2).  The contract is
	// enforced by the implementation; runtime checks here are sufficient.
	// ----------------------------------------------------------------------------
	{
		dd_cascade qnan(SpecificValue::qnan);
		dd_cascade one(1.0);
		if ( (qnan <  one)) { ++nrOfFailedTestCases; std::cerr << "FAIL: nan <  1.0 should be false\n"; }
		if ( (qnan >  one)) { ++nrOfFailedTestCases; std::cerr << "FAIL: nan >  1.0 should be false\n"; }
		if ( (qnan <= one)) { ++nrOfFailedTestCases; std::cerr << "FAIL: nan <= 1.0 should be false\n"; }
		if ( (qnan >= one)) { ++nrOfFailedTestCases; std::cerr << "FAIL: nan >= 1.0 should be false (>= must not be !operator<)\n"; }
		if ( (qnan == one)) { ++nrOfFailedTestCases; std::cerr << "FAIL: nan == 1.0 should be false\n"; }
		if (!(qnan != one)) { ++nrOfFailedTestCases; std::cerr << "FAIL: nan != 1.0 should be true\n"; }
		if ( (one  >= qnan)){ ++nrOfFailedTestCases; std::cerr << "FAIL: 1.0 >= nan should be false\n"; }
	}

	std::cout << "dd_cascade constexpr verification: "
	          << (nrOfFailedTestCases == 0 ? "PASS\n" : "FAIL\n");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const sw::universal::dd_cascade_arithmetic_exception& err) {
	std::cerr << "Uncaught dd_cascade arithmetic exception: " << err.what() << '\n';
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
