// constexpr.cpp: compile-time tests for adaptive precision rational (erational)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Scope of this constexpr coverage (per issue #749, parent epic #723):
//
//   * default construction: is_constant_evaluated() dispatch keeps
//     numerator/denominator as default-empty edecimals at constant
//     evaluation; setzero() is called at runtime to preserve the
//     historical "0/1" representation
//   * defaulted copy / move ctors and assignment operators (rely on
//     edecimal's now-constexpr defaults from #824)
//   * selectors that delegate to constexpr-clean edecimal selectors
//     or read trivial bool members:
//       iszero, isneg, ispos, isinf, isnan, sign, top, bottom
//   * sign-only modifiers: setsign, setneg, setpos
//
// Out of scope (deferred; see comment block in erational_impl.hpp):
//
//   * native-type ctors and operator=        - chain to convert_signed/
//                                              unsigned/ieee754 -> edecimal
//                                              heap operations
//   * arithmetic operators (+= -= *= /=)     - chain through edecimal
//                                              arithmetic which is
//                                              non-constexpr
//   * unary -, ++, --                        - allocate temporaries
//   * conversion-out (operator double, etc.) - uses edecimal arithmetic
//   * free comparison operators              - depend on non-constexpr
//                                              edecimal == and <
//                                              (edecimal arithmetic is
//                                              still pre-constexpr)
//   * setbits, setnumerator, setdenominator  - heap mutation
//   * parse() / normalize()                  - regex / arithmetic
//
// erational is composed of two edecimal members; the constexpr surface
// is therefore intersected with edecimal's own constexpr surface.  When
// edecimal arithmetic / comparison get promoted in a follow-up, the
// erational arithmetic and comparison surface will inherit constexpr-
// ness automatically.

#include <universal/utility/directives.hpp>

#include <iostream>
#include <iomanip>
#include <universal/number/erational/erational.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "erational constexpr verification";
	std::string test_tag    = "constexpr";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// ----------------------------------------------------------------------------
	// Default construction at constant evaluation: numerator/denominator
	// are default-constructed empty edecimals; iszero() recognizes the
	// empty numerator state via edecimal::iszero().
	// ----------------------------------------------------------------------------
	{
		constexpr erational z{};
		static_assert( z.iszero(),  "constexpr default-constructed erational is zero (numerator empty)");
		// Note: at constant evaluation BOTH numerator and denominator are
		// empty, so isnan() (defined as numerator.iszero() && denominator.iszero())
		// returns true.  This is the deliberate consequence of the empty-
		// vector constexpr representation: the runtime default ctor calls
		// setzero() which sets denominator = 1, but that path heap-allocates
		// and is not available at constant evaluation under C++20's
		// transient-allocation rule.  Users who need a non-NaN constexpr
		// erational must wait for edecimal arithmetic to become constexpr
		// (which would let them initialize denominator = 1 at compile time).
		static_assert( z.isnan(),   "constexpr default-constructed erational has empty num+denom -> isnan() at compile time");
		static_assert(!z.isinf(),   "constexpr default-constructed erational !isinf()");
		static_assert(!z.sign(),    "constexpr default-constructed erational sign() == false");
		static_assert(!z.isneg(),   "constexpr default-constructed erational !isneg()");
		static_assert( z.ispos(),   "constexpr default-constructed erational ispos()");
	}

	// ----------------------------------------------------------------------------
	// Sign-only modifiers at constant evaluation: setsign, setneg, setpos
	// operate purely on the bool member (no edecimal touch).
	// ----------------------------------------------------------------------------
	{
		// setsign(true) round-trip.
		constexpr erational neg = []() {
			erational t{};
			t.setsign(true);
			return t;
		}();
		static_assert( neg.sign(),    "constexpr setsign(true) -> sign() == true");
		static_assert( neg.isneg(),   "constexpr setsign(true) -> isneg()");
		static_assert(!neg.ispos(),   "constexpr setsign(true) -> !ispos()");

		// setneg() then setpos() round-trip.
		constexpr erational flipped = []() {
			erational t{};
			t.setneg();
			t.setpos();
			return t;
		}();
		static_assert(!flipped.sign(),  "constexpr setneg+setpos -> sign() == false");
		static_assert( flipped.ispos(), "constexpr setneg+setpos -> ispos()");
	}

	// ----------------------------------------------------------------------------
	// Defaulted copy and move constructors / assignment operators.
	// Backed by edecimal's defaulted copy/move which are constexpr per #824.
	// ----------------------------------------------------------------------------
	{
		constexpr erational src{};
		constexpr erational copied{ src };
		static_assert( copied.iszero(), "constexpr copy ctor preserves zero");
		static_assert(!copied.sign(),   "constexpr copy ctor preserves sign");

		constexpr erational assigned = []() {
			erational dst{};
			erational s{};
			s.setsign(true);
			dst = s;
			return dst;
		}();
		static_assert( assigned.iszero(), "constexpr copy assignment preserves zero");
		static_assert( assigned.isneg(),  "constexpr copy assignment carries sign");
	}

	// ----------------------------------------------------------------------------
	// is_erational trait is itself a constexpr variable template.
	// ----------------------------------------------------------------------------
	{
		static_assert( is_erational<erational>,    "is_erational<erational> == true");
		static_assert(!is_erational<int>,          "is_erational<int> == false");
		static_assert(!is_erational<double>,       "is_erational<double> == false");
	}

	std::cout << "erational constexpr verification: "
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
