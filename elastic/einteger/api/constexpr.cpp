// constexpr.cpp: compile-time tests for adaptive precision integer (einteger)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Scope of this constexpr coverage (per issue #748, parent epic #723):
//
//   * default construction (already empty-vector init in einteger; just
//     constexpr-annotated -- iszero() recognizes the empty state via
//     the size() == 0 branch)
//   * defaulted copy / move ctors and assignment operators
//   * read-only selectors:
//       iszero, isone, isodd, iseven, ispos, isneg, sign, scale,
//       test, block, limbs, nbits, findMsb
//   * modifiers: clear, setzero, setsign
//   * free comparison operators (type vs type):
//       ==, !=, <, <=, >, >=
//
// Out of scope (deferred; see comment block in einteger_impl.hpp):
//
//   * native-type ctors and operator=        - chain to setbits ->
//                                              push_back / resize
//   * setbits / setblock                     - heap mutation
//   * compound arithmetic (+= -= *= /= %=)   - mutate the digit vector
//   * unary -, ++, --                        - allocate temporaries
//   * binary arithmetic (+, -, *, /, %)      - compose from compounds
//   * conversion-out (operator int, etc.)    - iterate the digit vector
//   * type-vs-literal comparisons / arith    - construct einteger from
//                                              literal -> heap allocation
//   * parse() / assign(string) / to_*()      - regex / stringstream
//
// The fundamental constraint is C++20's "transient allocation" rule:
// memory allocated in a constant expression cannot persist outside it.
// einteger inherits a std::vector member, so any non-empty digit
// storage disqualifies the object from being a constexpr variable.
// Unlike edecimal/efloat, einteger's default ctor was already empty-
// vector-initialized, so no is_constant_evaluated() dispatch is needed.

#include <universal/utility/directives.hpp>

