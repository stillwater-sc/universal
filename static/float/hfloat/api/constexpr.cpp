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

	// hfp32 = hfloat<6, 7> = 32-bit IBM HFP short.
	// hfp64  = hfloat<14, 7> = 64-bit IBM HFP long.
	// IBM HFP has NO NaN, NO infinity, NO subnormals; truncation rounding
	// only.  The constexpr suite focuses on these invariants in addition
	// to the standard arithmetic / comparison contract.

	// ----------------------------------------------------------------------------
	// Native int construction (smoke test)
	// ----------------------------------------------------------------------------
	{
		constexpr hfp32 a(0);
		constexpr hfp32 b(1);
		constexpr hfp32 c(-3);
		(void)a; (void)b; (void)c;
	}

	// ----------------------------------------------------------------------------
	// Acceptance form from issue #732:
	//   constexpr hfloat<...> a(2.0), b(3.0); constexpr auto c = a + b;
	// (and analogous for *, -, /, +=, <)
	// ----------------------------------------------------------------------------
	{
		constexpr hfp32 a(2.0);
		constexpr hfp32 b(3.0);
		constexpr auto cx_sum = a + b;            // 2 + 3 = 5
		static_assert((cx_sum - hfp32(5.0)).iszero(), "issue #732 acceptance: a + b");
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
		constexpr hfp32 a(4.5);
		constexpr hfp32 b(1.5);
		constexpr auto cx_sum  = a + b;          // 4.5 + 1.5 = 6.0
		constexpr auto cx_diff = a - b;          // 4.5 - 1.5 = 3.0
		constexpr auto cx_prod = a * b;          // 4.5 * 1.5 = 6.75
		constexpr auto cx_quot = a / b;          // 4.5 / 1.5 = 3.0
		constexpr auto cx_neg  = -a;             // -4.5
		static_assert((cx_sum  - hfp32(6.0)).iszero(),  "constexpr +  failed");
		static_assert((cx_diff - hfp32(3.0)).iszero(),  "constexpr -  failed");
		static_assert((cx_prod - hfp32(6.75)).iszero(), "constexpr *  failed");
		static_assert((cx_quot - hfp32(3.0)).iszero(),  "constexpr /  failed");
		static_assert((cx_neg  - hfp32(-4.5)).iszero(), "constexpr unary - failed");

		// Compound assignment via lambda (constexpr lambdas are C++20)
		constexpr hfp32 cx_addeq = []() { hfp32 t(1.5); t += hfp32(3.0); return t; }();
		constexpr hfp32 cx_subeq = []() { hfp32 t(4.5); t -= hfp32(1.5); return t; }();
		constexpr hfp32 cx_muleq = []() { hfp32 t(1.5); t *= hfp32(3.0); return t; }();
		constexpr hfp32 cx_diveq = []() { hfp32 t(4.5); t /= hfp32(1.5); return t; }();
		static_assert((cx_addeq - hfp32(4.5)).iszero(), "constexpr += failed");
		static_assert((cx_subeq - hfp32(3.0)).iszero(), "constexpr -= failed");
		static_assert((cx_muleq - hfp32(4.5)).iszero(), "constexpr *= failed");
		static_assert((cx_diveq - hfp32(3.0)).iszero(), "constexpr /= failed");
	}

	// ----------------------------------------------------------------------------
	// Constexpr comparison (issue acceptance: <)
	// ----------------------------------------------------------------------------
	{
		constexpr hfp32 a(1.5);
		constexpr hfp32 b(3.0);
		static_assert(a < b,     "constexpr: hfp32(1.5) < hfp32(3.0)");
		static_assert(b > a,     "constexpr: hfp32(3.0) > hfp32(1.5)");
		static_assert(a <= b,    "constexpr: hfp32(1.5) <= hfp32(3.0)");
		static_assert(b >= a,    "constexpr: hfp32(3.0) >= hfp32(1.5)");
		static_assert(!(a == b), "constexpr: hfp32(1.5) != hfp32(3.0)");
		static_assert(a != b,    "constexpr: != operator");
		static_assert(a == a,    "constexpr: hfp32(1.5) == hfp32(1.5)");
	}

	// ----------------------------------------------------------------------------
	// Conversion-out: operator double() / float() are now constexpr
	// ----------------------------------------------------------------------------
	{
		constexpr hfp32 a(0.5);
		constexpr double d = double(a);
		static_assert(d == 0.5, "constexpr operator double()");

		constexpr hfp32 b(2.5);
		constexpr float f = float(b);
		static_assert(f == 2.5f, "constexpr operator float()");
	}

	// ----------------------------------------------------------------------------
	// Default value-initialization is a constexpr-safe representation of zero.
	// (Trivial default ctor leaves storage indeterminate; T{} value-initializes
	// the array aggregate to zero, which IS legal in a constant expression.)
	// ----------------------------------------------------------------------------
	{
		constexpr hfp32 cx_zero{};
		static_assert(cx_zero.iszero(), "constexpr hfp32{} is zero");

		// ++0 -> next representable value; --0 -> previous representable value
		constexpr hfp32 cx_inc_zero = []() { hfp32 t{}; ++t; return t; }();
		constexpr hfp32 cx_dec_zero = []() { hfp32 t{}; --t; return t; }();
		static_assert(cx_inc_zero > hfp32(0.0), "constexpr ++0 > 0");
		static_assert(cx_dec_zero < hfp32(0.0), "constexpr --0 < 0");
	}

	// ----------------------------------------------------------------------------
	// SpecificValue construction.  IBM HFP has no NaN, no infinity:
	//   infpos -> maxpos (saturation), infneg -> maxneg
	//   qnan   -> zero,                snan   -> zero
	// These mappings are part of the hfloat contract and must hold at
	// constant-evaluation time as well as runtime.
	// ----------------------------------------------------------------------------
	{
		constexpr hfp32 zero(SpecificValue::zero);
		constexpr hfp32 maxp(SpecificValue::maxpos);
		constexpr hfp32 minp(SpecificValue::minpos);
		constexpr hfp32 minn(SpecificValue::minneg);
		constexpr hfp32 maxn(SpecificValue::maxneg);
		constexpr hfp32 infp(SpecificValue::infpos);
		constexpr hfp32 infn(SpecificValue::infneg);
		constexpr hfp32 qn(SpecificValue::qnan);
		static_assert(zero == hfp32(0.0), "constexpr SpecificValue::zero");
		static_assert(maxp > hfp32(0.0),  "constexpr SpecificValue::maxpos > 0");
		static_assert(minp > hfp32(0.0),  "constexpr SpecificValue::minpos > 0");
		static_assert(minn < hfp32(0.0),  "constexpr SpecificValue::minneg < 0");
		static_assert(maxn < hfp32(0.0),  "constexpr SpecificValue::maxneg < 0");
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
		constexpr hfp32 zero(0.0);
		constexpr hfp32 one(1.0);
		constexpr hfp32 maxp(SpecificValue::maxpos);
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
		constexpr hfp32 zero(0.0);
		constexpr hfp32 one(1.0);
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
		constexpr hfp64 a(2.0);
		constexpr hfp64 b(3.0);
		constexpr auto cx_sum = a + b;
		static_assert((cx_sum - hfp64(5.0)).iszero(), "constexpr hfloat_long +");

		constexpr auto cx_prod = a * b;
		static_assert((cx_prod - hfp64(6.0)).iszero(), "constexpr hfloat_long *");
	}

	// ----------------------------------------------------------------------------
	// abs / fabs free functions are constexpr
	// ----------------------------------------------------------------------------
	{
		constexpr hfp32 negv(-3.5);
		constexpr hfp32 posv = abs(negv);
		static_assert(posv == hfp32(3.5), "constexpr abs(-3.5) == 3.5");
		constexpr hfp32 posv2 = fabs(negv);
		static_assert(posv2 == hfp32(3.5), "constexpr fabs(-3.5) == 3.5");
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
		constexpr hfp64 a(9007199254740992LL);   // 2^53
		constexpr hfp64 b(9007199254740993LL);   // 2^53 + 1
		static_assert(a != b, "constexpr: hfloat_long preserves 2^53 + 1 distinct from 2^53");
		static_assert(a < b,  "constexpr: hfloat_long 2^53 < 2^53 + 1 (tuple compare)");
		static_assert(b > a,  "constexpr: hfloat_long 2^53 + 1 > 2^53");

		// Same witness via the unsigned conversion path.
		constexpr hfp64 au(9007199254740992ULL);
		constexpr hfp64 bu(9007199254740993ULL);
		static_assert(au != bu, "constexpr: hfloat_long unsigned preserves 2^53 + 1");

		// INT64_MIN: |v| computed via -(v+1)+1 identity, no overflow.
		constexpr long long llmin = (-9223372036854775807LL) - 1LL;  // INT64_MIN
		constexpr hfp64 smin(llmin);
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
