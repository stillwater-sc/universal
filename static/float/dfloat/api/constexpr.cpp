// constexpr.cpp: compile-time tests for dfloat (IEEE 754-2008 decimal floating-point)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure dfloat: throwing arithmetic exceptions disabled so divide-by-zero
// has defined constexpr-safe behavior (returns IEEE 754-2008 +/-inf or NaN)
// rather than throwing, which would be ill-formed in a constant expression.
#define DFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#define BLOCKBINARY_THROW_ARITHMETIC_EXCEPTION 0

#include <iostream>
#include <iomanip>
#include <universal/number/dfloat/dfloat.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "dfloat constexpr verification";
	std::string test_tag    = "constexpr";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// decimal32 (dfloat<7, 6, BID, uint32_t>) -- the smallest IEEE 754-2008
	// decimal float.  decimal64 used where wider exponent range is needed.
	// decimal128 is intentionally excluded from this constexpr suite: its
	// convert_to_double() routes through std::strtod for sig_bits > 64,
	// which is not constexpr.  The wide path is fenced under
	// !std::is_constant_evaluated() so runtime callers still work.
	using D32 = decimal32;
	using D64 = decimal64;

	// ----------------------------------------------------------------------------
	// Native int construction (smoke test)
	// ----------------------------------------------------------------------------
	{
		constexpr D32 a(0);
		constexpr D32 b(1);
		constexpr D32 c(-3);
		(void)a; (void)b; (void)c;
	}

	// ----------------------------------------------------------------------------
	// Acceptance form from issue #730:
	//   constexpr dfloat<...> a(2.0), b(3.0); constexpr auto c = a + b;
	// (and analogous for *, -, /, +=, <)
	// ----------------------------------------------------------------------------
	{
		constexpr D32 a(2.0);
		constexpr D32 b(3.0);
		constexpr auto cx_sum = a + b;  // 2 + 3 = 5
		static_assert((cx_sum - D32(5.0)).iszero(), "issue #730 acceptance: a + b");
	}

	// ----------------------------------------------------------------------------
	// All four binary arithmetic operators in constexpr context
	//
	// Note on equality: dfloat's operator== uses structural equality (sign,
	// exponent, significand all match).  IEEE 754-2008 decimal stores values
	// in a cohort -- 3.0 can be encoded as (sig=3, exp=0) or (sig=30, exp=-1).
	// Arithmetic preserves trailing-zero cohorts that convert_ieee754 strips,
	// so we compare via subtraction + iszero() to assert mathematical equality
	// without depending on the cohort representation.
	// ----------------------------------------------------------------------------
	{
		constexpr D32 a(4.5);
		constexpr D32 b(1.5);
		constexpr auto cx_sum  = a + b;          // 4.5 + 1.5 = 6.0
		constexpr auto cx_diff = a - b;          // 4.5 - 1.5 = 3.0
		constexpr auto cx_prod = a * b;          // 4.5 * 1.5 = 6.75
		constexpr auto cx_quot = a / b;          // 4.5 / 1.5 = 3.0
		constexpr auto cx_neg  = -a;             // -4.5
		static_assert((cx_sum  - D32(6.0)).iszero(),  "constexpr +  failed");
		static_assert((cx_diff - D32(3.0)).iszero(),  "constexpr -  failed");
		static_assert((cx_prod - D32(6.75)).iszero(), "constexpr *  failed");
		static_assert((cx_quot - D32(3.0)).iszero(),  "constexpr /  failed");
		static_assert((cx_neg  - D32(-4.5)).iszero(), "constexpr unary - failed");

		// Compound assignment via lambda (constexpr lambdas are C++20)
		constexpr D32 cx_addeq = []() { D32 t(1.5); t += D32(3.0); return t; }();
		constexpr D32 cx_subeq = []() { D32 t(4.5); t -= D32(1.5); return t; }();
		constexpr D32 cx_muleq = []() { D32 t(1.5); t *= D32(3.0); return t; }();
		constexpr D32 cx_diveq = []() { D32 t(4.5); t /= D32(1.5); return t; }();
		static_assert((cx_addeq - D32(4.5)).iszero(), "constexpr += failed");
		static_assert((cx_subeq - D32(3.0)).iszero(), "constexpr -= failed");
		static_assert((cx_muleq - D32(4.5)).iszero(), "constexpr *= failed");
		static_assert((cx_diveq - D32(3.0)).iszero(), "constexpr /= failed");
	}

	// ----------------------------------------------------------------------------
	// Constexpr comparison (issue acceptance: <)
	// ----------------------------------------------------------------------------
	{
		constexpr D32 a(1.5);
		constexpr D32 b(3.0);
		static_assert(a < b,     "constexpr: D32(1.5) < D32(3.0)");
		static_assert(b > a,     "constexpr: D32(3.0) > D32(1.5)");
		static_assert(a <= b,    "constexpr: D32(1.5) <= D32(3.0)");
		static_assert(b >= a,    "constexpr: D32(3.0) >= D32(1.5)");
		static_assert(!(a == b), "constexpr: D32(1.5) != D32(3.0)");
		static_assert(a != b,    "constexpr: != operator");
		static_assert(a == a,    "constexpr: D32(1.5) == D32(1.5)");
	}

	// ----------------------------------------------------------------------------
	// Conversion-out: operator double() / float() / int() are now constexpr
	// (decimal32/64 paths only -- decimal128 wide path is runtime-fenced)
	// ----------------------------------------------------------------------------
	{
		constexpr D32 a(0.5);
		constexpr double d = double(a);
		static_assert(d == 0.5, "constexpr operator double()");

		constexpr D32 b(2.5);
		constexpr float f = float(b);
		static_assert(f == 2.5f, "constexpr operator float()");
	}

	// ----------------------------------------------------------------------------
	// Increment / decrement (next/prev representable value)
	//
	// Default-constructed `D32 t;` leaves storage indeterminate (trivial ctor
	// is required for plug-in semantics with native types) -- reading it in
	// a constant expression is ill-formed.  Value-initialization `D32 t{}`
	// zero-fills via the encoding's array aggregate initializer, which IS
	// legal in constexpr.  This is the contract we lock in: `D32{}` is a
	// constexpr-safe representation of zero.
	// ----------------------------------------------------------------------------
	{
		constexpr D32 cx_zero{};
		static_assert(cx_zero.iszero(), "constexpr D32{} is zero");

		constexpr D32 cx_inc_zero = []() { D32 t{}; ++t; return t; }();
		constexpr D32 cx_dec_zero = []() { D32 t{}; --t; return t; }();
		// ++0 = minpos, --0 = minneg (per dfloat operator++/--)
		static_assert(cx_inc_zero > D32(0.0), "constexpr ++0 > 0");
		static_assert(cx_dec_zero < D32(0.0), "constexpr --0 < 0");
	}

	// ----------------------------------------------------------------------------
	// SpecificValue construction
	// ----------------------------------------------------------------------------
	{
		constexpr D32 zero(SpecificValue::zero);
		constexpr D32 maxp(SpecificValue::maxpos);
		constexpr D32 minp(SpecificValue::minpos);
		constexpr D32 minn(SpecificValue::minneg);
		constexpr D32 maxn(SpecificValue::maxneg);
		constexpr D32 infp(SpecificValue::infpos);
		constexpr D32 infn(SpecificValue::infneg);
		constexpr D32 qn(SpecificValue::qnan);
		static_assert(zero == D32(0.0), "constexpr SpecificValue::zero");
		static_assert(maxp > D32(0.0),  "constexpr SpecificValue::maxpos > 0");
		static_assert(minp > D32(0.0),  "constexpr SpecificValue::minpos > 0");
		static_assert(minn < D32(0.0),  "constexpr SpecificValue::minneg < 0");
		static_assert(maxn < D32(0.0),  "constexpr SpecificValue::maxneg < 0");
		static_assert(maxp > minp,      "constexpr maxpos > minpos");
		static_assert(infp.isinf(),     "constexpr SpecificValue::infpos isinf");
		static_assert(infn.isinf(),     "constexpr SpecificValue::infneg isinf");
		static_assert(qn.isnan(),       "constexpr SpecificValue::qnan isnan");
	}

	// ----------------------------------------------------------------------------
	// Special-value arithmetic in constexpr (NaN/inf propagation)
	// ----------------------------------------------------------------------------
	{
		constexpr D32 zero(0.0);
		constexpr D32 one(1.0);
		// 1/0 = +inf (DFLOAT_THROW_ARITHMETIC_EXCEPTION 0)
		constexpr auto cx_div_zero = one / zero;
		static_assert(cx_div_zero.isinf(), "constexpr 1/0 -> inf");

		// 0/0 = NaN
		constexpr auto cx_zero_div_zero = zero / zero;
		static_assert(cx_zero_div_zero.isnan(), "constexpr 0/0 -> NaN");

		// inf - inf = NaN
		constexpr D32 inf_pos(SpecificValue::infpos);
		constexpr auto cx_inf_minus_inf = inf_pos - inf_pos;
		static_assert(cx_inf_minus_inf.isnan(), "constexpr inf - inf -> NaN");

		// inf * 0 = NaN
		constexpr auto cx_inf_times_zero = inf_pos * zero;
		static_assert(cx_inf_times_zero.isnan(), "constexpr inf * 0 -> NaN");
	}

	// ----------------------------------------------------------------------------
	// decimal64 smoke (wider exponent range, sig_bits still <= 64)
	// ----------------------------------------------------------------------------
	{
		constexpr D64 a(2.0);
		constexpr D64 b(3.0);
		constexpr auto cx_sum = a + b;
		static_assert((cx_sum - D64(5.0)).iszero(), "constexpr decimal64 +");
	}

	// ----------------------------------------------------------------------------
	// DPD encoding variant (pure constexpr through DPD codec lookup tables)
	// ----------------------------------------------------------------------------
	{
		using D32_DPD = dfloat<7, 6, DecimalEncoding::DPD, uint32_t>;
		constexpr D32_DPD a(12.5);
		constexpr D32_DPD b(2.5);
		constexpr auto sum_dpd = a + b;
		static_assert((sum_dpd - D32_DPD(15.0)).iszero(), "constexpr DPD addition");
	}

	std::cout << "dfloat constexpr verification: "
	          << (nrOfFailedTestCases == 0 ? "PASS\n" : "FAIL\n");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const sw::universal::dfloat_arithmetic_exception& err) {
	std::cerr << "Uncaught dfloat arithmetic exception: " << err.what() << '\n';
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
