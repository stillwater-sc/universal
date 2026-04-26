// constexpr.cpp: compile-time tests for lns
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure lns: throwing arithmetic exceptions disabled so divide-by-zero
// has defined constexpr-safe behavior (returns NaN encoding) rather than
// throwing, which would be ill-formed in a constant expression.
#define LNS_THROW_ARITHMETIC_EXCEPTION 0

#include <iostream>
#include <iomanip>
#include <universal/number/lns/lns.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "lns constexpr verification";
	std::string test_tag    = "constexpr";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// ----------------------------------------------------------------------------
	// Native int construction (already constexpr -- smoke test only)
	// ----------------------------------------------------------------------------
	{
		using L16_8 = lns<16, 8, std::uint16_t>;
		constexpr L16_8 a(0);
		constexpr L16_8 b(2);
		constexpr L16_8 c(-3);
		constexpr L16_8 d(1000);
		(void)a; (void)b; (void)c; (void)d;
	}

	// ----------------------------------------------------------------------------
	// Acceptance form from issue #722:
	//   constexpr lns<16,8> a(2.0), b(3.0); constexpr auto c = a * b;
	// Multiplication is integer add in the log domain -- the simplest case.
	// ----------------------------------------------------------------------------
	{
		using L16_8 = lns<16, 8, std::uint16_t>;
		constexpr L16_8 a(2.0);   // float construction via cm::log2
		constexpr L16_8 b(3.0);
		constexpr auto cx_prod = a * b;        // log-domain integer add -> 6.0
		(void)cx_prod;
	}

	// ----------------------------------------------------------------------------
	// All four arithmetic operators in constexpr context
	// ----------------------------------------------------------------------------
	{
		using L16_8 = lns<16, 8, std::uint16_t>;
		constexpr L16_8 a(4.0);
		constexpr L16_8 b(2.0);
		constexpr auto cx_sum  = a + b;        // double-trip arithmetic
		constexpr auto cx_diff = a - b;
		constexpr auto cx_prod = a * b;        // log-domain integer add
		constexpr auto cx_quot = a / b;        // log-domain integer sub
		constexpr auto cx_neg  = -a;
		(void)cx_sum; (void)cx_diff; (void)cx_prod; (void)cx_quot; (void)cx_neg;

		// Compound assignment via lambda (constexpr lambdas are C++20)
		constexpr L16_8 cx_addeq = []() { L16_8 t(4.0); t += L16_8(2.0); return t; }();
		constexpr L16_8 cx_muleq = []() { L16_8 t(4.0); t *= L16_8(2.0); return t; }();
		(void)cx_addeq; (void)cx_muleq;
	}

	// ----------------------------------------------------------------------------
	// Constexpr comparison
	// ----------------------------------------------------------------------------
	{
		using L16_8 = lns<16, 8, std::uint16_t>;
		constexpr L16_8 a(2.0);
		constexpr L16_8 b(3.0);
		static_assert(a < b,    "constexpr: lns(2) < lns(3)");
		static_assert(b > a,    "constexpr: lns(3) > lns(2)");
		static_assert(!(a == b), "constexpr: lns(2) != lns(3)");
		static_assert(a == a,    "constexpr: lns(2) == lns(2)");
	}

	// ----------------------------------------------------------------------------
	// Conversion-out: operator double() through to_ieee754<double>() now uses
	// cm::exp2 instead of std::pow.
	// ----------------------------------------------------------------------------
	{
		using L16_8 = lns<16, 8, std::uint16_t>;
		constexpr L16_8 a(2.0);
		constexpr double v = double(a);
		// lns has limited precision, so cm::exp2 round-trip won't give exact 2.0
		// unless the value exactly maps to a representable lns encoding.
		// 2.0 maps exactly: log2(2) = 1, lns encodes 1 in fixed-point.
		// exp2(1) = 2.0 exact.
		static_assert(v > 1.99 && v < 2.01, "lns(2.0) -> double round-trip");

		constexpr L16_8 b(0.5);
		constexpr double bv = double(b);  // log2(0.5) = -1, exp2(-1) = 0.5
		static_assert(bv > 0.49 && bv < 0.51, "lns(0.5) -> double round-trip");
	}

	// ----------------------------------------------------------------------------
	// Saturating variant: maxpos/maxneg construction at compile time + pinned
	// behavior on overflow.
	// ----------------------------------------------------------------------------
	{
		using LSat = lns<8, 4, std::uint8_t, Behavior::Saturating>;
		constexpr LSat maxpos(SpecificValue::maxpos);
		constexpr LSat maxneg(SpecificValue::maxneg);
		(void)maxpos; (void)maxneg;
	}

	// ----------------------------------------------------------------------------
	// Edge case: divide-by-zero under LNS_THROW_ARITHMETIC_EXCEPTION=0 returns
	// NaN encoding (constexpr-safe -- no throw).
	// ----------------------------------------------------------------------------
	{
		using L16_8 = lns<16, 8, std::uint16_t>;
		constexpr L16_8 div_zero = []() {
			L16_8 t(2.0);
			t /= L16_8(0.0);
			return t;
		}();
		static_assert(div_zero.isnan(), "constexpr: divide-by-zero returns NaN");
	}

	std::cout << "lns constexpr verification: PASS\n";

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const sw::universal::lns_arithmetic_exception& err) {
	std::cerr << "Uncaught lns arithmetic exception: " << err.what() << '\n';
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
