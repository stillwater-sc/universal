// constexpr.cpp: compile-time tests for adaptive precision binary floating-point (efloat)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Scope of this constexpr coverage (per issue #747, parent epic #723):
//
//   * default construction (constant-evaluated branch produces an empty
//     std::vector<uint32_t> _limb + state=Zero; iszero() recognizes the
//     Zero state directly without inspecting _limb)
//   * defaulted copy / move ctor + assignment operators
//   * state-only and sign-only selectors:
//       iszero, isone, isodd, iseven, ispos, isneg,
//       isinf, isnan, isqnan, issnan, sign, scale
//   * modifiers:  clear(), setzero()
//   * unary minus, compound arithmetic stubs (+= -= *= /=)
//   * free comparison operators (==, !=, <, <=, >, >=)
//   * free binary arithmetic operators (+, -, *, /)
//
// The compound arithmetic and comparison operators are currently no-op
// stubs in efloat_impl.hpp; their constexpr promotion locks in the
// surface so the contract is usable in static_assert today and the real
// arithmetic semantics will inherit constexpr-ness automatically when
// implemented.
//
// Out of scope (deferred; see comment block in efloat_impl.hpp):
//
//   * native-type ctors and operator=        - convert_ieee754 calls
//                                              std::fpclassify, which is
//                                              not constexpr in C++20
//   * conversion-out (operator double, etc.) - uses std::pow
//   * parse() / assign(string)               - std::regex is not constexpr
//
// The fundamental constraint is C++20's "transient allocation" rule:
// memory allocated in a constant expression cannot persist outside it.
// efloat carries a std::vector member, so any non-empty digit storage
// disqualifies the object from being a constexpr variable.  The default
// ctor uses std::is_constant_evaluated() to keep parallel invariants:
// empty vector at constant evaluation; push_back(0) at runtime.

#include <universal/utility/directives.hpp>

