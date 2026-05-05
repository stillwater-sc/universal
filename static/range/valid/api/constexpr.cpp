// constexpr.cpp: compile-time tests for valid (interval arithmetic, posit endpoints)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Scope of this constexpr coverage (per issue #744):
//
//   * default construction, copy, move
//   * selectors (isopen / isclosed / isopenlower / isopenupper)
//   * clear() modifier (constexpr-clean via posit's overridden bitblock reset)
//   * compound arithmetic stubs (+= -= *= /=) -- currently no-ops, kept
//     constexpr so the surface lights up at constant evaluation today
//   * comparison operators (==, !=, <, <=, >, >=) -- currently stub-false,
//     kept constexpr so callers can use them in static_assert
//   * binary arithmetic free functions (+, -, *, /) -- compose from the
//     compound stubs
//
// Out of scope at constant evaluation today (transitive limits of posit1):
//
//   * setnar / setbits / setinclusive -- ultimately call std::bitset::set,
//     which is not constexpr until C++23 (we target C++20).  The methods
//     are still constexpr-marked so they become callable at constant
//     evaluation as soon as the toolchain catches up.
//   * native-type ctors / operator=(int|double|...)  -- _assign() builds
//     an internal::value<>, whose int / unsigned long long path runs
//     through std::bitset mutation.  Same blocker; will light up under
//     C++23 or after the bitblock storage gets a constexpr-clean rewrite.
//
// The full semantic implementation of interval arithmetic (intersection,
// hull, midpoint) is left to a follow-up; this file locks in the constexpr
// API surface contract.
#include <universal/utility/directives.hpp>

