// constexpr.cpp: compile-time tests for dfixpnt (decimal fixed-point)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure dfixpnt: throwing arithmetic exceptions disabled so divide-by-zero
// has defined constexpr-safe behavior (silently returns *this) rather than
// throwing, which would be ill-formed in a constant expression.
#define DFIXPNT_THROW_ARITHMETIC_EXCEPTION 0
#define BLOCKDECIMAL_THROW_ARITHMETIC_EXCEPTION 0

#include <iostream>
#include <iomanip>
#include <universal/number/dfixpnt/dfixpnt.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "dfixpnt constexpr verification";
	std::string test_tag    = "constexpr";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// dfixpnt<8,3> = 8 total decimal digits, 3 fractional -> range +/- 99999.999

	using D = dfixpnt<8, 3>;

	// ----------------------------------------------------------------------------
	// Native int construction (already constexpr -- smoke test only)
	// ----------------------------------------------------------------------------
	{
		constexpr D a(0);
		constexpr D b(1);
		constexpr D c(-3);
		(void)a; (void)b; (void)c;
	}

	// ----------------------------------------------------------------------------
	// Acceptance form from issue #729:
	//   constexpr dfixpnt<...> a(2.0), b(3.0); constexpr auto c = a + b;
	// (and analogous for *, -, /, +=, <)
	// ----------------------------------------------------------------------------
	{
		constexpr D a(2.0);
		constexpr D b(3.0);
		constexpr auto cx_sum = a + b;  // 2 + 3 = 5
		static_assert(cx_sum == D(5.0), "issue #729 acceptance: a + b");
	}

	// ----------------------------------------------------------------------------
	// All four binary arithmetic operators in constexpr context
	// ----------------------------------------------------------------------------
	{
		constexpr D a(4.5);
		constexpr D b(1.5);
		constexpr auto cx_sum  = a + b;          // 4.5 + 1.5 = 6.0
		constexpr auto cx_diff = a - b;          // 4.5 - 1.5 = 3.0
		constexpr auto cx_prod = a * b;          // 4.5 * 1.5 = 6.75
		constexpr auto cx_quot = a / b;          // 4.5 / 1.5 = 3.0
		constexpr auto cx_neg  = -a;             // -4.5
		static_assert(cx_sum  == D(6.0),  "constexpr +  failed");
		static_assert(cx_diff == D(3.0),  "constexpr -  failed");
		static_assert(cx_prod == D(6.75), "constexpr *  failed");
		static_assert(cx_quot == D(3.0),  "constexpr /  failed");
		static_assert(cx_neg  == D(-4.5), "constexpr unary - failed");

		// Compound assignment via lambda (constexpr lambdas are C++20)
		constexpr D cx_addeq = []() { D t(1.5); t += D(3.0); return t; }();
		constexpr D cx_subeq = []() { D t(4.5); t -= D(1.5); return t; }();
		constexpr D cx_muleq = []() { D t(1.5); t *= D(3.0); return t; }();
		constexpr D cx_diveq = []() { D t(4.5); t /= D(1.5); return t; }();
		static_assert(cx_addeq == D(4.5), "constexpr += failed");
		static_assert(cx_subeq == D(3.0), "constexpr -= failed");
		static_assert(cx_muleq == D(4.5), "constexpr *= failed");
		static_assert(cx_diveq == D(3.0), "constexpr /= failed");
	}

	// ----------------------------------------------------------------------------
	// Constexpr comparison (issue acceptance: <)
	// ----------------------------------------------------------------------------
	{
		constexpr D a(1.5);
		constexpr D b(3.0);
		static_assert(a < b,     "constexpr: D(1.5) < D(3.0)");
		static_assert(b > a,     "constexpr: D(3.0) > D(1.5)");
		static_assert(a <= b,    "constexpr: D(1.5) <= D(3.0)");
		static_assert(b >= a,    "constexpr: D(3.0) >= D(1.5)");
		static_assert(!(a == b), "constexpr: D(1.5) != D(3.0)");
		static_assert(a != b,    "constexpr: != operator");
		static_assert(a == a,    "constexpr: D(1.5) == D(1.5)");
	}

	// ----------------------------------------------------------------------------
	// Conversion-out: operator double() / int() are now constexpr
	// ----------------------------------------------------------------------------
	{
		constexpr D a(0.5);
		constexpr double d = double(a);
		static_assert(d == 0.5, "constexpr operator double()");

		constexpr D b(42.0);
		constexpr int n = int(b);
		static_assert(n == 42, "constexpr operator int()");

		constexpr D c(2.5);
		constexpr float f = float(c);
		static_assert(f == 2.5f, "constexpr operator float()");

		// Negative int truncates toward zero
		constexpr D neg(-3.7);
		constexpr int neg_i = int(neg);
		static_assert(neg_i == -3, "constexpr operator int() truncates toward zero (negative)");
	}

	// ----------------------------------------------------------------------------
	// Increment / decrement (prefix and postfix)
	// ----------------------------------------------------------------------------
	{
		constexpr D cx_inc = []() { D t(2.5); ++t; return t; }();
		constexpr D cx_dec = []() { D t(2.5); --t; return t; }();
		static_assert(cx_inc == D(3.5), "constexpr ++ failed");
		static_assert(cx_dec == D(1.5), "constexpr -- failed");

		// Postfix returns the pre-increment value but mutates the operand.
		constexpr D cx_postinc_before = []() { D t(2.5); D old = t++; return old; }();
		constexpr D cx_postinc_after  = []() { D t(2.5); t++; return t; }();
		constexpr D cx_postdec_before = []() { D t(2.5); D old = t--; return old; }();
		constexpr D cx_postdec_after  = []() { D t(2.5); t--; return t; }();
		static_assert(cx_postinc_before == D(2.5), "constexpr postfix ++ return failed");
		static_assert(cx_postinc_after  == D(3.5), "constexpr postfix ++ state failed");
		static_assert(cx_postdec_before == D(2.5), "constexpr postfix -- return failed");
		static_assert(cx_postdec_after  == D(1.5), "constexpr postfix -- state failed");
	}

	// ----------------------------------------------------------------------------
	// SpecificValue construction
	// ----------------------------------------------------------------------------
	{
		constexpr D zero(SpecificValue::zero);
		constexpr D maxp(SpecificValue::maxpos);
		constexpr D minp(SpecificValue::minpos);
		constexpr D minn(SpecificValue::minneg);
		constexpr D maxn(SpecificValue::maxneg);
		static_assert(zero == D(0.0), "constexpr SpecificValue::zero");
		static_assert(maxp > D(0.0),  "constexpr SpecificValue::maxpos > 0");
		static_assert(minp > D(0.0),  "constexpr SpecificValue::minpos > 0");
		static_assert(minn < D(0.0),  "constexpr SpecificValue::minneg < 0");
		static_assert(maxn < D(0.0),  "constexpr SpecificValue::maxneg < 0");
		static_assert(maxp > minp,    "constexpr maxpos > minpos");
	}

	// ----------------------------------------------------------------------------
	// Pure-fractional dfixpnt<N, N> (idigits == 0): ++/-- must not write OOB.
	// For these types ++/-- are no-ops since 1 is not representable.
	// (Per CodeRabbit critical finding on PR #803.)
	// ----------------------------------------------------------------------------
	{
		using DF = dfixpnt<3, 3>;  // values in (-0.999, 0.999); idigits == 0
		constexpr DF half(0.5);
		constexpr DF cx_inc = []() { DF t(0.5); ++t; return t; }();
		constexpr DF cx_dec = []() { DF t(0.5); --t; return t; }();
		// ++/-- are no-ops for fully-fractional dfixpnt -- the alternative
		// would write past the last digit (UB).
		static_assert(cx_inc == half, "constexpr ++ is no-op for dfixpnt<N,N>");
		static_assert(cx_dec == half, "constexpr -- is no-op for dfixpnt<N,N>");
	}

	// ----------------------------------------------------------------------------
	// blockdecimal unsigned conversion preserves the full 64-bit range.
	// Pre-fix would clamp via to_long_long() at LLONG_MAX, losing the upper
	// half of the unsigned range.  (Per CodeRabbit major finding on PR #803.)
	// ----------------------------------------------------------------------------
	{
		// Use blockdecimal<19, BID> directly to exercise the unsigned conversion.
		// 19 digits = max for BID (uint64_t fits 9999999999999999999 < 2^64).
		using BD = blockdecimal<19, DecimalEncoding::BID>;
		// 9999999999999999999 > LLONG_MAX (9223372036854775807) but fits in uint64_t.
		constexpr unsigned long long large = 9999999999999999999ULL;
		constexpr BD a(large);
		constexpr unsigned long long round_trip = static_cast<unsigned long long>(a);
		static_assert(round_trip == large,
			"blockdecimal -> unsigned long long preserves upper 64-bit range");
	}

	// ----------------------------------------------------------------------------
	// Wide instantiations (ndigits >= 20) -- issue #804.
	//
	// Pre-fix, both to_int64() (LSD-first scale *= 10 overflows long long for
	// idigits >= 19) and operator=(double) (uint64_t materialization overflows
	// when scaled exceeds UINT64_MAX) were UB.  In a constant expression that
	// UB hard-fails compilation, so simply instantiating these constexpr
	// constructions is the test.
	// ----------------------------------------------------------------------------
	{
		// dfixpnt<25, 5>: 25 digits total, 5 fractional, 20 integer digits.
		// idigits = 20 -> to_int64()'s scale would reach 10^19 (overflows long long).
		// Pre-fix this would hard-fail in constexpr context.
		using D25 = dfixpnt<25, 5>;
		constexpr D25 small(42.0);
		constexpr long long round_trip = static_cast<long long>(small);
		static_assert(round_trip == 42LL, "constexpr: dfixpnt<25,5>(42) -> 42");

		// Negative path also goes through to_int64()'s clamp logic.
		constexpr D25 neg(-7.0);
		constexpr long long neg_round = static_cast<long long>(neg);
		static_assert(neg_round == -7LL, "constexpr: dfixpnt<25,5>(-7) -> -7");

		// Conversion-out clamping: a value larger than LLONG_MAX in dfixpnt
		// must clamp to LLONG_MAX rather than producing garbage / UB.
		// dfixpnt<25,5>'s max value (10^20 - 1, in integer digits) far exceeds
		// LLONG_MAX (~9.22e18).  Construct a value beyond LLONG_MAX via
		// repeated addition to verify clamping behavior.
		// Note: the maxpos SpecificValue construction sets all digits to 9,
		// giving the largest representable magnitude.
		constexpr D25 maxp(SpecificValue::maxpos);
		constexpr long long clamped = static_cast<long long>(maxp);
		static_assert(clamped == std::numeric_limits<long long>::max(),
			"constexpr: dfixpnt<25,5>(maxpos) clamps to LLONG_MAX");

		constexpr D25 maxn(SpecificValue::maxneg);
		constexpr long long clamped_neg = static_cast<long long>(maxn);
		static_assert(clamped_neg == std::numeric_limits<long long>::min(),
			"constexpr: dfixpnt<25,5>(maxneg) clamps to LLONG_MIN");
	}
	{
		// dfixpnt<20, 5>: ndigits = 20 -> max_magnitude = 10^20 exceeds UINT64_MAX.
		// Pre-fix the static_cast<uint64_t>(scaled) was UB for any scaled
		// approaching the type's range.  After the fix, FP-domain digit
		// extraction handles the wide range without an out-of-range cast.
		using D20 = dfixpnt<20, 5>;
		constexpr D20 small(123.5);
		constexpr long long lo = static_cast<long long>(small);
		static_assert(lo == 123LL, "constexpr: dfixpnt<20,5>(123.5) -> 123");

		// Round-trip the largest value that still fits in dfixpnt<20,5>'s
		// 15 integer digits.  Past this we saturate -- not a defect, just
		// the type's representational limit.
		constexpr D20 big(123456789012345.0);
		constexpr long long big_lo = static_cast<long long>(big);
		static_assert(big_lo == 123456789012345LL,
			"constexpr: dfixpnt<20,5> preserves 15-digit integer through wide path");

		// Above the 15-digit integer range, dfixpnt<20,5> saturates to maxpos.
		// Pre-fix the cast-to-uint64 of an out-of-range double was UB.
		constexpr D20 huge(1.0e15);  // exceeds 10^15 - 1, the max for D20
		constexpr long long huge_lo = static_cast<long long>(huge);
		static_assert(huge_lo == 999999999999999LL,
			"constexpr: dfixpnt<20,5>(>=1e15) saturates to maxpos magnitude");
	}
	{
		// dfixpnt<25,5> can hold the full 1e15 value (15 integer digits +
		// room to spare; 20 integer digits available).
		using D25_2 = dfixpnt<25, 5>;
		constexpr D25_2 big(1.0e15);
		constexpr long long big_lo = static_cast<long long>(big);
		static_assert(big_lo == 1000000000000000LL,
			"constexpr: dfixpnt<25,5>(1e15) round-trips through wide to_int64");
	}

	// ----------------------------------------------------------------------------
	// Encoding variants: BCD (default), BID, DPD
	// ----------------------------------------------------------------------------
	{
		using D_BCD = dfixpnt<6, 2, DecimalEncoding::BCD>;
		using D_BID = dfixpnt<6, 2, DecimalEncoding::BID>;
		using D_DPD = dfixpnt<6, 2, DecimalEncoding::DPD>;

		constexpr D_BCD a_bcd(12.5);
		constexpr D_BCD b_bcd(2.5);
		constexpr auto sum_bcd = a_bcd + b_bcd;
		static_assert(sum_bcd == D_BCD(15.0), "constexpr BCD addition");

		constexpr D_BID a_bid(12.5);
		constexpr D_BID b_bid(2.5);
		constexpr auto sum_bid = a_bid + b_bid;
		static_assert(sum_bid == D_BID(15.0), "constexpr BID addition");

		constexpr D_DPD a_dpd(12.5);
		constexpr D_DPD b_dpd(2.5);
		constexpr auto sum_dpd = a_dpd + b_dpd;
		static_assert(sum_dpd == D_DPD(15.0), "constexpr DPD addition");
	}

	std::cout << "dfixpnt constexpr verification: "
	          << (nrOfFailedTestCases == 0 ? "PASS\n" : "FAIL\n");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const sw::universal::dfixpnt_arithmetic_exception& err) {
	std::cerr << "Uncaught dfixpnt arithmetic exception: " << err.what() << '\n';
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