#include <iostream>
#include <iomanip>
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "efloat constexpr verification";
	std::string test_tag    = "constexpr";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// Use a small nlimbs so the type instantiation is cheap; the constexpr
	// surface is independent of nlimbs since _limb stays empty at constant
	// evaluation regardless.
	using ef4 = efloat<4>;
	using ef8 = efloat<8>;

	// ----------------------------------------------------------------------------
	// Default construction at constant evaluation: _state = Zero, sign = false,
	// _exponent = 0, _limb empty.  iszero() recognizes the Zero state
	// directly without dereferencing _limb.
	// ----------------------------------------------------------------------------
	{
		constexpr ef4 z{};
		static_assert( z.iszero(),  "constexpr default-constructed efloat is zero");
		static_assert(!z.isnan(),   "constexpr default-constructed efloat !isnan()");
		static_assert(!z.isqnan(),  "constexpr default-constructed efloat !isqnan()");
		static_assert(!z.issnan(),  "constexpr default-constructed efloat !issnan()");
		static_assert(!z.isinf(),   "constexpr default-constructed efloat !isinf()");
		static_assert(!z.ispos(),   "constexpr default-constructed efloat ispos()==false (state==Zero, not Normal)");
		static_assert(!z.isneg(),   "constexpr default-constructed efloat isneg()==false (state==Zero, not Normal)");
		static_assert(!z.isone(),   "constexpr default-constructed efloat !isone()");
		static_assert(!z.isodd(),   "constexpr default-constructed efloat !isodd()");
		static_assert( z.iseven(),  "constexpr default-constructed efloat iseven()");
		static_assert( z.sign() == 1,  "constexpr default-constructed efloat sign() == 1 (positive zero)");
		static_assert( z.scale() == 0, "constexpr default-constructed efloat scale() == 0");
	}

	// ----------------------------------------------------------------------------
	// Modifiers at constant evaluation: clear() resets to a Normal-state
	// non-zero, setzero() restores Zero state.  Both are constexpr-clean
	// because std::vector::clear() is constexpr in C++20 and our default
	// ctor leaves _limb empty in constant-evaluated context.
	// ----------------------------------------------------------------------------
	{
		// clear() leaves the object in Normal state (so iszero() == false).
		constexpr ef4 cleared = []() {
			ef4 t{};
			t.clear();
			return t;
		}();
		static_assert(!cleared.iszero(),  "constexpr clear() -> state=Normal -> !iszero()");
		static_assert( cleared.ispos(),   "constexpr clear() -> state=Normal, sign=false -> ispos()");

		// setzero() restores Zero state (issue #747 drive-by: previously
		// setzero() left state=Normal, contradicting its name; the
		// constexpr smoke test would otherwise flag this as buggy).
		constexpr ef4 zeroed = []() {
			ef4 t{};
			t.clear();    // first push to Normal
			t.setzero();  // then back to Zero
			return t;
		}();
		static_assert(zeroed.iszero(),    "constexpr setzero() -> state=Zero -> iszero()");
		static_assert(!zeroed.ispos(),    "constexpr setzero() -> state=Zero -> !ispos()");
	}

	// ----------------------------------------------------------------------------
	// Defaulted copy and move constructors / assignment operators.
	// Std::vector copy of an empty source is constexpr in C++20.
	// ----------------------------------------------------------------------------
	{
		constexpr ef4 src{};
		constexpr ef4 copied{ src };
		static_assert(copied.iszero(),   "constexpr copy ctor preserves zero");

		constexpr ef4 assigned = []() {
			ef4 dst{};
			ef4 s{};
			dst = s;
			return dst;
		}();
		static_assert(assigned.iszero(), "constexpr copy assignment preserves zero");
	}

	// ----------------------------------------------------------------------------
	// Unary minus at constant evaluation.  Returns a copy of *this; on the
	// empty default-constructed source the copy stays empty too.
	// ----------------------------------------------------------------------------
	{
		constexpr ef4 z{};
		constexpr ef4 neg = -z;
		// (negation is currently a stub that just copies; the surface is
		// promoted to constexpr to lock in the contract.)
		static_assert(neg.iszero(), "constexpr -z preserves zero state on stub negation");
	}

	// ----------------------------------------------------------------------------
	// Compound arithmetic stubs: marked constexpr, return *this unchanged.
	// ----------------------------------------------------------------------------
	{
		constexpr ef4 plus_eq = []() {
			ef4 x{};
			ef4 y{};
			x += y;
			return x;
		}();
		static_assert(plus_eq.iszero(), "constexpr += stub leaves zero unchanged");

		constexpr ef4 minus_eq = []() {
			ef4 x{};
			ef4 y{};
			x -= y;
			return x;
		}();
		static_assert(minus_eq.iszero(), "constexpr -= stub leaves zero unchanged");

		constexpr ef4 times_eq = []() {
			ef4 x{};
			ef4 y{};
			x *= y;
			return x;
		}();
		static_assert(times_eq.iszero(), "constexpr *= stub leaves zero unchanged");

		constexpr ef4 div_eq = []() {
			ef4 x{};
			ef4 y{};
			x /= y;
			return x;
		}();
		static_assert(div_eq.isnan(), "constexpr /= on 0/0 produces NaN");
	}

	// ----------------------------------------------------------------------------
	// Free comparison operators (currently stubs returning constants).
	// ----------------------------------------------------------------------------
	{
		constexpr ef4 a{};
		constexpr ef4 b{};
		static_assert(  a == b,  "constexpr efloat == stub returns true");
		static_assert(!(a != b), "constexpr efloat != stub returns false");
		static_assert(!(a <  b), "constexpr efloat <  stub returns false");
		static_assert(!(a >  b), "constexpr efloat >  stub returns false");
		static_assert(  a <= b,  "constexpr efloat <= stub returns true (! < || ==)");
		static_assert(  a >= b,  "constexpr efloat >= stub returns true (! <)");
	}

	// ----------------------------------------------------------------------------
	// Free binary arithmetic operators compose from constexpr compounds.
	// ----------------------------------------------------------------------------
	{
		constexpr ef4 a{};
		constexpr ef4 b{};
		constexpr ef4 sum  = a + b;
		constexpr ef4 diff = a - b;
		constexpr ef4 prod = a * b;
		// Don't pin 0/0 to current stub semantics: real division will
		// produce NaN, so we only verify the expression is well-formed
		// at constant evaluation.
		[[maybe_unused]] constexpr ef4 quot = a / b;
		static_assert(sum.iszero(),  "constexpr efloat + stub returns zero (0+0=0 holds for real semantics too)");
		static_assert(diff.iszero(), "constexpr efloat - stub returns zero (0-0=0 holds for real semantics too)");
		static_assert(prod.iszero(), "constexpr efloat * stub returns zero (0*0=0 holds for real semantics too)");
	}

	// ----------------------------------------------------------------------------
	// Wider configuration smoke: confirm the constexpr surface is template-
	// parameter-independent (the heap-escape boundary is the same regardless
	// of nlimbs because _limb stays empty at constant evaluation).
	// ----------------------------------------------------------------------------
	{
		constexpr ef8 z{};
		static_assert(z.iszero(),       "constexpr ef8{}.iszero()");
		static_assert(ef8::maxNrLimbs == 8u, "constexpr ef8::maxNrLimbs == 8");
	}

	// ----------------------------------------------------------------------------
	// is_efloat trait is a constexpr variable template.
	// ----------------------------------------------------------------------------
	{
		static_assert( is_efloat<ef4>,           "is_efloat<ef4> == true");
		static_assert( is_efloat<ef8>,           "is_efloat<ef8> == true");
		static_assert(!is_efloat<int>,           "is_efloat<int> == false");
		static_assert(!is_efloat<double>,        "is_efloat<double> == false");
	}

	std::cout << "efloat constexpr verification: "
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
