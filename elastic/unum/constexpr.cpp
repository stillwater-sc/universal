// constexpr.cpp: compile-time tests for unum Type I (Gustafson 2015)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Scope of this constexpr coverage (per issue #743):
//
//   * default construction, copy, move
//   * bitwise selectors / modifiers (clear, setbit, setbits, at, ubit,
//     fsize, esize, sign, exponent, fraction, iszero, isnan, etc.)
//   * special-value helpers (minpos, maxpos, minneg, maxneg, qnan, snan)
//   * integer / float / double / long double construction (CONSTEXPRESSION;
//     constexpr only when sw::bit_cast is constexpr)
//
// Out of scope (deferred to a follow-up issue):
//
//   * to_double / to_float / operator double() (use std::ldexp; would need
//     a constexpr_math::ldexp helper, plus a value-domain rewrite)
//   * comparison operators (delegate to to_double)
//   * arithmetic operators (delegate to to_double)
//   * increment / decrement (use std::ldexp directly)
//
// Per the issue body, the variable-length encoding makes constexpr
// arithmetic non-trivial; this file locks in the construction and bitwise
// contract so that a follow-up can add arithmetic without re-litigating
// the basics.
#include <universal/utility/directives.hpp>

#include <iostream>
#include <iomanip>
#include <universal/number/unum/unum.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "unum Type I constexpr verification";
	std::string test_tag    = "constexpr";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// unum<2, 2>: esizesize=2, fsizesize=2 -- maxesize=4, maxfsize=3,
	// utagsize=5, maxbits=13.  Small enough to constant-evaluate every
	// loop bound; large enough to exercise non-trivial encodings.
	using u22 = unum<2, 2>;
	using u33 = unum<3, 3>;

	// ----------------------------------------------------------------------------
	// Static structural invariants (already constexpr; smoke test).
	// ----------------------------------------------------------------------------
	{
		static_assert(u22::maxesize == 4u,  "constexpr u22::maxesize == 4");
		static_assert(u22::maxfsize == 3u,  "constexpr u22::maxfsize == 3");
		static_assert(u22::utagsize == 5u,  "constexpr u22::utagsize == 5");
		static_assert(u22::maxbits  == 13u, "constexpr u22::maxbits == 1+4+3+2+2+1 == 13");

		static_assert(u33::maxesize == 8u,  "constexpr u33::maxesize == 8");
		static_assert(u33::maxfsize == 7u,  "constexpr u33::maxfsize == 7");
		static_assert(u33::utagsize == 7u,  "constexpr u33::utagsize == 7");
	}

	// ----------------------------------------------------------------------------
	// Default construction: zero state, all bitwise selectors agree.
	// ----------------------------------------------------------------------------
	{
		constexpr u22 z{};
		static_assert(z.iszero(),     "constexpr value-init unum is zero");
		static_assert(!z.isnan(),     "constexpr value-init unum is not NaN");
		static_assert(!z.sign(),      "constexpr value-init unum has sign() == false");
		static_assert(!z.ubit(),      "constexpr value-init unum has ubit() == false (exact)");
		static_assert(z.fsize() == 0u, "constexpr value-init fsize() == 0");
		static_assert(z.esize() == 0u, "constexpr value-init esize() == 0");
		static_assert(z.fraction() == 0ull, "constexpr value-init fraction() == 0");
		static_assert(z.exponent() == 0ull, "constexpr value-init exponent() == 0");
		static_assert(z.exact(),      "constexpr value-init exact() == true");
		static_assert(!z.isinf(),     "constexpr unum has no infinity (always !isinf)");
	}

	// ----------------------------------------------------------------------------
	// Bitwise modifiers at constant evaluation: clear, setbit, setbits,
	// setnan, setzero.
	// ----------------------------------------------------------------------------
	{
		// setnan: every bit set -> isnan() returns true.
		constexpr u22 nanv = []() {
			u22 t{};
			t.setnan();
			return t;
		}();
		static_assert(nanv.isnan(),    "constexpr setnan() -> isnan()");
		static_assert(!nanv.iszero(),  "constexpr setnan() -> !iszero()");
		static_assert(!nanv.exact(),   "constexpr setnan() has ubit() set -> !exact()");

		// setzero: clears all bits.
		constexpr u22 zv = []() {
			u22 t{};
			t.setnan();   // first set all bits
			t.setzero();  // then clear
			return t;
		}();
		static_assert(zv.iszero(),     "constexpr setzero() round-trip -> iszero()");
		static_assert(!zv.isnan(),     "constexpr setzero() round-trip -> !isnan()");

		// setbit at specific positions: ubit (pos 0), low bit of fsize (pos 1).
		constexpr u22 ubit_only = []() {
			u22 t{};
			t.setbit(0, true);  // ubit -> 1 (inexact)
			return t;
		}();
		static_assert(ubit_only.ubit(),       "constexpr setbit(0,true) -> ubit() == true");
		static_assert(!ubit_only.exact(),     "constexpr ubit set -> !exact()");
		static_assert(!ubit_only.iszero(),    "constexpr non-zero bit -> !iszero()");

		// setbits: drop a uint64_t pattern into the storage.  Bit 0 = ubit,
		// bits 1..2 = fsize_field, bits 3..4 = esize_field, etc.  Pattern
		// 0b00000_00_00_00_01 = ubit only.
		constexpr u22 setbits_ubit = []() {
			u22 t{};
			t.setbits(1ull);
			return t;
		}();
		static_assert(setbits_ubit.ubit(),    "constexpr setbits(1) -> ubit() == true");
		static_assert(!setbits_ubit.iszero(), "constexpr setbits(1) -> !iszero()");
	}

	// ----------------------------------------------------------------------------
	// Special-value free helpers (minpos / maxpos / minneg / maxneg / qnan /
	// snan): all marked constexpr in this PR.
	// ----------------------------------------------------------------------------
	{
		constexpr u22 mp = []() {
			u22 t{};
			minpos(t);
			return t;
		}();
		static_assert(!mp.iszero(),   "constexpr minpos -> !iszero()");
		static_assert(!mp.isnan(),    "constexpr minpos -> !isnan()");
		static_assert(!mp.sign(),     "constexpr minpos -> sign() == false");
		static_assert(mp.exact(),     "constexpr minpos -> exact()");

		constexpr u22 mxp = []() {
			u22 t{};
			maxpos(t);
			return t;
		}();
		static_assert(!mxp.iszero(),  "constexpr maxpos -> !iszero()");
		static_assert(!mxp.isnan(),   "constexpr maxpos -> !isnan()");
		static_assert(!mxp.sign(),    "constexpr maxpos -> sign() == false");
		static_assert(mxp.exact(),    "constexpr maxpos -> exact()");

		constexpr u22 mn = []() {
			u22 t{};
			minneg(t);
			return t;
		}();
		static_assert(mn.sign(),      "constexpr minneg -> sign() == true");
		static_assert(mn.isneg(),     "constexpr minneg -> isneg()");

		constexpr u22 mxn = []() {
			u22 t{};
			maxneg(t);
			return t;
		}();
		static_assert(mxn.sign(),     "constexpr maxneg -> sign() == true");
		static_assert(mxn.isneg(),    "constexpr maxneg -> isneg()");

		constexpr u22 qn = []() {
			u22 t{};
			qnan(t);
			return t;
		}();
		static_assert(qn.isnan(),     "constexpr qnan -> isnan()");

		constexpr u22 sn = []() {
			u22 t{};
			snan(t);
			return t;
		}();
		static_assert(sn.isnan(),     "constexpr snan -> isnan()");
	}

	// ----------------------------------------------------------------------------
	// Integer / float / double / long double construction at constant
	// evaluation.  CONSTEXPRESSION: requires sw::bit_cast to be constexpr
	// (BIT_CAST_IS_CONSTEXPR).  Modern gcc and clang both satisfy that.
	// ----------------------------------------------------------------------------
	{
		// 1.0 -> exact unum encoding.  Spot-check: not zero, not NaN,
		// positive, exact (ubit == 0).  We deliberately do NOT assert the
		// exact bit pattern because the smallest fitting encoding depends
		// on the (esizesize, fsizesize) configuration.
		constexpr u22 one_i = u22(1);
		static_assert(!one_i.iszero(),   "constexpr u22(1).iszero() == false");
		static_assert(!one_i.isnan(),    "constexpr u22(1).isnan() == false");
		static_assert(!one_i.sign(),     "constexpr u22(1) is positive");
		static_assert(one_i.exact(),     "constexpr u22(1) is exact");

		constexpr u22 two_d = u22(2.0);
		static_assert(!two_d.iszero(),   "constexpr u22(2.0).iszero() == false");
		static_assert(!two_d.isnan(),    "constexpr u22(2.0).isnan() == false");
		static_assert(!two_d.sign(),     "constexpr u22(2.0) is positive");
		static_assert(two_d.exact(),     "constexpr u22(2.0) is exact");

		// negative literal: sign bit set, still exact.
		constexpr u22 mhalf = u22(-0.5);
		static_assert(mhalf.sign(),      "constexpr u22(-0.5).sign()");
		static_assert(mhalf.isneg(),     "constexpr u22(-0.5).isneg()");
		static_assert(mhalf.exact(),     "constexpr u22(-0.5).exact()");

		// 0.0 -> all-zero encoding.
		constexpr u22 z_d = u22(0.0);
		static_assert(z_d.iszero(),      "constexpr u22(0.0).iszero()");

		// Float source.
		constexpr u22 four_f = u22(4.0f);
		static_assert(!four_f.iszero(),  "constexpr u22(4.0f).iszero() == false");
		static_assert(four_f.exact(),    "constexpr u22(4.0f).exact()");

		// NaN source -> NaN encoding.
		constexpr u22 nan_d = u22(std::numeric_limits<double>::quiet_NaN());
		static_assert(nan_d.isnan(),     "constexpr u22(NaN).isnan()");

		// +/-infinity -> NaN encoding (unum Type I has no infinity).
		constexpr u22 pinf = u22(std::numeric_limits<double>::infinity());
		static_assert(pinf.isnan(),      "constexpr u22(+inf).isnan()");

		constexpr u22 ninf = u22(-std::numeric_limits<double>::infinity());
		static_assert(ninf.isnan(),      "constexpr u22(-inf).isnan()");
	}

	// ----------------------------------------------------------------------------
	// Wider configuration smoke test (esizesize=3, fsizesize=3).
	// ----------------------------------------------------------------------------
	{
		constexpr u33 z{};
		static_assert(z.iszero(),    "constexpr u33{}.iszero()");

		constexpr u33 one = u33(1);
		static_assert(!one.iszero(), "constexpr u33(1).iszero() == false");
		static_assert(one.exact(),   "constexpr u33(1).exact()");
	}

	std::cout << "unum Type I constexpr verification: "
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
