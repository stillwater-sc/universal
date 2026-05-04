// constexpr.cpp: compile-time tests for e8m0 (OCP exponent-only scaling factor)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#define E8M0_THROW_ARITHMETIC_EXCEPTION 0

#include <iostream>
#include <iomanip>
#include <universal/number/e8m0/e8m0.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "e8m0 constexpr verification";
	std::string test_tag    = "constexpr";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// e8m0: 8-bit exponent-only OCP scaling factor.
	// Value = 2^(encoding - 127), encoding 0xFF = NaN.
	// No fraction, no arithmetic operators (would be log-domain).

	// ----------------------------------------------------------------------------
	// SpecificValue construction (smoke)
	// ----------------------------------------------------------------------------
	{
		constexpr e8m0 zero(SpecificValue::zero);     // -> encoding 127 (= 1.0)
		constexpr e8m0 one(1.0f);                     // encoding 127
		constexpr e8m0 maxp(SpecificValue::maxpos);   // encoding 254 (= 2^127)
		constexpr e8m0 minp(SpecificValue::minpos);   // encoding 0   (= 2^-127)
		constexpr e8m0 qn(SpecificValue::qnan);       // encoding 0xFF (NaN)
		static_assert(zero.bits() == 127u,            "constexpr SpecificValue::zero -> encoding 127 (1.0)");
		static_assert(one.bits()  == 127u,            "constexpr e8m0(1.0f) -> encoding 127");
		static_assert(maxp.bits() == 254u,            "constexpr SpecificValue::maxpos -> encoding 254");
		static_assert(minp.bits() == 0u,              "constexpr SpecificValue::minpos -> encoding 0");
		static_assert(qn.isnan(),                     "constexpr SpecificValue::qnan -> isnan");
		static_assert(maxp.bits() != qn.bits(),       "constexpr maxpos != qnan encoding");
	}

	// ----------------------------------------------------------------------------
	// Issue #731 acceptance form: constexpr float-construction + comparison
	// ----------------------------------------------------------------------------
	{
		constexpr e8m0 a(2.0f);     // 2^1 -> encoding 128
		constexpr e8m0 b(4.0f);     // 2^2 -> encoding 129
		static_assert(a.bits() == 128u, "constexpr e8m0(2.0f) -> encoding 128");
		static_assert(b.bits() == 129u, "constexpr e8m0(4.0f) -> encoding 129");
		static_assert(a < b,            "constexpr: e8m0(2.0f) < e8m0(4.0f)");
	}

	// ----------------------------------------------------------------------------
	// Float construction across the dynamic range (powers of 2)
	// ----------------------------------------------------------------------------
	{
		constexpr e8m0 half(0.5f);          // 2^-1 -> encoding 126
		constexpr e8m0 one(1.0f);           // 2^0  -> encoding 127
		constexpr e8m0 two(2.0f);           // 2^1  -> encoding 128
		constexpr e8m0 e64(64.0f);          // 2^6  -> encoding 133
		constexpr e8m0 e1024(1024.0f);      // 2^10 -> encoding 137
		static_assert(half.bits()  == 126u, "constexpr e8m0(0.5f)    -> 126");
		static_assert(one.bits()   == 127u, "constexpr e8m0(1.0f)    -> 127");
		static_assert(two.bits()   == 128u, "constexpr e8m0(2.0f)    -> 128");
		static_assert(e64.bits()   == 133u, "constexpr e8m0(64.0f)   -> 133");
		static_assert(e1024.bits() == 137u, "constexpr e8m0(1024.0f) -> 137");
	}

	// ----------------------------------------------------------------------------
	// Round-to-nearest-power-of-2: midpoint behavior.
	// Geometric midpoint between 2^E and 2^(E+1) is sqrt(2) * 2^E.
	//   3.0 is between 2^1 (=2) and 2^2 (=4); 3.0 / 2.0 = 1.5 > sqrt(2),
	//   so e8m0(3.0f) rounds UP to encoding 129 (= 4.0).
	//   1.4 < sqrt(2), so e8m0(1.4f) rounds DOWN to encoding 127 (= 1.0).
	// ----------------------------------------------------------------------------
	{
		constexpr e8m0 a(3.0f);    // rounds up to 4.0 -> encoding 129
		constexpr e8m0 b(1.4f);    // rounds down to 1.0 -> encoding 127
		static_assert(a.bits() == 129u, "constexpr e8m0(3.0f) -> 4.0 (encoding 129)");
		static_assert(b.bits() == 127u, "constexpr e8m0(1.4f) -> 1.0 (encoding 127)");
	}

	// ----------------------------------------------------------------------------
	// Negative / zero input clamps to encoding 0 (e8m0 has no negative values
	// and no zero; minpos = 2^-127 is the smallest representation).
	// ----------------------------------------------------------------------------
	{
		constexpr e8m0 a(0.0f);
		constexpr e8m0 b(-1.0f);
		static_assert(a.bits() == 0u,   "constexpr e8m0(0.0f) clamps to encoding 0");
		static_assert(b.bits() == 0u,   "constexpr e8m0(-1.0f) clamps to encoding 0");
	}

	// ----------------------------------------------------------------------------
	// Infinity input clamps to maxpos (matching SpecificValue::infpos and
	// SpecificValue::infneg, both of which encode as maxpos in e8m0).
	// Caught by CodeRabbit on PR #810: pre-fix code routed -inf through
	// the v <= 0 clamp, encoding it as 0 (inconsistent with infneg ctor).
	// ----------------------------------------------------------------------------
	{
		constexpr float pinf = std::numeric_limits<float>::infinity();
		constexpr e8m0 pos_inf( pinf);
		constexpr e8m0 neg_inf(-pinf);
		static_assert(pos_inf.bits() == 254u, "constexpr e8m0(+inf) -> maxpos (254)");
		static_assert(neg_inf.bits() == 254u, "constexpr e8m0(-inf) -> maxpos (254)");
	}

	// ----------------------------------------------------------------------------
	// Integer construction
	// ----------------------------------------------------------------------------
	{
		constexpr e8m0 a(1);    // 2^0 -> 127
		constexpr e8m0 b(8);    // 2^3 -> 130
		constexpr e8m0 c(16);   // 2^4 -> 131
		static_assert(a.bits() == 127u, "constexpr e8m0(int 1)  -> 127");
		static_assert(b.bits() == 130u, "constexpr e8m0(int 8)  -> 130");
		static_assert(c.bits() == 131u, "constexpr e8m0(int 16) -> 131");
	}

	// ----------------------------------------------------------------------------
	// Conversion-out: operator float() / operator double()
	// ----------------------------------------------------------------------------
	{
		constexpr e8m0 a(2.0f);
		constexpr float fa = float(a);
		static_assert(fa == 2.0f,        "constexpr operator float() -> 2.0f");

		constexpr e8m0 b(0.5f);
		constexpr float fb = float(b);
		static_assert(fb == 0.5f,        "constexpr operator float() -> 0.5f");

		constexpr e8m0 c(1.0f);
		constexpr double dc = double(c);
		static_assert(dc == 1.0,         "constexpr operator double() -> 1.0");
	}

	// ----------------------------------------------------------------------------
	// Increment / decrement
	// ----------------------------------------------------------------------------
	{
		constexpr e8m0 cx_inc = []() { e8m0 t(1.0f); ++t; return t; }();
		constexpr e8m0 cx_dec = []() { e8m0 t(1.0f); --t; return t; }();
		static_assert(cx_inc.bits() == 128u, "constexpr ++e8m0(1.0f) -> 128 (= 2.0)");
		static_assert(cx_dec.bits() == 126u, "constexpr --e8m0(1.0f) -> 126 (= 0.5)");

		// Postfix: returns pre-increment value, mutates operand.
		constexpr e8m0 cx_postinc_before = []() { e8m0 t(1.0f); e8m0 old = t++; return old; }();
		constexpr e8m0 cx_postinc_after  = []() { e8m0 t(1.0f); t++; return t; }();
		static_assert(cx_postinc_before.bits() == 127u, "constexpr postfix ++ returns pre-value");
		static_assert(cx_postinc_after.bits()  == 128u, "constexpr postfix ++ mutates");

		// Saturation at boundaries: ++ caps at 254, -- caps at 0.
		constexpr e8m0 cx_inc_max = []() { e8m0 t(SpecificValue::maxpos); ++t; return t; }();
		constexpr e8m0 cx_dec_min = []() { e8m0 t(SpecificValue::minpos); --t; return t; }();
		static_assert(cx_inc_max.bits() == 254u, "constexpr ++ saturates at maxpos (254)");
		static_assert(cx_dec_min.bits() == 0u,   "constexpr -- saturates at minpos (0)");
	}

	// ----------------------------------------------------------------------------
	// Comparison operators (issue acceptance: <)
	// ----------------------------------------------------------------------------
	{
		constexpr e8m0 a(1.0f);   // encoding 127
		constexpr e8m0 b(2.0f);   // encoding 128
		static_assert(a < b,      "constexpr: e8m0(1.0) < e8m0(2.0)");
		static_assert(b > a,      "constexpr: e8m0(2.0) > e8m0(1.0)");
		static_assert(a <= b,     "constexpr: e8m0(1.0) <= e8m0(2.0)");
		static_assert(b >= a,     "constexpr: e8m0(2.0) >= e8m0(1.0)");
		static_assert(!(a == b),  "constexpr: e8m0(1.0) != e8m0(2.0)");
		static_assert(a != b,     "constexpr: != operator");
		static_assert(a == a,     "constexpr: e8m0(1.0) == e8m0(1.0)");

		// NaN compares unequal to anything (including itself).  Per IEEE 754
		// semantics, ALL six relational ops return false when either operand
		// is NaN -- including >= and <= (which is easy to get wrong if you
		// implement >= as !< -- see CR finding on PR #810).
		constexpr e8m0 nan(SpecificValue::qnan);
		static_assert(!(nan == nan), "constexpr: NaN != NaN");
		static_assert(!(nan <  a),   "constexpr: NaN <  x  -> false");
		static_assert(!(nan >  a),   "constexpr: NaN >  x  -> false");
		static_assert(!(nan <= a),   "constexpr: NaN <= x  -> false");
		static_assert(!(nan >= a),   "constexpr: NaN >= x  -> false");
		static_assert(!(a   <  nan), "constexpr: x   <  NaN -> false");
		static_assert(!(a   >  nan), "constexpr: x   >  NaN -> false");
		static_assert(!(a   <= nan), "constexpr: x   <= NaN -> false");
		static_assert(!(a   >= nan), "constexpr: x   >= NaN -> false");
		static_assert(!(nan <= nan), "constexpr: NaN <= NaN -> false");
		static_assert(!(nan >= nan), "constexpr: NaN >= NaN -> false");
		static_assert(nan != nan,    "constexpr: NaN != NaN via operator!=");
	}

	// ----------------------------------------------------------------------------
	// Selectors (already constexpr -- smoke test the contract)
	// ----------------------------------------------------------------------------
	{
		constexpr e8m0 a(1.0f);
		constexpr e8m0 nan(SpecificValue::qnan);
		static_assert(a.isone(),      "constexpr e8m0(1.0f).isone()");
		static_assert(!a.isnan(),     "constexpr e8m0(1.0f).isnan() == false");
		static_assert(!a.iszero(),    "constexpr e8m0 cannot represent zero");
		static_assert(!a.sign(),      "constexpr e8m0 always positive");
		static_assert(a.scale() == 0, "constexpr e8m0(1.0f).scale() == 0");
		static_assert(a.exponent() == 0, "constexpr e8m0(1.0f).exponent() == 0");
		static_assert(nan.isnan(),    "constexpr qnan isnan()");
	}

	std::cout << "e8m0 constexpr verification: "
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
