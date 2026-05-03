// constexpr.cpp: compile-time tests for qd (quad-double precision)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure qd: throwing arithmetic exceptions disabled so divide-by-zero
// has defined constexpr-safe behavior (returns +/-inf or NaN encoding) rather
// than throwing, which would be ill-formed in a constant expression.
#define QUADDOUBLE_THROW_ARITHMETIC_EXCEPTION 0

#include <iostream>
#include <iomanip>
#include <universal/number/qd/qd.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "qd constexpr verification";
	std::string test_tag    = "constexpr";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// qd is a (1, 11, 212) floating-point triple stored as an unevaluated sum
	// of four IEEE-754 doubles.

	// ----------------------------------------------------------------------------
	// Native int construction (already constexpr -- smoke test only)
	// ----------------------------------------------------------------------------
	{
		constexpr qd a(0);
		constexpr qd b(1);
		constexpr qd c(-3);
		(void)a; (void)b; (void)c;
	}

	// ----------------------------------------------------------------------------
	// Acceptance form from issue #738:
	//   constexpr qd a(2.0), b(3.0); constexpr auto c = a + b;
	// (and analogous for *, -, /, +=, <)
	// ----------------------------------------------------------------------------
	{
		constexpr qd a(2.0);
		constexpr qd b(3.0);
		constexpr auto cx_sum = a + b;
		static_assert(cx_sum == qd(5.0), "issue #738 acceptance: a + b");
	}

	// ----------------------------------------------------------------------------
	// All four binary arithmetic operators in constexpr context
	// ----------------------------------------------------------------------------
	{
		constexpr qd a(4.5);
		constexpr qd b(1.5);
		constexpr auto cx_sum  = a + b;          // 4.5 + 1.5 = 6.0
		constexpr auto cx_diff = a - b;          // 4.5 - 1.5 = 3.0
		constexpr auto cx_prod = a * b;          // 4.5 * 1.5 = 6.75
		constexpr auto cx_quot = a / b;          // 4.5 / 1.5 = 3.0
		constexpr auto cx_neg  = -a;             // -4.5
		static_assert(cx_sum  == qd(6.0),  "constexpr +  failed");
		static_assert(cx_diff == qd(3.0),  "constexpr -  failed");
		static_assert(cx_prod == qd(6.75), "constexpr *  failed");
		static_assert(cx_quot == qd(3.0),  "constexpr /  failed");
		static_assert(cx_neg  == qd(-4.5), "constexpr unary - failed");

		// Compound assignment via lambda (constexpr lambdas are C++20)
		constexpr qd cx_addeq = []() { qd t(1.5); t += qd(3.0); return t; }();
		constexpr qd cx_subeq = []() { qd t(4.5); t -= qd(1.5); return t; }();
		constexpr qd cx_muleq = []() { qd t(1.5); t *= qd(3.0); return t; }();
		constexpr qd cx_diveq = []() { qd t(4.5); t /= qd(1.5); return t; }();
		static_assert(cx_addeq == qd(4.5), "constexpr += failed");
		static_assert(cx_subeq == qd(3.0), "constexpr -= failed");
		static_assert(cx_muleq == qd(4.5), "constexpr *= failed");
		static_assert(cx_diveq == qd(3.0), "constexpr /= failed");

		// Compound assignment with double rhs
		constexpr qd cx_addeqd = []() { qd t(1.5); t += 3.0; return t; }();
		constexpr qd cx_subeqd = []() { qd t(4.5); t -= 1.5; return t; }();
		constexpr qd cx_muleqd = []() { qd t(1.5); t *= 3.0; return t; }();
		constexpr qd cx_diveqd = []() { qd t(4.5); t /= 1.5; return t; }();
		static_assert(cx_addeqd == qd(4.5), "constexpr += double failed");
		static_assert(cx_subeqd == qd(3.0), "constexpr -= double failed");
		static_assert(cx_muleqd == qd(4.5), "constexpr *= double failed");
		static_assert(cx_diveqd == qd(3.0), "constexpr /= double failed");
	}

	// ----------------------------------------------------------------------------
	// Constexpr comparison (issue acceptance: <)
	// ----------------------------------------------------------------------------
	{
		constexpr qd a(1.5);
		constexpr qd b(3.0);
		static_assert(a < b,     "constexpr: qd(1.5) < qd(3.0)");
		static_assert(b > a,     "constexpr: qd(3.0) > qd(1.5)");
		static_assert(a <= b,    "constexpr: qd(1.5) <= qd(3.0)");
		static_assert(b >= a,    "constexpr: qd(3.0) >= qd(1.5)");
		static_assert(!(a == b), "constexpr: qd(1.5) != qd(3.0)");
		static_assert(a != b,    "constexpr: != operator");
		static_assert(a == a,    "constexpr: qd(1.5) == qd(1.5)");

		// qd vs double comparisons
		static_assert(a < 3.0,      "constexpr: qd(1.5) < 3.0");
		static_assert(3.0 > a,      "constexpr: 3.0 > qd(1.5)");
		static_assert(a == 1.5,     "constexpr: qd(1.5) == 1.5");
		static_assert(1.5 == a,     "constexpr: 1.5 == qd(1.5)");
	}

	// ----------------------------------------------------------------------------
	// Conversion-out: operator double() / int() / float() are now constexpr.
	// ----------------------------------------------------------------------------
	{
		constexpr qd a(0.5);
		constexpr double d = double(a);
		static_assert(d == 0.5, "constexpr operator double()");

		constexpr qd b(42.0);
		constexpr int n = int(b);
		static_assert(n == 42, "constexpr operator int()");

		constexpr qd c(2.5);
		constexpr float f = float(c);
		static_assert(f == 2.5f, "constexpr operator float()");

		// Large integer conversion: limb-separated truncation (lower 3 limbs
		// collapsed via two_sum chain) preserves precision above 2^53.
		constexpr qd large(9007199254740992.0, 1.0, 0.0, 0.0);
		constexpr unsigned long long u = static_cast<unsigned long long>(large);
		static_assert(u == 9007199254740993ULL,
			"constexpr conversion preserves precision above 2^53");

		// Negative large
		constexpr qd large_neg(-9007199254740992.0, -1.0, 0.0, 0.0);
		constexpr long long s = static_cast<long long>(large_neg);
		static_assert(s == -9007199254740993LL,
			"constexpr signed conversion preserves precision below -2^53");

		// Boundary case: hi is integer, mid subtracts a sub-ulp fraction.
		constexpr qd boundary_neg(101.0, -0x1.0p-50, 0.0, 0.0);
		constexpr int b_neg = static_cast<int>(boundary_neg);
		static_assert(b_neg == 100,
			"constexpr trunc handles fractional borrow across int boundary");

		// Boundary case: positive mid pushing hi+mid over an integer boundary.
		constexpr qd boundary_pos(2.5, 0.6, 0.0, 0.0);
		constexpr int b_pos = static_cast<int>(boundary_pos);
		static_assert(b_pos == 3,
			"constexpr trunc handles fractional carry across int boundary");

		// Negative qd to unsigned saturates to 0.
		constexpr qd negval(-5.0);
		constexpr unsigned int neg_u = static_cast<unsigned int>(negval);
		static_assert(neg_u == 0u,
			"constexpr unsigned conversion clamps negative qd to 0");

		// Out-of-range saturates to integer max.
		constexpr qd huge(1.0e20);
		constexpr int huge_i = static_cast<int>(huge);
		static_assert(huge_i == (std::numeric_limits<int>::max)(),
			"constexpr signed conversion saturates above int max");

		// hi at the rounded long long boundary, lower limbs bring it back in range.
		constexpr qd boundary_max(0x1.0p63, -2.0, 0.0, 0.0);
		constexpr long long b_max = static_cast<long long>(boundary_max);
		static_assert(b_max == 9223372036854775806LL,
			"constexpr signed conversion at upper boundary uses lo to stay in range");

		constexpr qd boundary_umax(0x1.0p64, -2.0, 0.0, 0.0);
		constexpr unsigned long long b_umax = static_cast<unsigned long long>(boundary_umax);
		static_assert(b_umax == 18446744073709551614ULL,
			"constexpr unsigned conversion at upper boundary uses lo to stay in range");

		// Defensive saturation for unnormalized qd values.
		constexpr qd unnorm_pos(0.0, 1.0e30, 0.0, 0.0);
		constexpr long long unp = static_cast<long long>(unnorm_pos);
		static_assert(unp == (std::numeric_limits<long long>::max)(),
			"constexpr conversion saturates on unnormalized lo > max");

		constexpr qd unnorm_neg(0.0, -1.0e30, 0.0, 0.0);
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
		constexpr qd zero(SpecificValue::zero);
		constexpr qd inf_pos(SpecificValue::infpos);
		constexpr qd inf_neg(SpecificValue::infneg);
		static_assert(zero == qd(0.0), "constexpr SpecificValue::zero");
		static_assert(inf_pos > qd(0.0), "constexpr SpecificValue::infpos");
		static_assert(inf_neg < qd(0.0), "constexpr SpecificValue::infneg");
	}

	// ----------------------------------------------------------------------------
	// Mixed qd / double binary arithmetic in constexpr context
	// ----------------------------------------------------------------------------
	{
		constexpr qd a(2.0);
		constexpr auto cx_sum  = a + 3.0;
		constexpr auto cx_diff = a - 1.0;
		constexpr auto cx_prod = a * 4.0;
		constexpr auto cx_quot = a / 2.0;
		static_assert(cx_sum  == qd(5.0), "constexpr qd + double");
		static_assert(cx_diff == qd(1.0), "constexpr qd - double");
		static_assert(cx_prod == qd(8.0), "constexpr qd * double");
		static_assert(cx_quot == qd(1.0), "constexpr qd / double");

		constexpr auto cx_lhs_sum = 5.0 + qd(2.0);
		static_assert(cx_lhs_sum == qd(7.0), "constexpr double + qd");
	}

	// ----------------------------------------------------------------------------
	// Sign of negative zero in divide-by-zero: -0.0 should produce -infinity.
	// ----------------------------------------------------------------------------
	{
		constexpr auto cx_q = []() { qd t(1.0); t /= qd(-0.0); return t; }();
		static_assert(cx_q < qd(0.0), "constexpr 1.0 / -0.0 should be -inf");
		static_assert(cx_q.isinf(), "constexpr 1.0 / -0.0 should be inf");
	}

	// ----------------------------------------------------------------------------
	// IEEE-754 unordered comparison: nan >= x must be false (and similar).
	// Tested at runtime instead of via static_assert (MSVC constexpr NaN bug).
	// ----------------------------------------------------------------------------
	{
		qd qnan(SpecificValue::qnan);
		qd one(1.0);
		if ( (qnan <  one)) { ++nrOfFailedTestCases; std::cerr << "FAIL: nan <  1.0 should be false\n"; }
		if ( (qnan >  one)) { ++nrOfFailedTestCases; std::cerr << "FAIL: nan >  1.0 should be false\n"; }
		if ( (qnan <= one)) { ++nrOfFailedTestCases; std::cerr << "FAIL: nan <= 1.0 should be false\n"; }
		if ( (qnan >= one)) { ++nrOfFailedTestCases; std::cerr << "FAIL: nan >= 1.0 should be false\n"; }
		if ( (qnan == one)) { ++nrOfFailedTestCases; std::cerr << "FAIL: nan == 1.0 should be false\n"; }
		if (!(qnan != one)) { ++nrOfFailedTestCases; std::cerr << "FAIL: nan != 1.0 should be true\n"; }
		if ( (one  >= qnan)){ ++nrOfFailedTestCases; std::cerr << "FAIL: 1.0 >= nan should be false\n"; }
	}

	std::cout << "qd constexpr verification: "
	          << (nrOfFailedTestCases == 0 ? "PASS\n" : "FAIL\n");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const sw::universal::qd_arithmetic_exception& err) {
	std::cerr << "Uncaught qd arithmetic exception: " << err.what() << '\n';
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
