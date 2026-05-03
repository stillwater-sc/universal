// constexpr.cpp: compile-time tests for hfloat (IBM System/360 hexadecimal FP)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure hfloat: throwing arithmetic exceptions disabled so divide-by-zero
// has defined constexpr-safe behavior (saturates to zero per HFP semantics --
// IBM HFP has no NaN/Inf -- rather than throwing, which would be ill-formed
// in a constant expression.
#define HFLOAT_THROW_ARITHMETIC_EXCEPTION 0

#include <iostream>
#include <iomanip>
#include <universal/number/hfloat/hfloat.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "hfloat constexpr verification";
	std::string test_tag    = "constexpr";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// hfloat_short = hfloat<6, 7> = 32-bit IBM HFP short.
	// hfloat_long  = hfloat<14, 7> = 64-bit IBM HFP long.
	// IBM HFP has NO NaN, NO infinity, NO subnormals; truncation rounding
	// only.  The constexpr suite focuses on these invariants in addition
	// to the standard arithmetic / comparison contract.
	using HShort = hfloat_short;
	using HLong  = hfloat_long;

	// ----------------------------------------------------------------------------
	// Native int construction (smoke test)
	// ----------------------------------------------------------------------------
	{
		constexpr HShort a(0);
		constexpr HShort b(1);
		constexpr HShort c(-3);
		(void)a; (void)b; (void)c;
	}

	// ----------------------------------------------------------------------------
	// Acceptance form from issue #732:
	//   constexpr hfloat<...> a(2.0), b(3.0); constexpr auto c = a + b;
	// (and analogous for *, -, /, +=, <)
	// ----------------------------------------------------------------------------
	{
		constexpr HShort a(2.0);
		constexpr HShort b(3.0);
		constexpr auto cx_sum = a + b;            // 2 + 3 = 5
		static_assert((cx_sum - HShort(5.0)).iszero(), "issue #732 acceptance: a + b");
	}

	// ----------------------------------------------------------------------------
	// All four binary arithmetic operators in constexpr context
	//
	// Equality is asserted via subtraction + iszero() so that wobbling-precision
	// representations (IBM HFP truncation can leave 0-3 leading zero bits in
	// the leading hex digit, producing different bit patterns for equal values)
	// don't cause spurious failures.
	// ----------------------------------------------------------------------------
	{
		constexpr HShort a(4.5);
		constexpr HShort b(1.5);
		constexpr auto cx_sum  = a + b;          // 4.5 + 1.5 = 6.0
		constexpr auto cx_diff = a - b;          // 4.5 - 1.5 = 3.0
		constexpr auto cx_prod = a * b;          // 4.5 * 1.5 = 6.75
		constexpr auto cx_quot = a / b;          // 4.5 / 1.5 = 3.0
		constexpr auto cx_neg  = -a;             // -4.5
		static_assert((cx_sum  - HShort(6.0)).iszero(),  "constexpr +  failed");
		static_assert((cx_diff - HShort(3.0)).iszero(),  "constexpr -  failed");
		static_assert((cx_prod - HShort(6.75)).iszero(), "constexpr *  failed");
		static_assert((cx_quot - HShort(3.0)).iszero(),  "constexpr /  failed");
		static_assert((cx_neg  - HShort(-4.5)).iszero(), "constexpr unary - failed");

		// Compound assignment via lambda (constexpr lambdas are C++20)
		constexpr HShort cx_addeq = []() { HShort t(1.5); t += HShort(3.0); return t; }();
		constexpr HShort cx_subeq = []() { HShort t(4.5); t -= HShort(1.5); return t; }();
		constexpr HShort cx_muleq = []() { HShort t(1.5); t *= HShort(3.0); return t; }();
		constexpr HShort cx_diveq = []() { HShort t(4.5); t /= HShort(1.5); return t; }();
		static_assert((cx_addeq - HShort(4.5)).iszero(), "constexpr += failed");
		static_assert((cx_subeq - HShort(3.0)).iszero(), "constexpr -= failed");
		static_assert((cx_muleq - HShort(4.5)).iszero(), "constexpr *= failed");
		static_assert((cx_diveq - HShort(3.0)).iszero(), "constexpr /= failed");
	}

	// ----------------------------------------------------------------------------
	// Constexpr comparison (issue acceptance: <)
	// ----------------------------------------------------------------------------
	{
		constexpr HShort a(1.5);
		constexpr HShort b(3.0);
		static_assert(a < b,     "constexpr: HShort(1.5) < HShort(3.0)");
		static_assert(b > a,     "constexpr: HShort(3.0) > HShort(1.5)");
		static_assert(a <= b,    "constexpr: HShort(1.5) <= HShort(3.0)");
		static_assert(b >= a,    "constexpr: HShort(3.0) >= HShort(1.5)");
		static_assert(!(a == b), "constexpr: HShort(1.5) != HShort(3.0)");
		static_assert(a != b,    "constexpr: != operator");
		static_assert(a == a,    "constexpr: HShort(1.5) == HShort(1.5)");
	}

	// ----------------------------------------------------------------------------
	// Conversion-out: operator double() / float() are now constexpr
	// ----------------------------------------------------------------------------
	{
		constexpr HShort a(0.5);
		constexpr double d = double(a);
		static_assert(d == 0.5, "constexpr operator double()");

		constexpr HShort b(2.5);
		constexpr float f = float(b);
		static_assert(f == 2.5f, "constexpr operator float()");
	}

	// ----------------------------------------------------------------------------
	// Default value-initialization is a constexpr-safe representation of zero.
	// (Trivial default ctor leaves storage indeterminate; T{} value-initializes
	// the array aggregate to zero, which IS legal in a constant expression.)
	// ----------------------------------------------------------------------------
	{
		constexpr HShort cx_zero{};
		static_assert(cx_zero.iszero(), "constexpr HShort{} is zero");

		// ++0 -> next representable value; --0 -> previous representable value
		constexpr HShort cx_inc_zero = []() { HShort t{}; ++t; return t; }();
		constexpr HShort cx_dec_zero = []() { HShort t{}; --t; return t; }();
		static_assert(cx_inc_zero > HShort(0.0), "constexpr ++0 > 0");
		static_assert(cx_dec_zero < HShort(0.0), "constexpr --0 < 0");
	}

	// ----------------------------------------------------------------------------
	// SpecificValue construction.  IBM HFP has no NaN, no infinity:
	//   infpos -> maxpos (saturation), infneg -> maxneg
	//   qnan   -> zero,                snan   -> zero
	// These mappings are part of the hfloat contract and must hold at
	// constant-evaluation time as well as runtime.
	// ----------------------------------------------------------------------------
	{
		constexpr HShort zero(SpecificValue::zero);
		constexpr HShort maxp(SpecificValue::maxpos);
		constexpr HShort minp(SpecificValue::minpos);
		constexpr HShort minn(SpecificValue::minneg);
		constexpr HShort maxn(SpecificValue::maxneg);
		constexpr HShort infp(SpecificValue::infpos);
		constexpr HShort infn(SpecificValue::infneg);
		constexpr HShort qn(SpecificValue::qnan);
		static_assert(zero == HShort(0.0), "constexpr SpecificValue::zero");
		static_assert(maxp > HShort(0.0),  "constexpr SpecificValue::maxpos > 0");
		static_assert(minp > HShort(0.0),  "constexpr SpecificValue::minpos > 0");
		static_assert(minn < HShort(0.0),  "constexpr SpecificValue::minneg < 0");
		static_assert(maxn < HShort(0.0),  "constexpr SpecificValue::maxneg < 0");
		static_assert(maxp > minp,         "constexpr maxpos > minpos");

		// HFP saturation of infinity: infpos == maxpos, infneg == maxneg
		static_assert(infp == maxp,     "constexpr SpecificValue::infpos saturates to maxpos");
		static_assert(infn == maxn,     "constexpr SpecificValue::infneg saturates to maxneg");

		// HFP no-NaN: qnan/snan map to zero
		static_assert(qn.iszero(),      "constexpr SpecificValue::qnan maps to zero in HFP");
	}

	// ----------------------------------------------------------------------------
	// HFP invariants: isinf() and isnan() always return false (no infinity,
	// no NaN -- core IBM System/360 HFP property).
	// ----------------------------------------------------------------------------
	{
		constexpr HShort zero(0.0);
		constexpr HShort one(1.0);
		constexpr HShort maxp(SpecificValue::maxpos);
		static_assert(!zero.isinf(),  "constexpr: HFP zero is not inf");
		static_assert(!zero.isnan(),  "constexpr: HFP zero is not nan");
		static_assert(!one.isinf(),   "constexpr: HFP one is not inf");
		static_assert(!one.isnan(),   "constexpr: HFP one is not nan");
		static_assert(!maxp.isinf(),  "constexpr: HFP maxpos is not inf (saturation)");
		static_assert(!maxp.isnan(),  "constexpr: HFP maxpos is not nan");
	}

	// ----------------------------------------------------------------------------
	// Special-value arithmetic in constexpr
	//   HFLOAT_THROW_ARITHMETIC_EXCEPTION = 0 -> divide-by-zero saturates to zero
	//   (no NaN/inf available; this is the closest constexpr-safe behavior).
	// ----------------------------------------------------------------------------
	{
		constexpr HShort zero(0.0);
		constexpr HShort one(1.0);
		// 1/0 -> zero (HFP has no infinity; constexpr divide-by-zero contract)
		constexpr auto cx_div_zero = one / zero;
		static_assert(cx_div_zero.iszero(), "constexpr 1/0 -> zero (HFP saturation)");

		// 0/0 -> zero (HFP has no NaN)
		constexpr auto cx_zero_div_zero = zero / zero;
		static_assert(cx_zero_div_zero.iszero(), "constexpr 0/0 -> zero (HFP saturation)");
	}

	// ----------------------------------------------------------------------------
	// hfloat_long smoke (wider exponent range, 56 fraction bits)
	// ----------------------------------------------------------------------------
	{
		constexpr HLong a(2.0);
		constexpr HLong b(3.0);
		constexpr auto cx_sum = a + b;
		static_assert((cx_sum - HLong(5.0)).iszero(), "constexpr hfloat_long +");

		constexpr auto cx_prod = a * b;
		static_assert((cx_prod - HLong(6.0)).iszero(), "constexpr hfloat_long *");
	}

	// ----------------------------------------------------------------------------
	// abs / fabs free functions are constexpr
	// ----------------------------------------------------------------------------
	{
		constexpr HShort negv(-3.5);
		constexpr HShort posv = abs(negv);
		static_assert(posv == HShort(3.5), "constexpr abs(-3.5) == 3.5");
		constexpr HShort posv2 = fabs(negv);
		static_assert(posv2 == HShort(3.5), "constexpr fabs(-3.5) == 3.5");
	}

	// ----------------------------------------------------------------------------
	// Integer construction precision (CodeRabbit follow-up).
	// hfloat_long has a 56-bit fraction so values above 2^53 (the IEEE 754
	// double mantissa width) must be packed directly from the integer
	// without round-tripping through double.
	//
	// 9007199254740992  = 2^53      (exactly representable in double)
	// 9007199254740993  = 2^53 + 1  (NOT representable in double; rounds
	//                                to 2^53 under any double round-trip)
	//
	// Both values fit in hfloat_long's 56-bit fraction, so direct integer
	// packing must keep them distinct.  The previous double round-trip
	// implementation failed this contract -- the new pack_uint64() path
	// satisfies it.  We verify via tuple-based comparison (which is itself
	// precision-preserving) rather than via operator double() (which would
	// lose the very bit we're checking).
	// ----------------------------------------------------------------------------
	{
		constexpr HLong a(9007199254740992LL);   // 2^53
		constexpr HLong b(9007199254740993LL);   // 2^53 + 1
		static_assert(a != b, "constexpr: hfloat_long preserves 2^53 + 1 distinct from 2^53");
		static_assert(a < b,  "constexpr: hfloat_long 2^53 < 2^53 + 1 (tuple compare)");
		static_assert(b > a,  "constexpr: hfloat_long 2^53 + 1 > 2^53");

		// Same witness via the unsigned conversion path.
		constexpr HLong au(9007199254740992ULL);
		constexpr HLong bu(9007199254740993ULL);
		static_assert(au != bu, "constexpr: hfloat_long unsigned preserves 2^53 + 1");

		// INT64_MIN: |v| computed via -(v+1)+1 identity, no overflow.
		constexpr long long llmin = (-9223372036854775807LL) - 1LL;  // INT64_MIN
		constexpr HLong smin(llmin);
		static_assert(smin.sign(), "constexpr: hfloat_long INT64_MIN is negative");
		static_assert(!smin.iszero(), "constexpr: hfloat_long INT64_MIN is non-zero");
	}

	std::cout << "hfloat constexpr verification: "
	          << (nrOfFailedTestCases == 0 ? "PASS\n" : "FAIL\n");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const sw::universal::hfloat_arithmetic_exception& err) {
	std::cerr << "Uncaught hfloat arithmetic exception: " << err.what() << '\n';
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
