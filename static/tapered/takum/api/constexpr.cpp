// constexpr.cpp: compile-time tests for takum (linear takum logarithmic encoding)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Locks in the constexpr contract for takum after the Epic #723 / issue #741
// promotion: convert_ieee754 (frexp -> bit-extraction), to_ieee754 (std::exp2
// -> sw::math::constexpr_math::exp2), and all arithmetic / comparison /
// conversion operators evaluable at constant evaluation when sw::bit_cast is
// constexpr (BIT_CAST_IS_CONSTEXPR).
#include <universal/utility/directives.hpp>

#include <iostream>
#include <iomanip>
#include <universal/number/takum/takum.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "takum constexpr verification";
	std::string test_tag    = "constexpr";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// We use takum<16, 3, uint8_t> as the headline configuration (the linear
	// takum reference uses rbits = 3, and uint8_t storage exercises the
	// multi-block path) and takum<24, 3, uint32_t> for wider-storage coverage.
	// Note: rbits >= 5 hits a pre-existing int-overflow in
	// max_characteristic() / min_characteristic() that surfaces only at
	// constant evaluation; tracking that as a separate fix.
	using tk16  = takum<16, 3, std::uint8_t>;
	using tk24w = takum<24, 3, std::uint32_t>;

	// ----------------------------------------------------------------------------
	// Static accessors and structural invariants (already constexpr; smoke test).
	// ----------------------------------------------------------------------------
	{
		static_assert(tk16::nbits == 16u,  "constexpr nbits == 16");
		static_assert(tk16::rbits == 3u,   "constexpr rbits == 3");
		static_assert(tk16::overhead == 5u, "constexpr overhead == 2 + rbits");
		static_assert(tk16::dr_bits == 4u, "constexpr dr_bits == 1 + rbits");
		static_assert(tk16::nr_dr_values == 16u, "constexpr nr_dr_values == 2^dr_bits");
		static_assert(tk16::max_r == 7u,   "constexpr max_r == 2^rbits - 1");
		static_assert(tk16::maxCharBits == 11u, "constexpr maxCharBits == nbits - overhead");
		static_assert(tk24w::nbits == 24u, "constexpr tk24w::nbits == 24");
		static_assert(tk24w::rbits == 3u,  "constexpr tk24w::rbits == 3");
	}

	// ----------------------------------------------------------------------------
	// SpecificValue construction (already constexpr in the prior code; we
	// re-assert here so the contract is locked in alongside the new constexpr
	// arithmetic).
	// ----------------------------------------------------------------------------
	{
		constexpr tk16 z(SpecificValue::zero);
		static_assert(z.iszero(),       "constexpr SpecificValue::zero -> iszero()");
		static_assert(!z.isnar(),       "constexpr SpecificValue::zero -> !isnar()");

		constexpr tk16 mp(SpecificValue::maxpos);
		static_assert(!mp.iszero(),     "constexpr SpecificValue::maxpos -> !iszero()");
		static_assert(!mp.sign(),       "constexpr SpecificValue::maxpos -> sign() == false");
		static_assert(mp.ispos(),       "constexpr SpecificValue::maxpos -> ispos()");

		constexpr tk16 mn(SpecificValue::maxneg);
		static_assert(mn.sign(),        "constexpr SpecificValue::maxneg -> sign() == true");
		static_assert(mn.isneg(),       "constexpr SpecificValue::maxneg -> isneg()");

		constexpr tk16 nar(SpecificValue::nar);
		static_assert(nar.isnar(),      "constexpr SpecificValue::nar -> isnar()");
		static_assert(nar.isnan(),      "constexpr SpecificValue::nar -> isnan()");
		// takum has no infinity (NaR replaces both infinities and NaN)
		static_assert(!nar.isinf(),     "constexpr takum has no infinity");
	}

	// ----------------------------------------------------------------------------
	// Trivial-default contract: default-constructed instance is non-trivially
	// initialized (storage indeterminate).  We use value-initialization in
	// every constexpr context so the test stays well-defined.
	// ----------------------------------------------------------------------------
	{
		constexpr tk16 v{};
		static_assert(v.iszero(), "constexpr value-init takum is zero");
		static_assert(!v.isnar(), "constexpr value-init takum is not NaR");
	}

	// ----------------------------------------------------------------------------
	// Conversion in (int / float / double) at constant evaluation.
	// The key promotion: convert_ieee754 no longer calls std::frexp, so this
	// now constant-evaluates.
	// ----------------------------------------------------------------------------
	{
		// Integer one -> takum, then back to double via operator double().
		constexpr tk16 one_i = tk16(1);
		static_assert(!one_i.iszero(), "constexpr tk16(1) is non-zero");
		static_assert(one_i.ispos(),   "constexpr tk16(1) is positive");
		// 1.0 has c = 0 in the takum decomposition (m_real = 0); round-trip exact.
		static_assert(static_cast<double>(one_i) == 1.0, "constexpr round-trip 1 -> tk16 -> 1.0");

		constexpr tk16 two_d = tk16(2.0);
		static_assert(static_cast<double>(two_d) == 2.0, "constexpr round-trip 2.0 -> tk16 -> 2.0");

		constexpr tk16 four_f = tk16(4.0f);
		static_assert(static_cast<double>(four_f) == 4.0, "constexpr round-trip 4.0f -> tk16 -> 4.0");

		// Negative power-of-two: also exact.
		constexpr tk16 mhalf = tk16(-0.5);
		static_assert(static_cast<double>(mhalf) == -0.5, "constexpr round-trip -0.5 -> tk16 -> -0.5");

		// Zero round-trip.
		constexpr tk16 zero_d = tk16(0.0);
		static_assert(static_cast<double>(zero_d) == 0.0, "constexpr round-trip 0.0");
		static_assert(zero_d.iszero(), "constexpr tk16(0.0).iszero()");

		// NaN -> NaR.
		constexpr tk16 nan_d = tk16(std::numeric_limits<double>::quiet_NaN());
		static_assert(nan_d.isnar(), "constexpr tk16(NaN).isnar()");
	}

	// ----------------------------------------------------------------------------
	// Arithmetic operators at constant evaluation.  Internally these go
	// through double and rely on the constexpr exp2 / bit-extraction paths.
	// ----------------------------------------------------------------------------
	{
		// Addition of two power-of-two values that round to representable
		// takum values.  2 + 2 = 4 has an exact takum encoding.
		constexpr tk16 a = tk16(2.0);
		constexpr tk16 b = tk16(2.0);
		constexpr tk16 sum = a + b;
		static_assert(static_cast<double>(sum) == 4.0, "constexpr 2 + 2 == 4");

		// Subtraction: 4 - 2 = 2.
		constexpr tk16 c = tk16(4.0);
		constexpr tk16 d = tk16(2.0);
		constexpr tk16 diff = c - d;
		static_assert(static_cast<double>(diff) == 2.0, "constexpr 4 - 2 == 2");

		// Multiplication: 2 * 4 = 8.
		constexpr tk16 prod = a * c;
		static_assert(static_cast<double>(prod) == 8.0, "constexpr 2 * 4 == 8");

		// Division: 8 / 2 = 4.
		constexpr tk16 e = tk16(8.0);
		constexpr tk16 ratio = e / d;
		static_assert(static_cast<double>(ratio) == 4.0, "constexpr 8 / 2 == 4");

		// Negation.
		constexpr tk16 neg = -a;
		static_assert(static_cast<double>(neg) == -2.0, "constexpr -tk16(2) == -2");
		static_assert(neg.isneg(),   "constexpr negation flips sign");
		static_assert(neg.sign(),    "constexpr negation sets sign() bit");
	}

	// ----------------------------------------------------------------------------
	// Compound assignment forms (operator+= -= *= /=).  Lambda-driven so we
	// can exercise the assignment in a constexpr context and inspect the
	// final value.
	// ----------------------------------------------------------------------------
	{
		constexpr tk16 cx_plus_eq = []() {
			tk16 x{ 2.0 };
			x += tk16(2.0);
			return x;
		}();
		static_assert(static_cast<double>(cx_plus_eq) == 4.0, "constexpr (a += b) leaves a = 4");

		constexpr tk16 cx_mul_eq = []() {
			tk16 x{ 2.0 };
			x *= tk16(4.0);
			return x;
		}();
		static_assert(static_cast<double>(cx_mul_eq) == 8.0, "constexpr (a *= b) leaves a = 8");

		constexpr tk16 cx_div_eq = []() {
			tk16 x{ 8.0 };
			x /= tk16(2.0);
			return x;
		}();
		static_assert(static_cast<double>(cx_div_eq) == 4.0, "constexpr (a /= b) leaves a = 4");
	}

	// ----------------------------------------------------------------------------
	// Comparison operators at constant evaluation.  takum uses two's
	// complement encoding so ordered comparison reduces to signed integer
	// comparison on the raw bits.
	// ----------------------------------------------------------------------------
	{
		constexpr tk16 a = tk16(1.0);
		constexpr tk16 b = tk16(2.0);
		constexpr tk16 c = tk16(1.0);

		static_assert(a == c,  "constexpr equal takums compare ==");
		static_assert(a != b,  "constexpr unequal takums compare !=");
		static_assert(a <  b,  "constexpr a < b for a=1 b=2");
		static_assert(b >  a,  "constexpr b > a for a=1 b=2");
		static_assert(a <= b,  "constexpr a <= b");
		static_assert(b >= a,  "constexpr b >= a");
		static_assert(a <= c,  "constexpr a <= c (equal)");
		static_assert(a >= c,  "constexpr a >= c (equal)");

		// NaR comparisons: all return false except !=.
		constexpr tk16 nar(SpecificValue::nar);
		static_assert(!(nar == nar),  "constexpr NaR == NaR is false (NaN semantics)");
		static_assert(  nar != nar,   "constexpr NaR != NaR is true (NaN semantics)");
		static_assert(!(nar <  a),    "constexpr NaR < a is false");
		static_assert(!(nar <= a),    "constexpr NaR <= a is false");
		static_assert(!(nar >  a),    "constexpr NaR > a is false");
		static_assert(!(nar >= a),    "constexpr NaR >= a is false");
	}

	// ----------------------------------------------------------------------------
	// Increment / decrement: navigate the lattice of representable values.
	// ----------------------------------------------------------------------------
	{
		constexpr tk16 cx_inc = []() {
			tk16 x{ 1.0 };
			++x;
			return x;
		}();
		// post-increment from tk16(1.0) lands on the next-larger representable
		// takum (some value just above 1.0); we just verify it's strictly
		// greater than 1.0.
		static_assert(cx_inc > tk16(1.0), "constexpr ++x > x");

		constexpr tk16 cx_dec = []() {
			tk16 x{ 1.0 };
			--x;
			return x;
		}();
		static_assert(cx_dec < tk16(1.0), "constexpr --x < x");

		// Saturation: ++maxpos stays at maxpos.
		constexpr tk16 cx_inc_sat = []() {
			tk16 x(SpecificValue::maxpos);
			++x;
			return x;
		}();
		static_assert(cx_inc_sat == tk16(SpecificValue::maxpos),
		              "constexpr ++maxpos saturates to maxpos");

		// Saturation: --maxneg stays at maxneg.
		constexpr tk16 cx_dec_sat = []() {
			tk16 x(SpecificValue::maxneg);
			--x;
			return x;
		}();
		static_assert(cx_dec_sat == tk16(SpecificValue::maxneg),
		              "constexpr --maxneg saturates to maxneg");
	}

	// ----------------------------------------------------------------------------
	// abs(): pure constexpr free function.
	// ----------------------------------------------------------------------------
	{
		constexpr tk16 a = tk16(-2.0);
		constexpr tk16 abs_a = abs(a);
		static_assert(abs_a.ispos(),                 "constexpr abs(-2) is positive");
		static_assert(static_cast<double>(abs_a) == 2.0, "constexpr abs(-2) == 2");

		constexpr tk16 b = tk16(2.0);
		constexpr tk16 abs_b = abs(b);
		static_assert(abs_b == b, "constexpr abs(positive) == positive");

		constexpr tk16 z = tk16(0.0);
		constexpr tk16 abs_z = abs(z);
		static_assert(abs_z.iszero(), "constexpr abs(zero) == zero");
	}

	// ----------------------------------------------------------------------------
	// Wider-storage configuration smoke test (rbits=3, uint32_t single-block).
	// ----------------------------------------------------------------------------
	{
		constexpr tk24w one(1.0);
		static_assert(static_cast<double>(one) == 1.0, "constexpr tk24w(1.0) -> 1.0");

		constexpr tk24w sum = tk24w(2.0) + tk24w(2.0);
		static_assert(static_cast<double>(sum) == 4.0, "constexpr tk24w 2+2 == 4");

		constexpr tk24w prod = tk24w(2.0) * tk24w(2.0);
		static_assert(static_cast<double>(prod) == 4.0, "constexpr tk24w 2*2 == 4");
	}

	std::cout << "takum constexpr verification: "
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
