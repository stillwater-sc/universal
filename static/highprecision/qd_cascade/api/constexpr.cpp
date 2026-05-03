// constexpr.cpp: compile-time tests for qd_cascade (quad-double via floatcascade<4>)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure qd_cascade: throwing arithmetic exceptions disabled so divide-by-zero
// has defined constexpr-safe behavior (returns +/-inf or NaN encoding) rather
// than throwing, which would be ill-formed in a constant expression.
#define QD_CASCADE_THROW_ARITHMETIC_EXCEPTION 0

#include <iostream>
#include <iomanip>
#include <universal/number/qd_cascade/qd_cascade.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "qd_cascade constexpr verification";
	std::string test_tag    = "constexpr";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// qd_cascade is a (1, 11, 212) floating-point triple stored as a
	// floatcascade<4> -- an unevaluated sum of four IEEE-754 doubles.

	// ----------------------------------------------------------------------------
	// Native int construction (already constexpr -- smoke test only)
	// ----------------------------------------------------------------------------
	{
		constexpr qd_cascade a(0);
		constexpr qd_cascade b(1);
		constexpr qd_cascade c(-3);
		(void)a; (void)b; (void)c;
	}

	// ----------------------------------------------------------------------------
	// Acceptance form from issue #739:
	//   constexpr qd_cascade a(2.0), b(3.0); constexpr auto c = a + b;
	// (and analogous for *, -, /, +=, <)
	// ----------------------------------------------------------------------------
	{
		constexpr qd_cascade a(2.0);
		constexpr qd_cascade b(3.0);
		constexpr auto cx_sum = a + b;
		static_assert(cx_sum == qd_cascade(5.0), "issue #739 acceptance: a + b");
	}

	// ----------------------------------------------------------------------------
	// All four binary arithmetic operators in constexpr context
	// ----------------------------------------------------------------------------
	{
		constexpr qd_cascade a(4.5);
		constexpr qd_cascade b(1.5);
		constexpr auto cx_sum  = a + b;          // 4.5 + 1.5 = 6.0
		constexpr auto cx_diff = a - b;          // 4.5 - 1.5 = 3.0
		constexpr auto cx_prod = a * b;          // 4.5 * 1.5 = 6.75
		constexpr auto cx_quot = a / b;          // 4.5 / 1.5 = 3.0
		constexpr auto cx_neg  = -a;             // -4.5
		static_assert(cx_sum  == qd_cascade(6.0),  "constexpr +  failed");
		static_assert(cx_diff == qd_cascade(3.0),  "constexpr -  failed");
		static_assert(cx_prod == qd_cascade(6.75), "constexpr *  failed");
		static_assert(cx_quot == qd_cascade(3.0),  "constexpr /  failed");
		static_assert(cx_neg  == qd_cascade(-4.5), "constexpr unary - failed");

		// Compound assignment via lambda (constexpr lambdas are C++20)
		constexpr qd_cascade cx_addeq = []() { qd_cascade t(1.5); t += qd_cascade(3.0); return t; }();
		constexpr qd_cascade cx_subeq = []() { qd_cascade t(4.5); t -= qd_cascade(1.5); return t; }();
		constexpr qd_cascade cx_muleq = []() { qd_cascade t(1.5); t *= qd_cascade(3.0); return t; }();
		constexpr qd_cascade cx_diveq = []() { qd_cascade t(4.5); t /= qd_cascade(1.5); return t; }();
		static_assert(cx_addeq == qd_cascade(4.5), "constexpr += failed");
		static_assert(cx_subeq == qd_cascade(3.0), "constexpr -= failed");
		static_assert(cx_muleq == qd_cascade(4.5), "constexpr *= failed");
		static_assert(cx_diveq == qd_cascade(3.0), "constexpr /= failed");

		// Compound assignment with double rhs
		constexpr qd_cascade cx_addeqd = []() { qd_cascade t(1.5); t += 3.0; return t; }();
		constexpr qd_cascade cx_subeqd = []() { qd_cascade t(4.5); t -= 1.5; return t; }();
		constexpr qd_cascade cx_muleqd = []() { qd_cascade t(1.5); t *= 3.0; return t; }();
		constexpr qd_cascade cx_diveqd = []() { qd_cascade t(4.5); t /= 1.5; return t; }();
		static_assert(cx_addeqd == qd_cascade(4.5), "constexpr += double failed");
		static_assert(cx_subeqd == qd_cascade(3.0), "constexpr -= double failed");
		static_assert(cx_muleqd == qd_cascade(4.5), "constexpr *= double failed");
		static_assert(cx_diveqd == qd_cascade(3.0), "constexpr /= double failed");
	}

	// ----------------------------------------------------------------------------
	// Constexpr comparison (issue acceptance: <)
	// ----------------------------------------------------------------------------
	{
		constexpr qd_cascade a(1.5);
		constexpr qd_cascade b(3.0);
		static_assert(a < b,     "constexpr: qd_cascade(1.5) < qd_cascade(3.0)");
		static_assert(b > a,     "constexpr: qd_cascade(3.0) > qd_cascade(1.5)");
		static_assert(a <= b,    "constexpr: qd_cascade(1.5) <= qd_cascade(3.0)");
		static_assert(b >= a,    "constexpr: qd_cascade(3.0) >= qd_cascade(1.5)");
		static_assert(!(a == b), "constexpr: qd_cascade(1.5) != qd_cascade(3.0)");
		static_assert(a != b,    "constexpr: != operator");
		static_assert(a == a,    "constexpr: qd_cascade(1.5) == qd_cascade(1.5)");

		// qd_cascade vs double comparisons
		static_assert(a < 3.0,      "constexpr: qd_cascade(1.5) < 3.0");
		static_assert(3.0 > a,      "constexpr: 3.0 > qd_cascade(1.5)");
		static_assert(a == 1.5,     "constexpr: qd_cascade(1.5) == 1.5");
		static_assert(1.5 == a,     "constexpr: 1.5 == qd_cascade(1.5)");
	}

	// ----------------------------------------------------------------------------
	// Conversion-out: operator double() / int() / float() are now constexpr.
	// ----------------------------------------------------------------------------
	{
		constexpr qd_cascade a(0.5);
		constexpr double d = double(a);
		static_assert(d == 0.5, "constexpr operator double()");

		constexpr qd_cascade b(42.0);
		constexpr int n = int(b);
		static_assert(n == 42, "constexpr operator int()");

		constexpr qd_cascade c(2.5);
		constexpr float f = float(c);
		static_assert(f == 2.5f, "constexpr operator float()");

		// Large integer conversion: limb-separated truncation (lower 3 limbs
		// collapsed via two_sum chain) preserves precision above 2^53.
		constexpr qd_cascade large(9007199254740992.0, 1.0, 0.0, 0.0);
		constexpr unsigned long long u = static_cast<unsigned long long>(large);
		static_assert(u == 9007199254740993ULL,
			"constexpr conversion preserves precision above 2^53");

		// Negative large
		constexpr qd_cascade large_neg(-9007199254740992.0, -1.0, 0.0, 0.0);
		constexpr long long s = static_cast<long long>(large_neg);
		static_assert(s == -9007199254740993LL,
			"constexpr signed conversion preserves precision below -2^53");

		// Boundary case: hi is integer, mid subtracts a sub-ulp fraction.
		constexpr qd_cascade boundary_neg(101.0, -0x1.0p-50, 0.0, 0.0);
		constexpr int b_neg = static_cast<int>(boundary_neg);
		static_assert(b_neg == 100,
			"constexpr trunc handles fractional borrow across int boundary");

		// Boundary case: positive mid pushing hi+mid over an integer boundary.
		constexpr qd_cascade boundary_pos(2.5, 0.6, 0.0, 0.0);
		constexpr int b_pos = static_cast<int>(boundary_pos);
		static_assert(b_pos == 3,
			"constexpr trunc handles fractional carry across int boundary");

		// Negative qd_cascade to unsigned saturates to 0.
		constexpr qd_cascade negval(-5.0);
		constexpr unsigned int neg_u = static_cast<unsigned int>(negval);
		static_assert(neg_u == 0u,
			"constexpr unsigned conversion clamps negative qd_cascade to 0");

		// Out-of-range saturates to integer max.
		constexpr qd_cascade huge(1.0e20);
		constexpr int huge_i = static_cast<int>(huge);
		static_assert(huge_i == (std::numeric_limits<int>::max)(),
			"constexpr signed conversion saturates above int max");

		// hi at the rounded long long boundary, lower limbs bring it back in range.
		constexpr qd_cascade boundary_max(0x1.0p63, -2.0, 0.0, 0.0);
		constexpr long long b_max = static_cast<long long>(boundary_max);
		static_assert(b_max == 9223372036854775806LL,
			"constexpr signed conversion at upper boundary uses lo to stay in range");

		constexpr qd_cascade boundary_umax(0x1.0p64, -2.0, 0.0, 0.0);
		constexpr unsigned long long b_umax = static_cast<unsigned long long>(boundary_umax);
		static_assert(b_umax == 18446744073709551614ULL,
			"constexpr unsigned conversion at upper boundary uses lo to stay in range");

		// Defensive saturation for unnormalized qd_cascade values.
		constexpr qd_cascade unnorm_pos(0.0, 1.0e30, 0.0, 0.0);
		constexpr long long unp = static_cast<long long>(unnorm_pos);
		static_assert(unp == (std::numeric_limits<long long>::max)(),
			"constexpr conversion saturates on unnormalized lo > max");

		constexpr qd_cascade unnorm_neg(0.0, -1.0e30, 0.0, 0.0);
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
		constexpr qd_cascade zero(SpecificValue::zero);
		constexpr qd_cascade inf_pos(SpecificValue::infpos);
		constexpr qd_cascade inf_neg(SpecificValue::infneg);
		static_assert(zero == qd_cascade(0.0), "constexpr SpecificValue::zero");
		static_assert(inf_pos > qd_cascade(0.0), "constexpr SpecificValue::infpos");
		static_assert(inf_neg < qd_cascade(0.0), "constexpr SpecificValue::infneg");
	}

	// ----------------------------------------------------------------------------
	// Mixed qd_cascade / double binary arithmetic in constexpr context
	// ----------------------------------------------------------------------------
	{
		constexpr qd_cascade a(2.0);
		constexpr auto cx_sum  = a + 3.0;
		constexpr auto cx_diff = a - 1.0;
		constexpr auto cx_prod = a * 4.0;
		constexpr auto cx_quot = a / 2.0;
		static_assert(cx_sum  == qd_cascade(5.0), "constexpr qd_cascade + double");
		static_assert(cx_diff == qd_cascade(1.0), "constexpr qd_cascade - double");
		static_assert(cx_prod == qd_cascade(8.0), "constexpr qd_cascade * double");
		static_assert(cx_quot == qd_cascade(1.0), "constexpr qd_cascade / double");

		constexpr auto cx_lhs_sum = 5.0 + qd_cascade(2.0);
		static_assert(cx_lhs_sum == qd_cascade(7.0), "constexpr double + qd_cascade");
	}

	// ----------------------------------------------------------------------------
	// Sign of negative zero in divide-by-zero: -0.0 should produce -infinity.
	// ----------------------------------------------------------------------------
	{
		constexpr auto cx_q = []() { qd_cascade t(1.0); t /= qd_cascade(-0.0); return t; }();
		static_assert(cx_q < qd_cascade(0.0), "constexpr 1.0 / -0.0 should be -inf");
		static_assert(cx_q.isinf(), "constexpr 1.0 / -0.0 should be inf");
	}

	// ----------------------------------------------------------------------------
	// IEEE-754 unordered comparison: nan >= x must be false (and similar).
	// Tested at runtime instead of via static_assert (MSVC constexpr NaN bug).
	// ----------------------------------------------------------------------------
	{
		qd_cascade qnan(SpecificValue::qnan);
		qd_cascade one(1.0);
		if ( (qnan <  one)) { ++nrOfFailedTestCases; std::cerr << "FAIL: nan <  1.0 should be false\n"; }
		if ( (qnan >  one)) { ++nrOfFailedTestCases; std::cerr << "FAIL: nan >  1.0 should be false\n"; }
		if ( (qnan <= one)) { ++nrOfFailedTestCases; std::cerr << "FAIL: nan <= 1.0 should be false\n"; }
		if ( (qnan >= one)) { ++nrOfFailedTestCases; std::cerr << "FAIL: nan >= 1.0 should be false\n"; }
		if ( (qnan == one)) { ++nrOfFailedTestCases; std::cerr << "FAIL: nan == 1.0 should be false\n"; }
		if (!(qnan != one)) { ++nrOfFailedTestCases; std::cerr << "FAIL: nan != 1.0 should be true\n"; }
		if ( (one  >= qnan)){ ++nrOfFailedTestCases; std::cerr << "FAIL: 1.0 >= nan should be false\n"; }
	}

	std::cout << "qd_cascade constexpr verification: "
	          << (nrOfFailedTestCases == 0 ? "PASS\n" : "FAIL\n");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const sw::universal::qd_cascade_arithmetic_exception& err) {
	std::cerr << "Uncaught qd_cascade arithmetic exception: " << err.what() << '\n';
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
