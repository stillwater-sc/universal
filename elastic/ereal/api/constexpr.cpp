// constexpr.cpp: compile-time tests for adaptive precision multi-component real (ereal)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Scope of this constexpr coverage (per issue #750, parent epic #723):
//
//   * default construction (is_constant_evaluated() dispatch leaves
//     _limb empty at constant evaluation; runtime path pushes 0.0)
//   * defaulted copy / move ctors and assignment operators
//   * selectors that don't depend on non-constexpr stdlib helpers:
//       iszero, isone, ispos, isneg, sign, significant, limbs
//     (all empty-_limb-guarded so the compile-time empty-vector path
//     is well-defined; the runtime path always has _limb.size() >= 1)
//
// Out of scope (deferred; see comment block in ereal_impl.hpp):
//
//   * isnan / isinf / signbit / scale -- depend on non-constexpr
//                                        std::fpclassify / std::signbit /
//                                        sw::universal::scale
//   * native-type ctors and operator=  - convert_signed/unsigned/ieee754
//                                        all heap-mutate via clear() +
//                                        push_back
//   * setzero / setnan / setinf /      - clear + push_back; heap escape
//     maxpos / minpos / maxneg / minneg
//   * arithmetic operators (+= etc.)   - Shewchuk expansion arithmetic
//                                        mutates the digit vector
//   * unary -, conversion-out          - allocate or call non-constexpr
//                                        helpers
//   * free comparison operators        - compare_adaptive iterates the
//                                        expansion (non-constexpr)
//   * parse / assign / to_string       - regex / stringstream / frexp
//
// ereal is more complex than the other elastic types because Shewchuk's
// expansion arithmetic is the only available algorithm for normalization
// and ordering, and it itself is not constexpr-clean today.  This PR
// pins the construction + read-only-selector surface so static_assert
// usage is possible and a future arithmetic-constexpr promotion (likely
// after the expansion_ops module is itself promoted) inherits the
// existing contract.

#include <universal/utility/directives.hpp>

#include <iostream>
#include <iomanip>
#include <utility>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "ereal constexpr verification";
	std::string test_tag    = "constexpr";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// Default ereal uses maxlimbs = 8; also exercise maxlimbs = 4 to
	// confirm the constexpr surface is template-parameter-independent.
	using er4 = ereal<4>;
	using er8 = ereal<8>;

	// ----------------------------------------------------------------------------
	// Static structural invariants.
	// ----------------------------------------------------------------------------
	{
		static_assert(er4::maxNrLimbs == 4u, "constexpr er4::maxNrLimbs == 4");
		static_assert(er8::maxNrLimbs == 8u, "constexpr er8::maxNrLimbs == 8");
		static_assert(er8::EXP_BIAS == 1023, "constexpr ereal::EXP_BIAS == 1023");
	}

	// ----------------------------------------------------------------------------
	// Default construction at constant evaluation: _limb is empty;
	// iszero() recognizes the empty state as canonical zero.
	// ----------------------------------------------------------------------------
	{
		constexpr er8 z{};
		static_assert( z.iszero(),   "constexpr default-constructed ereal is zero");
		static_assert(!z.isone(),    "constexpr default-constructed ereal !isone()");
		static_assert(!z.ispos(),    "constexpr default-constructed ereal !ispos() (empty -> not strictly positive)");
		static_assert(!z.isneg(),    "constexpr default-constructed ereal !isneg()");
		static_assert( z.sign() == 1, "constexpr default-constructed ereal sign() == 1");
		static_assert( z.significant() == 0.0, "constexpr default-constructed ereal significant() == 0.0");
		static_assert( z.limbs().empty(), "constexpr default-constructed ereal limbs() is empty at compile time");
	}

	// ----------------------------------------------------------------------------
	// Defaulted copy and move constructors / assignment operators.
	// Backed by std::vector's constexpr defaulted copy/move (in C++20
	// for empty sources).
	// ----------------------------------------------------------------------------
	{
		constexpr er8 src{};
		constexpr er8 copied{ src };
		static_assert( copied.iszero(),         "constexpr copy ctor preserves zero");
		static_assert( copied.limbs().empty(),  "constexpr copy ctor preserves empty limbs");

		constexpr er8 assigned = []() {
			er8 dst{};
			er8 s{};
			dst = s;
			return dst;
		}();
		static_assert( assigned.iszero(),       "constexpr copy assignment preserves zero");

		constexpr er8 moved = []() {
			er8 s{};
			er8 d{ std::move(s) };
			return d;
		}();
		static_assert( moved.iszero(),          "constexpr move ctor preserves zero");

		constexpr er8 move_assigned = []() {
			er8 dst{};
			er8 s{};
			dst = std::move(s);
			return dst;
		}();
		static_assert( move_assigned.iszero(),  "constexpr move assignment preserves zero");
	}

	// ----------------------------------------------------------------------------
	// Template-parameter independence: same constexpr surface for er4 and er8.
	// ----------------------------------------------------------------------------
	{
		constexpr er4 z4{};
		static_assert( z4.iszero(),  "constexpr er4{}.iszero()");
		static_assert( z4.limbs().empty(), "constexpr er4{}.limbs() is empty at compile time");
	}

	// ----------------------------------------------------------------------------
	// is_ereal trait is itself a constexpr variable template.
	// ----------------------------------------------------------------------------
	{
		static_assert( is_ereal<er4>,           "is_ereal<er4> == true");
		static_assert( is_ereal<er8>,           "is_ereal<er8> == true");
		static_assert(!is_ereal<int>,           "is_ereal<int> == false");
		static_assert(!is_ereal<double>,        "is_ereal<double> == false");
	}

	std::cout << "ereal constexpr verification: "
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
