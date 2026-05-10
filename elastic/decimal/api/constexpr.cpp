// constexpr.cpp: compile-time tests for adaptive precision decimal integer (edecimal)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Scope of this constexpr coverage (per issue #746, parent epic #723):
//
//   * default construction (constant-evaluated branch produces an empty
//     std::vector<uint8_t> + negative=false; iszero() recognizes the
//     empty vector as zero)
//   * sign selectors:    sign(), isneg(), ispos()
//   * sign modifiers:    setsign(), setneg(), setpos()
//   * iszero() on the empty (constant-evaluated) state
//   * defaulted copy / move ctors and assignment operators
//
// Out of scope (deferred; see comment block in edecimal_impl.hpp):
//
//   * setzero / setdigit / setbits           - call push_back; heap-escape
//   * native-type ctors and operator=        - go through convert_integer
//                                              / convert_ieee754, both of
//                                              which call push_back
//   * compound arithmetic (+= -= *= /= %=)   - mutate the digit vector
//   * unary -, ++, --                        - allocate temporary edecimals
//   * conversion operators (to_long_long etc.) iterate the digit vector
//   * comparison operators (==, !=, <, ...)  - read the digit vector via
//                                              non-constexpr free funcs
//   * parse() / regex                        - regex is not constexpr
//
// The fundamental constraint is C++20's "transient allocation" rule:
// memory allocated in a constant expression cannot persist outside it.
// edecimal inherits from std::vector, so any non-empty digit storage
// disqualifies the object from being a constexpr variable.  This file
// locks in the surface that the language permits today; relaxation of
// the transient-allocation rule (P2738 / C++23) will lift the boundary.

#include <universal/utility/directives.hpp>

#include <iostream>
#include <iomanip>
#include <universal/number/edecimal/edecimal.hpp>
#include <universal/traits/edecimal_traits.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "edecimal constexpr verification";
	std::string test_tag    = "constexpr";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// ----------------------------------------------------------------------------
	// Default construction at constant evaluation: empty vector + sign=false.
	// iszero() recognizes the empty vector as canonical zero.
	// ----------------------------------------------------------------------------
	{
		constexpr edecimal z{};
		static_assert(z.iszero(),  "constexpr default-constructed edecimal is zero");
		static_assert(!z.sign(),   "constexpr default-constructed edecimal sign() == false");
		static_assert(!z.isneg(),  "constexpr default-constructed edecimal !isneg()");
		static_assert(z.ispos(),   "constexpr default-constructed edecimal ispos()");
	}

	// ----------------------------------------------------------------------------
	// Sign-only modifiers at constant evaluation: setsign, setneg, setpos
	// operate purely on the bool member (no vector mutation).
	// ----------------------------------------------------------------------------
	{
		// setsign(true) round-trip.
		constexpr edecimal neg = []() {
			edecimal t{};
			t.setsign(true);
			return t;
		}();
		static_assert(neg.sign(),   "constexpr setsign(true) -> sign() == true");
		static_assert(neg.isneg(),  "constexpr setsign(true) -> isneg()");
		static_assert(!neg.ispos(), "constexpr setsign(true) -> !ispos()");
		// iszero() ignores the sign bit; an empty vector with negative=true
		// is still zero.
		static_assert(neg.iszero(), "constexpr empty vector + sign=true -> iszero()");

		// setneg() / setpos() round-trip.
		constexpr edecimal flipped = []() {
			edecimal t{};
			t.setneg();
			t.setpos();
			return t;
		}();
		static_assert(flipped.ispos(), "constexpr setneg+setpos -> ispos()");
		static_assert(!flipped.sign(), "constexpr setneg+setpos -> sign() == false");

		// setsign(true) followed by setsign(false) is identity for sign-only
		// state; the vector remains empty.
		constexpr edecimal toggled = []() {
			edecimal t{};
			t.setsign(true);
			t.setsign(false);
			return t;
		}();
		static_assert(!toggled.sign(),  "constexpr setsign true->false -> sign() == false");
		static_assert(toggled.iszero(), "constexpr toggled is still zero");
	}

	// ----------------------------------------------------------------------------
	// Defaulted copy and move constructors / assignment operators are
	// constexpr (the underlying std::vector copy/move is constexpr in C++20
	// when applied to an empty source).
	// ----------------------------------------------------------------------------
	{
		constexpr edecimal src{};
		constexpr edecimal copied{ src };
		static_assert(copied.iszero(), "constexpr copy ctor preserves zero");
		static_assert(!copied.sign(),  "constexpr copy ctor preserves sign");

		// Copy assignment via lambda (so we can persist the result).
		constexpr edecimal assigned = []() {
			edecimal dst{};
			edecimal s{};
			s.setsign(true);
			dst = s;
			return dst;
		}();
		static_assert(assigned.iszero(), "constexpr copy assignment preserves zero");
		static_assert(assigned.isneg(),  "constexpr copy assignment carries sign over");
	}

	// ----------------------------------------------------------------------------
	// is_edecimal trait is itself a constexpr variable template.  This
	// also pins the spelling fix for the type alias (was is_edecimalal_trait).
	// ----------------------------------------------------------------------------
	{
		static_assert(is_edecimal<edecimal>,        "is_edecimal<edecimal> == true");
		static_assert(!is_edecimal<int>,            "is_edecimal<int> == false");
		static_assert(!is_edecimal<unsigned long>,  "is_edecimal<unsigned long> == false");
	}

	std::cout << "edecimal constexpr verification: "
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