#include <iostream>
#include <iomanip>
#include <universal/number/einteger/einteger.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "einteger constexpr verification";
	std::string test_tag    = "constexpr";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// Default BlockType is uint32_t; also exercise uint8_t and uint16_t
	// to confirm the constexpr surface is template-parameter-independent.
	using ei8  = einteger<std::uint8_t>;
	using ei16 = einteger<std::uint16_t>;
	using ei32 = einteger<std::uint32_t>;

	// ----------------------------------------------------------------------------
	// Static structural invariants.
	// ----------------------------------------------------------------------------
	{
		static_assert(ei8::bitsInBlock  == 8u,  "constexpr ei8::bitsInBlock == 8");
		static_assert(ei16::bitsInBlock == 16u, "constexpr ei16::bitsInBlock == 16");
		static_assert(ei32::bitsInBlock == 32u, "constexpr ei32::bitsInBlock == 32");
	}

	// ----------------------------------------------------------------------------
	// Default construction at constant evaluation: _block is empty,
	// _sign is false.  iszero() recognizes the empty vector as the
	// canonical zero.
	// ----------------------------------------------------------------------------
	{
		constexpr ei32 z{};
		static_assert( z.iszero(),   "constexpr default-constructed einteger is zero");
		static_assert(!z.isodd(),    "constexpr default-constructed einteger !isodd()");
		static_assert( z.iseven(),   "constexpr default-constructed einteger iseven()");
		static_assert( z.ispos(),    "constexpr default-constructed einteger ispos()");
		static_assert(!z.isneg(),    "constexpr default-constructed einteger !isneg()");
		static_assert(!z.sign(),     "constexpr default-constructed einteger sign() == false");
		static_assert( z.limbs() == 0u, "constexpr default-constructed einteger limbs() == 0");
		static_assert( z.nbits() == 0u, "constexpr default-constructed einteger nbits() == 0");
		static_assert( z.findMsb() == -1, "constexpr default-constructed einteger findMsb() == -1");
		static_assert( z.scale() == -1,   "constexpr default-constructed einteger scale() == -1");
		static_assert(!z.test(0),    "constexpr default-constructed einteger test(0) == false");
		static_assert( z.block(0) == 0u, "constexpr default-constructed einteger block(0) == 0 (out-of-bounds returns 0)");
	}

	// ----------------------------------------------------------------------------
	// Sign-only modifiers at constant evaluation: setsign, setzero, clear
	// operate purely on the bool member or on the vector's clear() (which
	// is constexpr in C++20 and a no-op on the empty vector).
	// ----------------------------------------------------------------------------
	{
		// setsign(true) round-trip.  iszero() ignores _sign; an empty
		// vector with negative=true is still zero in magnitude.
		constexpr ei32 neg = []() {
			ei32 t{};
			t.setsign(true);
			return t;
		}();
		static_assert( neg.sign(),    "constexpr setsign(true) -> sign() == true");
		static_assert( neg.isneg(),   "constexpr setsign(true) -> isneg()");
		static_assert(!neg.ispos(),   "constexpr setsign(true) -> !ispos()");
		static_assert( neg.iszero(),  "constexpr empty vector + sign=true -> iszero()");

		// setsign(false) default round-trip.
		constexpr ei32 toggled = []() {
			ei32 t{};
			t.setsign(true);
			t.setsign(false);
			return t;
		}();
		static_assert(!toggled.sign(),  "constexpr setsign true->false -> sign() == false");
		static_assert( toggled.iszero(),"constexpr toggled is still zero");

		// clear() and setzero() are no-ops on an already-empty einteger.
		constexpr ei32 cleared = []() {
			ei32 t{};
			t.setsign(true);
			t.clear();
			return t;
		}();
		static_assert( cleared.iszero(), "constexpr clear() preserves zero");
		static_assert(!cleared.sign(),   "constexpr clear() resets sign to false");

		constexpr ei32 zeroed = []() {
			ei32 t{};
			t.setsign(true);
			t.setzero();
			return t;
		}();
		static_assert( zeroed.iszero(), "constexpr setzero() restores zero state");
		static_assert(!zeroed.sign(),   "constexpr setzero() resets sign to false");
	}

	// ----------------------------------------------------------------------------
	// Defaulted copy and move constructors / assignment operators.
	// std::vector copy of an empty source is constexpr in C++20.
	// ----------------------------------------------------------------------------
	{
		constexpr ei32 src{};
		constexpr ei32 copied{ src };
		static_assert(copied.iszero(),   "constexpr copy ctor preserves zero");
		static_assert(!copied.sign(),    "constexpr copy ctor preserves sign");

		constexpr ei32 assigned = []() {
			ei32 dst{};
			ei32 s{};
			s.setsign(true);
			dst = s;
			return dst;
		}();
		static_assert( assigned.iszero(), "constexpr copy assignment preserves zero");
		static_assert( assigned.isneg(),  "constexpr copy assignment carries sign");
	}

	// ----------------------------------------------------------------------------
	// Free comparison operators (type vs type) at constant evaluation.
	// All operands here are zero (different sign), so comparisons reduce
	// to magnitude comparisons; einteger equality ignores sign.
	// ----------------------------------------------------------------------------
	{
		constexpr ei32 a{};
		constexpr ei32 b{};
		// Two empty einteger objects are equal (both zero); operator<
		// returns false; operator<= and operator>= are true.  Equality
		// is now sign-aware, with +0 == -0 as the only sign-mismatch
		// equality case.
		static_assert(  a == b,  "constexpr einteger == zero compare");
		static_assert(!(a != b), "constexpr einteger != zero compare");
		static_assert(!(a <  b), "constexpr einteger <  zero compare");
		static_assert(!(a >  b), "constexpr einteger >  zero compare");
		static_assert(  a <= b,  "constexpr einteger <= zero compare");
		static_assert(  a >= b,  "constexpr einteger >= zero compare");

		// +0 == -0: sign-mismatch equality is the only allowed case.
		constexpr ei32 minus_zero = []() {
			ei32 t{};
			t.setsign(true);
			return t;
		}();
		static_assert(  a == minus_zero,  "constexpr +0 == -0 (sign-aware equality preserves zero)");
		static_assert(!(a <  minus_zero), "constexpr +0 < -0 -> false");
		static_assert(!(a >  minus_zero), "constexpr +0 > -0 -> false");
	}

	// ----------------------------------------------------------------------------
	// Runtime sign-sensitive comparison checks: covers the non-constexpr
	// path (native-integer ctors construct via heap-allocating setbits)
	// that the constexpr block above cannot exercise.  Pins the fix to
	// the previously sign-blind comparison operators.
	// ----------------------------------------------------------------------------
	{
		ei32 neg(-1);
		ei32 pos(1);
		ei32 z(0);

		if (!(neg <  pos))   { ++nrOfFailedTestCases; std::cerr << "FAIL: -1 < 1\n"; }
		if (!(pos >  neg))   { ++nrOfFailedTestCases; std::cerr << "FAIL: 1 > -1\n"; }
		if (!(neg != pos))   { ++nrOfFailedTestCases; std::cerr << "FAIL: -1 != 1\n"; }
		if (  neg == pos)    { ++nrOfFailedTestCases; std::cerr << "FAIL: -1 should not == 1\n"; }
		if (!(neg <= pos))   { ++nrOfFailedTestCases; std::cerr << "FAIL: -1 <= 1\n"; }
		if (!(pos >= neg))   { ++nrOfFailedTestCases; std::cerr << "FAIL: 1 >= -1\n"; }

		// Ordering between two negatives: -2 < -1 (larger magnitude is more negative)
		ei32 neg2(-2);
		if (!(neg2 < neg))   { ++nrOfFailedTestCases; std::cerr << "FAIL: -2 < -1\n"; }
		if (!(neg > neg2))   { ++nrOfFailedTestCases; std::cerr << "FAIL: -1 > -2\n"; }

		// +0 == -0 at runtime
		ei32 nz; nz.setsign(true);
		if (!(z == nz))      { ++nrOfFailedTestCases; std::cerr << "FAIL: +0 == -0 at runtime\n"; }
	}

	// ----------------------------------------------------------------------------
	// Block-type independence: same constexpr surface for ei8 and ei16.
	// ----------------------------------------------------------------------------
	{
		constexpr ei8 z8{};
		static_assert(z8.iszero(),       "constexpr ei8{}.iszero()");
		static_assert(z8.limbs() == 0u,  "constexpr ei8{}.limbs() == 0");

		constexpr ei16 z16{};
		static_assert(z16.iszero(),      "constexpr ei16{}.iszero()");
		static_assert(z16.limbs() == 0u, "constexpr ei16{}.limbs() == 0");
	}

	// ----------------------------------------------------------------------------
	// is_einteger trait is itself a constexpr variable template.
	// ----------------------------------------------------------------------------
	{
		static_assert( is_einteger<ei8>,           "is_einteger<ei8> == true");
		static_assert( is_einteger<ei16>,          "is_einteger<ei16> == true");
		static_assert( is_einteger<ei32>,          "is_einteger<ei32> == true");
		static_assert(!is_einteger<int>,           "is_einteger<int> == false");
		static_assert(!is_einteger<double>,        "is_einteger<double> == false");
	}

	std::cout << "einteger constexpr verification: "
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
