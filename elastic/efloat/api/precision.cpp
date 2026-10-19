// precision.cpp: regression tests for efloat dynamic runtime precision management.
//
// REGRESSION_LEVEL convention (intensity progression):
//   LEVEL 1 -- all foundational hand-curated tests: algebraic invariants,
//              hostile arithmetic corner cases (round-to-even, massive
//              exponent gaps, complete overlap), subnormal boundaries,
//              IEEE 754 special values.
//   LEVEL 2 -- property-based fuzzer over random multi-component expansions
//              (~1,000 iterations per invariant).
//   LEVEL 3 -- same fuzzer at higher intensity (~100,000 iterations).
//   LEVEL 4 -- exhaustive fuzzer (~10,000,000 iterations).
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <limits>
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	// =========================================================================
	// LEVEL 1: foundational hand-curated tests
	// =========================================================================
	int VerifyEfloatPrecisionManagement(bool reportTestCases) {
		using namespace sw::universal;
		int nrOfFailedTestCases = 0;

		// ---------------------------------------------------------------------
		// 1. Global Default Precision
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Global default precision...\n";
		{
			set_default_precision(0);
			if (get_default_precision() != 0) {
				if (reportTestCases) std::cout << "    FAIL: initial default precision is not 0\n";
				++nrOfFailedTestCases;
			}
			set_default_precision(64);
			if (get_default_precision() != 64) {
				if (reportTestCases) std::cout << "    FAIL: default precision was not set to 64\n";
				++nrOfFailedTestCases;
			}
			// Newly constructed efloat should use default precision (64 bits = 2 limbs)
			efloat<4> a;
			if (a.get_precision() != 64) {
				if (reportTestCases) std::cout << "    FAIL: newly constructed efloat did not inherit default precision. Result: " << a.get_precision() << "\n";
				++nrOfFailedTestCases;
			}
			set_default_precision(0); // restore default
		}

		// ---------------------------------------------------------------------
		// 2. Per-Instance Precision Controls
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Per-instance precision...\n";
		{
			efloat<4> a;
			if (a.get_precision() != 128) { // static fallback 4 * 32 = 128
				if (reportTestCases) std::cout << "    FAIL: static fallback precision was incorrect\n";
				++nrOfFailedTestCases;
			}
			a.set_precision(32); // resize to 32 bits (1 limb)
			if (a.get_precision() != 32) {
				if (reportTestCases) std::cout << "    FAIL: setting instance precision failed\n";
				++nrOfFailedTestCases;
			}

			// Verify clear() retains instance-level narrowed precision
			a.clear();
			if (a.get_precision() != 32) {
				if (reportTestCases) std::cout << "    FAIL: clear() clobbered instance precision. Result: " << a.get_precision() << "\n";
				++nrOfFailedTestCases;
			}

			// Verify parse() respects instance-level narrowed precision (32 bits = 1 limb)
			parse("0.33333333333333333", a);
			if (a.get_precision() != 32) {
				if (reportTestCases) std::cout << "    FAIL: parse() clobbered instance precision. Result: " << a.get_precision() << "\n";
				++nrOfFailedTestCases;
			}
			if (a.bits().size() != 1) {
				if (reportTestCases) std::cout << "    FAIL: parse() exceeded narrowed precision limb size. Size: " << a.bits().size() << "\n";
				++nrOfFailedTestCases;
			}
		}

		// ---------------------------------------------------------------------
		// 3. Arithmetic Precision Escalation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Arithmetic precision escalation...\n";
		{
			efloat<4> a(1.0);
			efloat<4> b(1.0);
			a.set_precision(32); // 1 limb
			b.set_precision(96); // 3 limbs

			a += b; // Result precision should be std::max(32, 96) = 96
			if (a.get_precision() != 96) {
				if (reportTestCases) std::cout << "    FAIL: addition did not escalate precision. Result: " << a.get_precision() << "\n";
				++nrOfFailedTestCases;
			}

			efloat<4> c(2.0);
			efloat<4> d(3.0);
			c.set_precision(64); // 2 limbs
			d.set_precision(128); // 4 limbs

			c *= d; // Result precision should be std::max(64, 128) = 128
			if (c.get_precision() != 128) {
				if (reportTestCases) std::cout << "    FAIL: multiplication did not escalate precision. Result: " << c.get_precision() << "\n";
				++nrOfFailedTestCases;
			}
		}

		// ---------------------------------------------------------------------
		// 3b. Native values carry no precision (#1757)
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Native operands do not widen the result...\n";
		{
			efloat<100> x(1.0);
			x /= 3;
			x.set_precision(640);   // the caller asks for 640 of the type's 3200 bits
			const efloat<100> results[] = {
				x * efloat<100>(3), x / efloat<100>(7), x + efloat<100>(0.1), x - efloat<100>(2.5),
				x * 3, x / 7, x + 0.1, x - 2.5, 3 * x, 7.0 / x, 2 + x, 0.5 - x
			};
			for (const auto& r : results) {
				if (r.get_precision() != 640) {
					if (reportTestCases) std::cout << "    FAIL: x op literal widened x's 640 bits to " << r.get_precision() << "\n";
					++nrOfFailedTestCases;
				}
			}
			efloat<100> y(x);
			y *= 3; y /= 7; y += 1; y -= 0.25;
			if (y.get_precision() != 640) {
				if (reportTestCases) std::cout << "    FAIL: compound ops with literals widened x's precision to " << y.get_precision() << "\n";
				++nrOfFailedTestCases;
			}
			// a function of literals carries no precision either, as their arithmetic would not
			using E = efloat<100>;
			const E functions_of_literals[] = {
				sqrt(E(2)), cbrt(E(2)), exp(E(1)), log(E(3)), sin(E(1)), atan(E(2)), sinh(E(1)),
				pow(E(2), E(0.5)), atan2(E(1), E(2)), log1p(E(0.25)), expm1(E(0.125))
			};
			for (const auto& f : functions_of_literals) {
				const E r = x * f;
				if (f.carries_precision() || f.get_precision() != 3200 || r.get_precision() != 640) {
					if (reportTestCases) std::cout << "    FAIL: a function of literals carries " << f.get_precision() << " bits (" << (f.carries_precision() ? "explicit" : "none") << "), x * it has " << r.get_precision() << "\n";
					++nrOfFailedTestCases;
				}
			}
			if (!sqrt(x).carries_precision() || !pow(E(2), x).carries_precision()) {
				if (reportTestCases) std::cout << "    FAIL: a function of a value with a precision dropped it\n";
				++nrOfFailedTestCases;
			}
			// a function of x runs at x's precision too
			const efloat<100> e = exp(x), s = sqrt(x), h = sinh(x);
			if (e.get_precision() != 640 || s.get_precision() != 640 || h.get_precision() != 640) {
				if (reportTestCases) std::cout << "    FAIL: exp/sqrt/sinh of a 640-bit x returned " << e.get_precision() << "/" << s.get_precision() << "/" << h.get_precision() << " bits\n";
				++nrOfFailedTestCases;
			}
		}
		{
			// literals alone still compute at the type's precision, or at the default when set
			const efloat<4> third = efloat<4>(1) / efloat<4>(3);
			if (third.get_precision() != 128 || third.bits().size() != 4) {
				if (reportTestCases) std::cout << "    FAIL: 1/3 of two literals is not at the type's 128 bits: " << third.get_precision() << "\n";
				++nrOfFailedTestCases;
			}
			set_default_precision(64);
			const efloat<4> third64 = efloat<4>(1) / efloat<4>(3);
			if (third64.get_precision() != 64 || third64.bits().size() != 2) {
				if (reportTestCases) std::cout << "    FAIL: 1/3 of two literals ignored the default precision: " << third64.get_precision() << "\n";
				++nrOfFailedTestCases;
			}
			set_default_precision(0);
		}
		{
			// a value without precision rounds to its partner's, including through the
			// power-of-two and zero shortcuts
			efloat<4> one(1.0);
			one.set_precision(32);
			const efloat<4> tenth = efloat<4>(1) / efloat<4>(10);   // 128 bits, carries none
			efloat<4> expected(tenth);
			expected.set_precision(32);
			const efloat<4> p1 = tenth * one, p2 = one * tenth, p3 = tenth / one;
			efloat<4> zero;
			zero.set_precision(32);
			const efloat<4> p4 = zero + tenth, p5 = tenth + zero;
			for (const auto& r : { p1, p2, p3, p4, p5 }) {
				if (r != expected || r.get_precision() != 32 || r.bits().size() != 1) {
					if (reportTestCases) std::cout << "    FAIL: a precision-free value did not round to its 32-bit partner: " << r.get_precision() << " bits, " << r.bits().size() << " limbs\n";
					++nrOfFailedTestCases;
				}
			}
		}
		{
			// a native value is exact even where the default is narrower than its 64 bits: it is
			// rounded by the operation that uses it, not at construction
			using E1 = efloat<1>;   // a 32-bit default
			const unsigned long long top = 18446744073709551615ull;
			const E1 d1 = E1(top) - E1(top - 1ull);
			const E1 d2 = top - E1(18446744073709551616.0);   // 2^64
			const E1 d3 = E1(0x123456789ll) - E1(0x123456788ll);
			if (d1 != E1(1) || d2 != E1(-1) || d3 != E1(1)) {
				if (reportTestCases) std::cout << "    FAIL: a 64-bit native value was rounded to the 32-bit default before the subtraction\n";
				++nrOfFailedTestCases;
			}
			set_default_precision(32);
			const efloat<4> d4 = efloat<4>(top) - efloat<4>(top - 1ull);
			set_default_precision(0);
			if (d4 != efloat<4>(1)) {
				if (reportTestCases) std::cout << "    FAIL: a 64-bit native value was rounded to set_default_precision(32) before the subtraction\n";
				++nrOfFailedTestCases;
			}
		}
		{
			// an integer operand converts exactly, not through double
			efloat<4> one(1.0);
			one.set_precision(128);
			const long long big = 4611686018427387905LL;   // 2^62 + 1: not a double
			const efloat<4> product = one * big;
			if (product != efloat<4>(big) || efloat<4>(double(big)) == efloat<4>(big)) {
				if (reportTestCases) std::cout << "    FAIL: x * (2^62 + 1) did not keep the integer exactly\n";
				++nrOfFailedTestCases;
			}
		}

		// ---------------------------------------------------------------------
		// 4. Re-rounding on Downscaling
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Re-rounding on Downscaling...\n";
		{
			// Construct a 64-bit precision value that has guard bit set for 32-bit truncation
			// a = { 0x80000000, 0x80000000 } (exact halfway tie at 32-bit)
			efloat_rounding_mode = RoundingMode::RoundToNearest;
			{
				efloat<4> a(false, 0, { 0x80000000, 0x80000000 });
				a.set_precision(64);
				a.set_precision(32); // round to nearest (ties to even) -> truncates to 1.0 (even)
				efloat<4> expected(false, 0, { 0x80000000 });
				expected.set_precision(32);
				if (a != expected) {
					if (reportTestCases) std::cout << "    FAIL: halfway-tie downscaling did not round to even. Result: " << a << "\n";
					++nrOfFailedTestCases;
				}
			}
			// Toward Positive: positive inexact halfway rounds up!
			efloat_rounding_mode = RoundingMode::RoundTowardPositive;
			{
				efloat<4> a(false, 0, { 0x80000000, 0x80000000 });
				a.set_precision(64);
				a.set_precision(32); // round toward positive -> rounds up
				efloat<4> expected(false, 0, { 0x80000001 });
				expected.set_precision(32);
				if (a != expected) {
					if (reportTestCases) std::cout << "    FAIL: halfway-tie downscaling did not round toward positive. Result: " << a << "\n";
					++nrOfFailedTestCases;
				}
			}
		}

		// Restore default rounding mode
		efloat_rounding_mode = RoundingMode::RoundToNearest;
		return nrOfFailedTestCases;
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

	std::string test_suite          = "efloat dynamic runtime precision";
	std::string test_tag            = "precision";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyEfloatPrecisionManagement(reportTestCases), "efloat", "precision manual");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyEfloatPrecisionManagement(reportTestCases), "efloat", "precision foundational");
#	endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif
}
catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
