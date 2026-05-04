// constexpr.cpp: compile-time tests for microfloat (8-bit float family: e4m3, e5m2, e2m1)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#define MICROFLOAT_THROW_ARITHMETIC_EXCEPTION 0

#include <iostream>
#include <iomanip>
#include <universal/number/microfloat/microfloat.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "microfloat constexpr verification";
	std::string test_tag    = "constexpr";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// e4m3: 8 bits, 4 exponent bits, no infinity, has NaN, saturating.
	// e5m2: 8 bits, 5 exponent bits, has infinity, has NaN, IEEE-like.
	using e4m3 = microfloat<8, 4, false, true, true>;
	using e5m2 = microfloat<8, 5, true,  true, false>;

	// ----------------------------------------------------------------------------
	// SpecificValue construction (smoke)
	// ----------------------------------------------------------------------------
	{
		constexpr e4m3 zero(SpecificValue::zero);
		constexpr e4m3 maxp(SpecificValue::maxpos);
		constexpr e4m3 minp(SpecificValue::minpos);
		constexpr e4m3 minn(SpecificValue::minneg);
		constexpr e4m3 maxn(SpecificValue::maxneg);
		constexpr e4m3 qn(SpecificValue::qnan);
		static_assert(zero.iszero(),     "constexpr e4m3 zero");
		static_assert(maxp > e4m3(0.0f), "constexpr e4m3 maxpos > 0");
		static_assert(minp > e4m3(0.0f), "constexpr e4m3 minpos > 0");
		static_assert(minn < e4m3(0.0f), "constexpr e4m3 minneg < 0");
		static_assert(maxn < e4m3(0.0f), "constexpr e4m3 maxneg < 0");
		static_assert(qn.isnan(),        "constexpr e4m3 qnan isnan");
	}

	// ----------------------------------------------------------------------------
	// Issue #733 acceptance form: constexpr a + b + comparison
	// ----------------------------------------------------------------------------
	{
		constexpr e4m3 a(2.0f);
		constexpr e4m3 b(3.0f);
		constexpr auto cx_sum = a + b;        // 5.0
		static_assert(cx_sum == e4m3(5.0f),   "issue #733 acceptance: e4m3(2.0) + e4m3(3.0) == 5.0");
		static_assert(a < b,                  "issue #733 acceptance: e4m3(2.0) < e4m3(3.0)");
	}

	// ----------------------------------------------------------------------------
	// All four binary arithmetic operators in constexpr context
	// ----------------------------------------------------------------------------
	{
		constexpr e4m3 a(4.0f);
		constexpr e4m3 b(2.0f);
		constexpr auto cx_sum  = a + b;       // 6.0
		constexpr auto cx_diff = a - b;       // 2.0
		constexpr auto cx_prod = a * b;       // 8.0
		constexpr auto cx_quot = a / b;       // 2.0
		constexpr auto cx_neg  = -a;          // -4.0
		static_assert(cx_sum  == e4m3(6.0f),  "constexpr +  failed");
		static_assert(cx_diff == e4m3(2.0f),  "constexpr -  failed");
		static_assert(cx_prod == e4m3(8.0f),  "constexpr *  failed");
		static_assert(cx_quot == e4m3(2.0f),  "constexpr /  failed");
		static_assert(cx_neg  == e4m3(-4.0f), "constexpr unary - failed");

		// Compound assignment via constexpr lambda (C++20)
		constexpr e4m3 cx_addeq = []() { e4m3 t(2.0f); t += e4m3(1.0f); return t; }();
		constexpr e4m3 cx_subeq = []() { e4m3 t(4.0f); t -= e4m3(1.0f); return t; }();
		constexpr e4m3 cx_muleq = []() { e4m3 t(2.0f); t *= e4m3(2.0f); return t; }();
		constexpr e4m3 cx_diveq = []() { e4m3 t(4.0f); t /= e4m3(2.0f); return t; }();
		static_assert(cx_addeq == e4m3(3.0f), "constexpr += failed");
		static_assert(cx_subeq == e4m3(3.0f), "constexpr -= failed");
		static_assert(cx_muleq == e4m3(4.0f), "constexpr *= failed");
		static_assert(cx_diveq == e4m3(2.0f), "constexpr /= failed");
	}

	// ----------------------------------------------------------------------------
	// Comparison operators (issue acceptance: <)
	// ----------------------------------------------------------------------------
	{
		constexpr e4m3 a(1.0f);
		constexpr e4m3 b(2.0f);
		static_assert(a < b,      "constexpr: e4m3(1) < e4m3(2)");
		static_assert(b > a,      "constexpr: e4m3(2) > e4m3(1)");
		static_assert(a <= b,     "constexpr: e4m3(1) <= e4m3(2)");
		static_assert(b >= a,     "constexpr: e4m3(2) >= e4m3(1)");
		static_assert(!(a == b),  "constexpr: e4m3(1) != e4m3(2)");
		static_assert(a != b,     "constexpr: != operator");
		static_assert(a == a,     "constexpr: e4m3(1) == e4m3(1)");

		// IEEE-754 semantics: ALL relational ops return false for any NaN
		// operand, including >= and <= (the !operator< trap fires here).
		constexpr e4m3 nan(SpecificValue::qnan);
		static_assert(!(nan == nan), "constexpr: NaN != NaN");
		static_assert(!(nan <  a),   "constexpr: NaN <  x  -> false");
		static_assert(!(nan >  a),   "constexpr: NaN >  x  -> false");
		static_assert(!(nan <= a),   "constexpr: NaN <= x  -> false");
		static_assert(!(nan >= a),   "constexpr: NaN >= x  -> false");
		static_assert(!(a   <  nan), "constexpr: x   <  NaN -> false");
		static_assert(!(a   >  nan), "constexpr: x   >  NaN -> false");
		static_assert(!(a   <= nan), "constexpr: x   <= NaN -> false");
		static_assert(!(a   >= nan), "constexpr: x   >= NaN -> false");
		static_assert(nan != nan,    "constexpr: NaN != NaN via operator!=");
	}

	// ----------------------------------------------------------------------------
	// Conversion-out: operator float() / operator double() / operator int()
	// ----------------------------------------------------------------------------
	{
		constexpr e4m3 a(2.0f);
		constexpr float fa = float(a);
		static_assert(fa == 2.0f, "constexpr operator float() -> 2.0f");

		constexpr e4m3 b(0.5f);
		constexpr double db = double(b);
		static_assert(db == 0.5,  "constexpr operator double() -> 0.5");

		constexpr e4m3 c(7.0f);
		constexpr int ic = int(c);
		static_assert(ic == 7,    "constexpr operator int() -> 7");
	}

	// ----------------------------------------------------------------------------
	// Increment / decrement at encoding granularity
	// ----------------------------------------------------------------------------
	{
		// e4m3 encoding 0x38 = 1.0 (sign=0, exp=7, frac=0).  ++ moves to next
		// representable encoding; the float value is unspecified beyond
		// "next encoding above" so we assert via comparison rather than
		// exact float equality.
		constexpr e4m3 cx_inc = []() { e4m3 t(1.0f); ++t; return t; }();
		constexpr e4m3 cx_dec = []() { e4m3 t(1.0f); --t; return t; }();
		static_assert(cx_inc > e4m3(1.0f), "constexpr ++e4m3(1.0f) > 1.0");
		static_assert(cx_dec < e4m3(1.0f), "constexpr --e4m3(1.0f) < 1.0");

		// Postfix returns pre-value, mutates operand.
		constexpr e4m3 cx_postinc_before = []() { e4m3 t(1.0f); e4m3 old = t++; return old; }();
		constexpr e4m3 cx_postinc_after  = []() { e4m3 t(1.0f); t++; return t; }();
		static_assert(cx_postinc_before == e4m3(1.0f), "constexpr postfix ++ returns pre-value");
		static_assert(cx_postinc_after  >  e4m3(1.0f), "constexpr postfix ++ mutates");
	}

	// ----------------------------------------------------------------------------
	// e5m2 (IEEE-like with infinity) construction + arithmetic
	// ----------------------------------------------------------------------------
	{
		constexpr e5m2 a(2.0f);
		constexpr e5m2 b(0.5f);
		constexpr auto cx_sum = a + b;        // 2.5
		static_assert(cx_sum == e5m2(2.5f),   "constexpr e5m2 +");

		// e5m2 has infinity; constructing from numeric_limits<float>::infinity()
		// should produce a microfloat infinity, not maxpos.
		constexpr e5m2 inf_pos(std::numeric_limits<float>::infinity());
		static_assert(inf_pos.isinf(),        "constexpr e5m2(+inf) isinf");
	}

	// ----------------------------------------------------------------------------
	// Native int construction
	// ----------------------------------------------------------------------------
	{
		constexpr e4m3 a(3);
		constexpr e4m3 b(-2);
		static_assert(a == e4m3(3.0f),    "constexpr e4m3(int 3) == e4m3(3.0f)");
		static_assert(b == e4m3(-2.0f),   "constexpr e4m3(int -2) == e4m3(-2.0f)");
	}

	// ----------------------------------------------------------------------------
	// Selectors smoke test (already constexpr -- lock in the contract)
	// ----------------------------------------------------------------------------
	{
		constexpr e4m3 a(1.0f);
		constexpr e4m3 nan(SpecificValue::qnan);
		static_assert(a.isone(),      "constexpr e4m3(1.0f).isone()");
		static_assert(!a.isnan(),     "constexpr e4m3(1.0f).isnan() == false");
		static_assert(!a.iszero(),    "constexpr e4m3(1.0f).iszero() == false");
		static_assert(!a.sign(),      "constexpr e4m3(1.0f) is positive");
		static_assert(nan.isnan(),    "constexpr qnan isnan");
	}

	// ----------------------------------------------------------------------------
	// Mixed-type comparison NaN-safety (CR round-2 catch on PR #811).
	//
	// Pre-fix the mf-float / float-mf overloads narrowed the float operand to
	// microfloat first; for hasNaN==false instantiations from_float(NaN)
	// collapses to zero, so `mf == NaN_float` would silently become
	// `mf == 0` -- breaking the IEEE 754 "all relational ops false / != true
	// for any NaN operand" contract.  Tests cover BOTH a hasNaN=true type
	// (e4m3) and a hasNaN=false type (a synthetic 4-bit no-NaN config) so
	// the fix is verified across the relevant configurations.
	// ----------------------------------------------------------------------------
	{
		constexpr float fnan = std::numeric_limits<float>::quiet_NaN();
		constexpr e4m3 a(1.0f);

		// e4m3 has hasNaN=true: pre-fix would also have returned false for ==
		// because microfloat(NaN) becomes a NaN encoding and the mf-mf == op
		// catches it.  Still, the float-NaN short-circuit is the right
		// place to handle this and keeps behavior consistent.
		static_assert(!(a == fnan),  "constexpr: e4m3 == float NaN -> false");
		static_assert( (a != fnan),  "constexpr: e4m3 != float NaN -> true");
		static_assert(!(a <  fnan),  "constexpr: e4m3 <  float NaN -> false");
		static_assert(!(a >  fnan),  "constexpr: e4m3 >  float NaN -> false");
		static_assert(!(a <= fnan),  "constexpr: e4m3 <= float NaN -> false");
		static_assert(!(a >= fnan),  "constexpr: e4m3 >= float NaN -> false");
		// Symmetric float-mf overloads
		static_assert(!(fnan == a),  "constexpr: float NaN == e4m3 -> false");
		static_assert( (fnan != a),  "constexpr: float NaN != e4m3 -> true");
		static_assert(!(fnan <  a),  "constexpr: float NaN <  e4m3 -> false");
		static_assert(!(fnan >  a),  "constexpr: float NaN >  e4m3 -> false");
		static_assert(!(fnan <= a),  "constexpr: float NaN <= e4m3 -> false");
		static_assert(!(fnan >= a),  "constexpr: float NaN >= e4m3 -> false");

		// hasNaN=false instantiation: this is where the original bug bit.
		// from_float(NaN) for these collapses to zero, so without the
		// short-circuit `e_no_nan == NaN` would compare against 0.
		using e2m1_no_nan = microfloat<4, 2, false, false, true>;
		constexpr e2m1_no_nan z{};  // value-init -> encoding 0 (positive zero)
		constexpr e2m1_no_nan one_v(1.0f);
		// Without the fix, `z == fnan` would narrow fnan to 0 (= z) and return TRUE.
		static_assert(!(z == fnan),     "constexpr: hasNaN=false e2m1 == NaN -> false");
		static_assert( (z != fnan),     "constexpr: hasNaN=false e2m1 != NaN -> true");
		static_assert(!(one_v < fnan),  "constexpr: hasNaN=false e2m1 < NaN -> false");
		static_assert(!(fnan == z),     "constexpr: NaN == hasNaN=false e2m1 -> false");
		static_assert( (fnan != z),     "constexpr: NaN != hasNaN=false e2m1 -> true");
	}

	// ----------------------------------------------------------------------------
	// Signed zero preservation (CR catch on PR #811).  microfloat models
	// +0 and -0 distinctly; converting from -0.0f must produce the
	// negative-zero encoding so unary - and to_float() round-trip.
	// ----------------------------------------------------------------------------
	{
		constexpr e4m3 pos_zero(0.0f);
		constexpr e4m3 neg_zero(-0.0f);
		static_assert(pos_zero.iszero(),     "constexpr +0.0f -> iszero");
		static_assert(neg_zero.iszero(),     "constexpr -0.0f -> iszero");
		static_assert(!pos_zero.sign(),      "constexpr +0.0f -> sign() == false");
		static_assert(neg_zero.sign(),       "constexpr -0.0f -> sign() == true (preserved)");
		// Round-trip: -0.0f -> microfloat -> float sign bit preserved
		static_assert(float(neg_zero) == 0.0f, "constexpr -0.0 round-trips numerically");
		// Unary minus on +0 should produce a microfloat with the sign bit set.
		constexpr e4m3 negated_zero = -pos_zero;
		static_assert(negated_zero.sign(),    "constexpr -(+0) sets sign bit");
	}

	// ----------------------------------------------------------------------------
	// abs / unary minus
	// ----------------------------------------------------------------------------
	{
		constexpr e4m3 negv(-3.0f);
		constexpr e4m3 absv = abs(negv);
		static_assert(absv == e4m3(3.0f), "constexpr abs(-3.0) == 3.0");
		constexpr e4m3 negn = -negv;
		static_assert(negn == e4m3(3.0f), "constexpr -(-3.0) == 3.0");
	}

	std::cout << "microfloat constexpr verification: "
	          << (nrOfFailedTestCases == 0 ? "PASS\n" : "FAIL\n");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << '\n';
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