#include <iostream>
#include <iomanip>
#include <universal/number/valid/valid.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "valid constexpr verification";
	std::string test_tag    = "constexpr";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using v16 = valid<16, 1>;
	using v32 = valid<32, 2>;

	// ----------------------------------------------------------------------------
	// Static accessors and structural invariants.
	// ----------------------------------------------------------------------------
	{
		static_assert(v16::somebits == 10u, "constexpr v16::somebits == 10");
		static_assert(v32::somebits == 10u, "constexpr v32::somebits == 10");
	}

	// ----------------------------------------------------------------------------
	// Default construction is constexpr (zero endpoints, both ubits false).
	// ----------------------------------------------------------------------------
	{
		constexpr v16 v{};
		static_assert(!v.isopenlower(), "constexpr default v.isopenlower() == false");
		static_assert(!v.isopenupper(), "constexpr default v.isopenupper() == false");
		// isclosed() := lubit && uubit -> default == false.  isopen() := !isclosed() -> true.
		static_assert(!v.isclosed(),    "constexpr default v.isclosed() == false");
		static_assert(v.isopen(),       "constexpr default v.isopen() == true");
	}

	// ----------------------------------------------------------------------------
	// clear() modifier at constant evaluation.  posit::clear() is overridden
	// to use bitblock::reset() (which is constexpr in C++20), so the whole
	// path is constexpr-clean.
	// ----------------------------------------------------------------------------
	{
		// clear(): both ubits set to true -> isclosed() returns true.
		constexpr v16 cleared = []() {
			v16 t{};
			t.clear();
			return t;
		}();
		static_assert(cleared.isclosed(),     "constexpr clear() -> isclosed()");
		static_assert(!cleared.isopen(),      "constexpr clear() -> !isopen()");
		static_assert(cleared.isopenlower(),  "constexpr clear() -> lubit == true (open-lower flag)");
		static_assert(cleared.isopenupper(),  "constexpr clear() -> uubit == true (open-upper flag)");

		// Repeated clear: idempotent.
		constexpr v16 cleared_twice = []() {
			v16 t{};
			t.clear();
			t.clear();
			return t;
		}();
		static_assert(cleared_twice.isclosed(), "constexpr clear() x2 still isclosed()");
	}

	// ----------------------------------------------------------------------------
	// Compound arithmetic stubs (currently no-ops) and binary free functions
	// at constant evaluation.  We don't assert any value semantics here --
	// those are left to a follow-up that implements interval arithmetic --
	// only that the operations compile at constant evaluation.
	// ----------------------------------------------------------------------------
	{
		constexpr v16 a{};
		constexpr v16 b{};

		constexpr v16 plus_eq = []() {
			v16 x{};
			x += v16{};
			return x;
		}();
		(void)plus_eq;

		constexpr v16 minus_eq = []() {
			v16 x{};
			x -= v16{};
			return x;
		}();
		(void)minus_eq;

		constexpr v16 mul_eq = []() {
			v16 x{};
			x *= v16{};
			return x;
		}();
		(void)mul_eq;

		constexpr v16 div_eq = []() {
			v16 x{};
			x /= v16{};
			return x;
		}();
		(void)div_eq;

		constexpr v16 sum  = a + b;
		constexpr v16 diff = a - b;
		constexpr v16 prod = a * b;
		constexpr v16 quot = a / b;
		(void)sum; (void)diff; (void)prod; (void)quot;
	}

	// ----------------------------------------------------------------------------
	// Comparison operators at constant evaluation.  They currently return
	// false unconditionally per the stub implementation; the contract lock-in
	// here is that the surface is constant-evaluable.
	// ----------------------------------------------------------------------------
	{
		constexpr v16 a{};
		constexpr v16 b{};

		static_assert(!(a == b),   "constexpr stub operator== returns false");
		static_assert(  a != b,    "constexpr operator!= follows stub == result");
		static_assert(!(a < b),    "constexpr stub operator< returns false");
		static_assert(!(a > b),    "constexpr operator> derived from < (stub-false)");
		static_assert(  a <= b,    "constexpr operator<= derived from > (stub-true)");
		static_assert(  a >= b,    "constexpr operator>= derived from < (stub-true)");
	}

	// ----------------------------------------------------------------------------
	// Literal-rhs comparison operators (valid vs double) -- runtime
	// instantiation smoke.  Cannot run at constant evaluation today because
	// they delegate via valid<>(rhs) which goes through internal::value<>'s
	// double path (std::bitset mutation, blocker documented above).  But the
	// runtime instantiation pins the dependency direction so the bug CR
	// found in the original code ("operator>(valid, double) called the non-
	// existent operator<(double, valid)") cannot recur silently: if the
	// signatures get reverted, this code stops compiling.
	// ----------------------------------------------------------------------------
	{
		v16 a{};
		bool b1 = (a == 0.0); (void)b1;
		bool b2 = (a != 0.0); (void)b2;
		bool b3 = (a <  0.0); (void)b3;
		bool b4 = (a >  0.0); (void)b4;
		bool b5 = (a <= 0.0); (void)b5;
		bool b6 = (a >= 0.0); (void)b6;

		// Smoke that valid(long) is unambiguous.  The original code had
		// valid(long initial_value) but no operator=(long); overload
		// resolution among operator=(int|unsigned long long|double|long
		// double) was ambiguous.  Adding operator=(long) (this PR's CR
		// round 2 fix) makes the assignment well-formed.
		v16 along(static_cast<long>(7));
		(void)along;
	}

	// ----------------------------------------------------------------------------
	// Wider configuration smoke (32-bit posit endpoints).
	// ----------------------------------------------------------------------------
	{
		constexpr v32 v{};
		static_assert(v.isopen(),     "constexpr v32 default v.isopen() == true");
		static_assert(!v.isclosed(),  "constexpr v32 default v.isclosed() == false");

		constexpr v32 cleared = []() {
			v32 t{};
			t.clear();
			return t;
		}();
		static_assert(cleared.isclosed(), "constexpr v32 clear() -> isclosed()");
	}

	std::cout << "valid constexpr verification: "
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
