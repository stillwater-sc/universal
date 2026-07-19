// next.cpp: regression tests for efloat next-representable-value functions.
//
// nextafter(x, y) returns the value one ULP from x toward y, at x's WORKING
// precision. Covers direction, precision-consistent ULP magnitude (the property
// a stored-limb-count ULP got wrong), within-binade reversibility, zero/sign,
// and special values. nexttoward mirrors nextafter with a long double target.
// (Issue #1120)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

int VerifyEfloatNext(bool reportTestCases) {
	using namespace sw::universal;
	using E      = efloat<8>;   // 256-bit working precision
	using EH     = efloat<16>;  // 512-bit working precision
	int failures = 0;

	// ---------------------------------------------------------------------
	// 1. direction: step toward y lands strictly on the y side of x
	// ---------------------------------------------------------------------
	if (reportTestCases)
		std::cout << "  Verifying step direction...\n";
	{
		E one(1.0);
		if (!(nextafter(one, E(2.0)) > one)) {
			if (reportTestCases)
				std::cout << "    FAIL: nextafter(1,2) !> 1\n";
			++failures;
		}
		if (!(nextafter(one, E(0.0)) < one)) {
			if (reportTestCases)
				std::cout << "    FAIL: nextafter(1,0) !< 1\n";
			++failures;
		}
		E neg(-1.0);
		if (!(nextafter(neg, E(-2.0)) < neg)) {
			if (reportTestCases)
				std::cout << "    FAIL: nextafter(-1,-2) !< -1\n";
			++failures;
		}
		if (!(nextafter(neg, E(0.0)) > neg)) {
			if (reportTestCases)
				std::cout << "    FAIL: nextafter(-1,0) !> -1\n";
			++failures;
		}
	}

	// ---------------------------------------------------------------------
	// 2. x == y returns y unchanged
	// ---------------------------------------------------------------------
	if (reportTestCases)
		std::cout << "  Verifying x == y short-circuit...\n";
	{
		E x(3.5);
		if (nextafter(x, x) != x) {
			if (reportTestCases)
				std::cout << "    FAIL: nextafter(x,x) != x\n";
			++failures;
		}
	}

	// ---------------------------------------------------------------------
	// 3. ULP magnitude tracks WORKING precision (the fixed bug: it used to
	//    key off the stored limb count, giving a fixed 2^-31 step).
	//    step = 2^(scale(x) - precision + 1); for x = 1.0 (scale 0) that is
	//    2^-255 at 256-bit and 2^-511 at 512-bit.
	// ---------------------------------------------------------------------
	if (reportTestCases)
		std::cout << "  Verifying precision-consistent ULP...\n";
	{
		E       x8(1.0);
		E       step8 = nextafter(x8, E(2.0)) - x8;
		int64_t want8 = x8.scale() - static_cast<int64_t>(x8.get_precision()) + 1;  // -255
		if (step8.scale() != want8) {
			if (reportTestCases)
				std::cout << "    FAIL: efloat<8> ULP scale=" << step8.scale() << " (want " << want8 << ")\n";
			++failures;
		}
		EH      x16(1.0);
		EH      step16 = nextafter(x16, EH(2.0)) - x16;
		int64_t want16 = x16.scale() - static_cast<int64_t>(x16.get_precision()) + 1;  // -511
		if (step16.scale() != want16) {
			if (reportTestCases)
				std::cout << "    FAIL: efloat<16> ULP scale=" << step16.scale() << " (want " << want16 << ")\n";
			++failures;
		}
		// higher precision => strictly smaller step
		if (!(step16.scale() < step8.scale())) {
			if (reportTestCases)
				std::cout << "    FAIL: efloat<16> ULP not smaller than efloat<8>\n";
			++failures;
		}
	}

	// ---------------------------------------------------------------------
	// 4. within-binade reversibility: step up then down returns x exactly.
	//    (Down across the binade boundary halves the ULP, so we only test the
	//    up-then-down direction, which stays inside [1,2).)
	// ---------------------------------------------------------------------
	if (reportTestCases)
		std::cout << "  Verifying up-then-down reversibility...\n";
	{
		EH x(1.0);
		EH up   = nextafter(x, EH(2.0));
		EH back = nextafter(up, EH(0.0));
		if (back != x) {
			if (reportTestCases)
				std::cout << "    FAIL: nextafter(nextafter(1,up),down) != 1\n";
			++failures;
		}
	}

	// ---------------------------------------------------------------------
	// 5. zero: step away from zero is a tiny value on the correct side
	// ---------------------------------------------------------------------
	if (reportTestCases)
		std::cout << "  Verifying step from zero...\n";
	{
		E zero(0.0);
		E zu = nextafter(zero, E(1.0));
		E zd = nextafter(zero, E(-1.0));
		if (!(zu > zero) || zu.scale() != -1074) {
			if (reportTestCases)
				std::cout << "    FAIL: nextafter(0,+) scale=" << zu.scale() << "\n";
			++failures;
		}
		if (!(zd < zero) || zd.sign() != -1) {
			if (reportTestCases)
				std::cout << "    FAIL: nextafter(0,-) not negative\n";
			++failures;
		}
	}

	// ---------------------------------------------------------------------
	// 6. special values: NaN in either argument -> NaN
	// ---------------------------------------------------------------------
	if (reportTestCases)
		std::cout << "  Verifying special values...\n";
	{
		E nan;
		nan.setnan();
		if (!nextafter(nan, E(1.0)).isnan()) {
			if (reportTestCases)
				std::cout << "    FAIL: nextafter(NaN,1) != NaN\n";
			++failures;
		}
		if (!nextafter(E(1.0), nan).isnan()) {
			if (reportTestCases)
				std::cout << "    FAIL: nextafter(1,NaN) != NaN\n";
			++failures;
		}
	}

	// ---------------------------------------------------------------------
	// 7. nexttoward mirrors nextafter with a long double target
	// ---------------------------------------------------------------------
	if (reportTestCases)
		std::cout << "  Verifying nexttoward...\n";
	{
		E x(1.0);
		if (nexttoward(x, 2.0L) != nextafter(x, E(2.0))) {
			if (reportTestCases)
				std::cout << "    FAIL: nexttoward != nextafter(2)\n";
			++failures;
		}
		if (nexttoward(x, 0.0L) != nextafter(x, E(0.0))) {
			if (reportTestCases)
				std::cout << "    FAIL: nexttoward != nextafter(0)\n";
			++failures;
		}
		if (!(nexttoward(x, 2.0L) > x)) {
			if (reportTestCases)
				std::cout << "    FAIL: nexttoward(1,2) !> 1\n";
			++failures;
		}
	}

	return failures;
}

}  // anonymous namespace

#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 0
#	define REGRESSION_LEVEL_3 0
#	define REGRESSION_LEVEL_4 0
#endif

int main() try {
	using namespace sw::universal;

	std::string test_suite          = "efloat next representable value library";
	std::string test_tag            = "next";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyEfloatNext(reportTestCases), "efloat", "next manual");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyEfloatNext(reportTestCases), "efloat", "next foundational");
#	endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif
} catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
} catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
