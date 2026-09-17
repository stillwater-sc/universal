// test_tracked_lns.cpp: verification of TrackedLNS<LNSType> error tracking
//
// TrackedLNS<LNSType> (include/sw/universal/utility/tracked_lns.hpp) exploits the one
// structural advantage of the logarithmic number system: a product is a SUM of
// logarithms, so multiplication and division introduce no rounding error of their own.
// Only addition and subtraction do, and the tracker keeps a separate accumulator,
// addition_error(), for exactly that, alongside per-class operation counters.
//
// The contracts pinned here:
//   - addition_error() stays exactly zero through any chain of * and /, and grows only
//     through + and - (and sqrt); that is the claim the whole design rests on
//   - the counters partition the work: operations() == additions() + multiplications()
//     + divisions(), exact_operations() == multiplications() + divisions(), and each
//     operator increments exactly its own class while summing the operands'
//   - a value that LNS represents exactly (a power of two) carries no error at all,
//     through construction and through any product of such values
//   - the two constructors differ on purpose: from a double the shadow keeps the
//     requested value, from an LNS it keeps the already-rounded one
//   - cancellation and absorption are detected by the documented shadow predicates
//
// Two checks below pin behaviour that is wrong (#1546); each says so, so that a fix is
// noticed here rather than silently changing what callers see.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>

#include <universal/number/lns/lns.hpp>
#include <universal/utility/tracked_lns.hpp>
#include <universal/verification/test_suite.hpp>

// set to 1 to run the exploratory walk-through instead of the regression suite
#define MANUAL_TESTING 0

namespace {

	using namespace sw::universal;

	using LNS32 = lns<32, 8>;
	using LNS16 = lns<16, 5>;
	using Tracked32 = TrackedLNS<LNS32>;

	// ---- reporting helpers -----------------------------------------------------

	int expect_exact(double actual, double wanted, const char* what, bool reportTestCases) {
		if (actual == wanted) return 0;
		if (reportTestCases) {
			std::cout << "    FAIL " << what << ": got " << std::setprecision(17) << std::scientific
			          << actual << ", expected " << wanted << std::defaultfloat << '\n';
		}
		return 1;
	}

	int expect_count(std::uint64_t actual, std::uint64_t wanted, const char* what, bool reportTestCases) {
		if (actual == wanted) return 0;
		if (reportTestCases) {
			std::cout << "    FAIL " << what << ": got " << actual << ", expected " << wanted << '\n';
		}
		return 1;
	}

	int expect_true(bool actual, const char* what, bool reportTestCases) {
		if (actual) return 0;
		if (reportTestCases) std::cout << "    FAIL " << what << '\n';
		return 1;
	}

	// ---- multiplication and division introduce no error -------------------------
	//
	// This is the claim the class exists to make. addition_error() is propagated but
	// never increased by * or /, so any chain of products and quotients must leave it
	// at exactly zero, and must leave additions() at zero too.

	int VerifyMultiplicationIsErrorFree(bool reportTestCases) {
		int fails = 0;

		Tracked32 a = 3.0, b = 7.0, c = 11.0;   // none of these is representable in LNS
		fails += expect_true(a.error() > 0.0, "3.0 is not representable in lns<32,8>", reportTestCases);

		// a long chain of products and quotients
		auto chain = a * b / c * a * b / c;
		fails += expect_exact(chain.addition_error(), 0.0,
			"products and quotients add no addition error", reportTestCases);
		fails += expect_count(chain.additions(), 0, "no additions in the chain", reportTestCases);
		fails += expect_count(chain.multiplications(), 3, "three multiplications", reportTestCases);
		fails += expect_count(chain.divisions(), 2, "two divisions", reportTestCases);
		fails += expect_count(chain.operations(), 5, "five operations", reportTestCases);
		fails += expect_count(chain.exact_operations(), 5, "all five are exact operations", reportTestCases);

		// one addition changes that, and only that
		auto withSum = chain + a;
		fails += expect_true(withSum.addition_error() > 0.0, "an addition introduces error",
			reportTestCases);
		fails += expect_count(withSum.additions(), 1, "the addition is counted", reportTestCases);
		fails += expect_count(withSum.exact_operations(), chain.exact_operations() + a.exact_operations(),
			"the addition is not an exact operation", reportTestCases);

		// and multiplying afterwards propagates that error without adding to it
		auto after = withSum * b;
		fails += expect_exact(after.addition_error(), withSum.addition_error(),
			"multiplication propagates the addition error unchanged", reportTestCases);
		fails += expect_count(after.multiplications(), withSum.multiplications() + 1,
			"the multiplication is counted", reportTestCases);

		return fails;
	}

	// ---- powers of two are exact all the way through -----------------------------

	int VerifyExactPowersOfTwo(bool reportTestCases) {
		int fails = 0;

		// LNS stores the base-2 logarithm, so a power of two lands on an exact encoding
		for (double v : { 0.25, 0.5, 1.0, 2.0, 4.0, 16.0 }) {
			Tracked32 a = v;
			fails += expect_exact(double(a.value()), v, "a power of two is represented exactly",
				reportTestCases);
			fails += expect_exact(a.error(), 0.0, "a power of two carries no error", reportTestCases);
			fails += expect_exact(a.relative_error(), 0.0, "no relative error", reportTestCases);
			fails += expect_true(a.is_exact(), "a power of two is exact", reportTestCases);
			fails += expect_count(a.operations(), 0, "construction is not an operation", reportTestCases);
		}

		// and products of powers of two stay exact
		{
			Tracked32 two = 2.0, four = 4.0;
			auto eight = two * four;
			fails += expect_exact(double(eight.value()), 8.0, "2 * 4", reportTestCases);
			fails += expect_exact(eight.shadow(), 8.0, "the shadow agrees", reportTestCases);
			fails += expect_exact(eight.error(), 0.0, "2 * 4 is exact", reportTestCases);
			fails += expect_true(eight.is_exact(), "2 * 4 reports exact", reportTestCases);
			fails += expect_count(eight.multiplications(), 1, "one multiplication", reportTestCases);

			auto back = eight / four;
			fails += expect_exact(double(back.value()), 2.0, "8 / 4", reportTestCases);
			fails += expect_exact(back.error(), 0.0, "8 / 4 is exact", reportTestCases);
			fails += expect_count(back.divisions(), 1, "one division", reportTestCases);
			fails += expect_exact(back.addition_error(), 0.0, "division adds no error", reportTestCases);
		}

		// an addition of powers of two whose sum is also a power of two stays exact
		{
			Tracked32 a = 2.0, b = 2.0;
			auto sum = a + b;
			fails += expect_exact(double(sum.value()), 4.0, "2 + 2", reportTestCases);
			fails += expect_exact(sum.addition_error(), 0.0, "2 + 2 rounds nothing", reportTestCases);
			fails += expect_count(sum.additions(), 1, "the addition is still counted", reportTestCases);
		}

		// a value LNS cannot represent reports its conversion error before any arithmetic
		{
			Tracked32 three = 3.0;
			fails += expect_exact(three.shadow(), 3.0, "the shadow keeps the requested value",
				reportTestCases);
			fails += expect_exact(three.error(), std::abs(3.0 - double(LNS32(3.0))),
				"the error is the conversion gap", reportTestCases);
			fails += expect_true(!three.is_exact(), "3.0 is not exact", reportTestCases);
			fails += expect_count(three.additions(), 0, "and no addition was involved", reportTestCases);
		}

		return fails;
	}

	// ---- the two constructors differ on purpose ----------------------------------

	int VerifyConstructorForms(bool reportTestCases) {
		int fails = 0;

		// from a double, the shadow keeps what the caller asked for, so the conversion
		// rounding shows up as error
		Tracked32 fromDouble = 3.0;
		// from an LNS value, the shadow is taken from the already-rounded encoding, so
		// there is nothing left to report
		LNS32 rounded = 3.0;
		Tracked32 fromLns = rounded;

		fails += expect_exact(fromDouble.shadow(), 3.0, "from a double: the shadow is 3.0", reportTestCases);
		fails += expect_exact(fromLns.shadow(), double(rounded), "from an LNS: the shadow is the encoding",
			reportTestCases);
		fails += expect_true(fromDouble.error() > 0.0, "from a double: the conversion error is visible",
			reportTestCases);
		fails += expect_exact(fromLns.error(), 0.0, "from an LNS: nothing left to report", reportTestCases);
		fails += expect_true(!fromDouble.is_exact() && fromLns.is_exact(),
			"the two forms disagree on exactness", reportTestCases);
		// both hold the same encoding
		fails += expect_true(fromDouble.value() == fromLns.value(), "both hold the same encoding",
			reportTestCases);

		return fails;
	}

	// ---- counter algebra ---------------------------------------------------------

	int VerifyCounterAlgebra(bool reportTestCases) {
		int fails = 0;

		Tracked32 a = 2.0, b = 3.0, c = 5.0;

		struct Case { Tracked32 r; std::uint64_t adds, mults, divs; const char* tag; };
		const Case cases[] = {
			{ a + b,             1, 0, 0, "one addition" },
			{ a - b,             1, 0, 0, "subtraction counts as an addition" },
			{ a * b,             0, 1, 0, "one multiplication" },
			{ a / b,             0, 0, 1, "one division" },
			{ (a + b) * c,       1, 1, 0, "an addition and a product" },
			{ (a + b) * (a - c), 2, 1, 0, "two additions and a product" },
			{ (a * b) / (a * c), 0, 2, 1, "two products and a quotient" },
		};
		for (const Case& t : cases) {
			fails += expect_count(t.r.additions(), t.adds, t.tag, reportTestCases);
			fails += expect_count(t.r.multiplications(), t.mults, t.tag, reportTestCases);
			fails += expect_count(t.r.divisions(), t.divs, t.tag, reportTestCases);
			// the partition holds for every result
			fails += expect_count(t.r.operations(), t.adds + t.mults + t.divs,
				"operations is the sum of the three counters", reportTestCases);
			fails += expect_count(t.r.exact_operations(), t.mults + t.divs,
				"exact_operations excludes the additions", reportTestCases);
			fails += expect_true(t.r.exact_operations() <= t.r.operations(),
				"exact operations are a subset", reportTestCases);
		}

		// unary minus and abs charge nothing and keep every counter
		{
			auto lossy = (a + b) * c;
			auto neg = -lossy;
			fails += expect_count(neg.additions(), lossy.additions(), "unary minus keeps additions",
				reportTestCases);
			fails += expect_count(neg.multiplications(), lossy.multiplications(),
				"unary minus keeps multiplications", reportTestCases);
			fails += expect_exact(neg.addition_error(), lossy.addition_error(),
				"unary minus keeps the addition error", reportTestCases);
			auto m = abs(neg);
			fails += expect_count(m.operations(), lossy.operations(), "abs charges nothing", reportTestCases);
			fails += expect_exact(m.addition_error(), lossy.addition_error(), "abs keeps the error",
				reportTestCases);
		}

		// sqrt is charged as an error-introducing operation, i.e. as an addition
		{
			auto r = sqrt(a);
			fails += expect_count(r.additions(), 1, "sqrt counts as an addition", reportTestCases);
			fails += expect_count(r.multiplications(), 0, "sqrt is not a multiplication", reportTestCases);
		}

		// pow multiplies in a linear loop: x^n costs n-1 multiplications
		{
			fails += expect_count(pow(a, 2).multiplications(), 1, "pow(x,2)", reportTestCases);
			fails += expect_count(pow(a, 4).multiplications(), 3, "pow(x,4)", reportTestCases);
			fails += expect_count(pow(a, 4).additions(), 0, "pow adds nothing", reportTestCases);
			fails += expect_exact(pow(a, 4).addition_error(), 0.0, "pow keeps the addition error at zero",
				reportTestCases);
			fails += expect_exact(double(pow(a, 4).value()), 16.0, "2^4 is exact", reportTestCases);
			fails += expect_count(pow(a, 0).operations(), 0, "pow(x,0) is a constant", reportTestCases);
			fails += expect_exact(double(pow(a, 0).value()), 1.0, "pow(x,0) is one", reportTestCases);
			fails += expect_count(pow(a, -2).divisions(), 1, "a negative power divides once", reportTestCases);
		}

		// sqr is a multiplication with itself, so the operand's counters are doubled
		{
			auto lossy = a + b;                // one addition
			auto s = sqr(lossy);
			fails += expect_count(s.multiplications(), 1, "sqr is one multiplication", reportTestCases);
			fails += expect_count(s.additions(), 2, "sqr doubles the operand's additions", reportTestCases);
		}

		return fails;
	}

	// ---- the addition error only grows -------------------------------------------

	int VerifyAdditionErrorMonotonicity(bool reportTestCases) {
		int fails = 0;

		// 1 + 5 does not land on a representable logarithm, so it rounds and the
		// tracker has something to report. (1 + 3 would not: it lands exactly on 4.)
		Tracked32 x = 1.0, step = 5.0;
		auto acc = x + step;
		double previous = acc.addition_error();
		fails += expect_true(previous > 0.0, "the first addition introduces error", reportTestCases);

		for (int i = 0; i < 10; ++i) {
			acc = acc + step;
			if (!(acc.addition_error() >= previous)) {
				++fails;
				if (reportTestCases) std::cout << "    FAIL the addition error decreased at step " << i << '\n';
			}
			previous = acc.addition_error();
			// multiplying in between must not change it
			auto scaled = acc * Tracked32(2.0);
			if (scaled.addition_error() != acc.addition_error()) {
				++fails;
				if (reportTestCases) std::cout << "    FAIL multiplication changed the addition error\n";
			}
		}
		fails += expect_count(acc.additions(), 11, "eleven additions", reportTestCases);

		return fails;
	}

	// ---- cancellation and absorption ----------------------------------------------
	//
	// Both are shadow-space predicates with documented thresholds: a cancellation when
	// the operands agree to within 10 percent in the direction that cancels, an
	// absorption when their magnitudes differ by more than half the shadow type's
	// significand (26.5 bits for double).

	int VerifyCancellationAndAbsorption(bool reportTestCases) {
		int fails = 0;

		// subtraction of nearly equal values
		{
			Tracked32 a = 1.0, b = 1.05;
			auto d = a - b;
			fails += expect_count(d.cancellations(), 1, "1 - 1.05 cancels", reportTestCases);
			fails += expect_true(d.had_cancellation(), "had_cancellation agrees", reportTestCases);
		}
		// well separated values do not
		{
			Tracked32 a = 1.0, b = 2.0;
			auto d = a - b;
			fails += expect_count(d.cancellations(), 0, "1 - 2 does not cancel", reportTestCases);
			fails += expect_true(!d.had_cancellation(), "had_cancellation agrees", reportTestCases);
		}
		// addition of nearly opposite values cancels too
		{
			Tracked32 a = 1.0, b = -1.05;
			auto s = a + b;
			fails += expect_count(s.cancellations(), 1, "1 + (-1.05) cancels", reportTestCases);
		}
		// cancellation counts are inherited
		{
			Tracked32 a = 1.0, b = 1.05, c = 3.0;
			auto chain = (a - b) * c;
			fails += expect_count(chain.cancellations(), 1, "the count survives a multiplication",
				reportTestCases);
		}

		// absorption: the magnitudes must differ by more than 26.5 bits
		{
			Tracked32 a = 1.0;
			fails += expect_count((a + Tracked32(1e-12)).absorptions(), 1, "1 + 1e-12 absorbs",
				reportTestCases);
			fails += expect_count((a + Tracked32(0.001)).absorptions(), 0, "1 + 0.001 does not absorb",
				reportTestCases);
			fails += expect_true((a + Tracked32(1e-12)).had_absorption(), "had_absorption agrees",
				reportTestCases);
			// and the count is inherited through a product
			auto scaled = (a + Tracked32(1e-12)) * Tracked32(2.0);
			fails += expect_count(scaled.absorptions(), 1, "the count survives a multiplication",
				reportTestCases);
		}

		return fails;
	}

	// ---- assignment resets the tracking state --------------------------------------

	int VerifyResetOnAssignment(bool reportTestCases) {
		int fails = 0;

		Tracked32 a = 1.0;
		a = a + Tracked32(1.05);
		a = a - Tracked32(2.0);
		a = a * Tracked32(4.0);
		fails += expect_true(a.operations() > 0, "operations accumulated before the reset", reportTestCases);
		fails += expect_true(a.addition_error() > 0.0, "error accumulated before the reset", reportTestCases);

		a = 2.0;
		fails += expect_exact(double(a.value()), 2.0, "value after assignment", reportTestCases);
		fails += expect_exact(a.shadow(), 2.0, "shadow after assignment", reportTestCases);
		fails += expect_exact(a.addition_error(), 0.0, "addition error reset", reportTestCases);
		fails += expect_count(a.additions(), 0, "additions reset", reportTestCases);
		fails += expect_count(a.multiplications(), 0, "multiplications reset", reportTestCases);
		fails += expect_count(a.divisions(), 0, "divisions reset", reportTestCases);
		fails += expect_count(a.cancellations(), 0, "cancellations reset", reportTestCases);
		fails += expect_count(a.absorptions(), 0, "absorptions reset", reportTestCases);
		fails += expect_true(a.is_exact(), "is_exact after assignment", reportTestCases);

		return fails;
	}

	// ---- a narrower LNS loses more --------------------------------------------------

	int VerifyNarrowerIsCoarser(bool reportTestCases) {
		int fails = 0;

		// lns<16,5> keeps five fractional bits of the logarithm against lns<32,8>'s
		// eight, so the same unrepresentable value lands further from the truth
		TrackedLNS<LNS16> narrow = 3.0;
		Tracked32 wide = 3.0;
		fails += expect_true(narrow.error() > wide.error(), "the narrower LNS is further off",
			reportTestCases);
		fails += expect_true(narrow.relative_error() > wide.relative_error(),
			"and relatively so", reportTestCases);

		// both are exact on a power of two
		fails += expect_exact(TrackedLNS<LNS16>(4.0).error(), 0.0, "lns<16,5> holds 4.0 exactly",
			reportTestCases);

		return fails;
	}

	// ---- behaviour that is pinned although it is wrong -------------------------------

	int VerifyKnownDefects(bool reportTestCases) {
		int fails = 0;

		// 1. A completely cancelled result claims FULL precision. relative_error()
		// returns 0 when the shadow is zero -- reading "no relative error" off a value
		// that has lost every significant bit -- and valid_bits() maps that to the cap.
		// The cancellation counter does record the event, so the information is there;
		// it just does not reach the two metrics a caller is most likely to read.
		{
			Tracked32 a = 2.0;
			auto d = a - a;
			fails += expect_exact(d.shadow(), 0.0, "the shadow cancels to zero", reportTestCases);
			fails += expect_count(d.additions(), 1, "the subtraction is counted", reportTestCases);
			fails += expect_count(d.cancellations(), 1, "and recorded as a cancellation", reportTestCases);
			fails += expect_exact(d.relative_error(), 0.0,
				"a fully cancelled result reports no relative error (known defect #1546)", reportTestCases);
			fails += expect_exact(d.valid_bits(), 32.0,
				"and reports the full 32 valid bits (known defect #1546)", reportTestCases);
		}

		// 2. valid_bits() is capped by nbits, which is not a precision. lns<32,8> keeps
		// eight fractional bits of the logarithm -- roughly nine bits of relative
		// precision -- yet an exact value reports 32. The header admits the proxy in a
		// comment; the number is still four times the truth.
		{
			Tracked32 exact = 2.0;
			fails += expect_exact(exact.valid_bits(), 32.0,
				"an exact value reports nbits valid bits (known defect #1546: nbits is not a precision)",
				reportTestCases);
			fails += expect_exact(TrackedLNS<LNS16>(2.0).valid_bits(), 16.0,
				"and the narrow type reports its own nbits (known defect #1546)", reportTestCases);
		}

		return fails;
	}

	// ---- the type tag -----------------------------------------------------------------

	int VerifyTypeTag(bool reportTestCases) {
		int fails = 0;

		// the mangled type name is compiler dependent, so only the prefix is pinned
		const std::string tag = type_tag(Tracked32());
		fails += expect_true(tag.rfind("TrackedLNS<", 0) == 0, "type_tag names the class", reportTestCases);
		fails += expect_true(tag.back() == '>', "type_tag is closed", reportTestCases);

		return fails;
	}

	// ---- the exploratory narrative, kept for manual inspection -------------------------

#if MANUAL_TESTING
	void ReportTrackedLnsBehaviour() {
		std::cout << "\n=== TrackedLNS<lns<32,8>> walk-through ===\n";

		std::cout << "\nmultiplication is exact in the log domain:\n";
		Tracked32 a = 3.0, b = 7.0;
		auto p = a * b;
		p.report(std::cout);

		std::cout << "\naddition is where the error comes from:\n";
		auto s = a + b;
		s.report(std::cout);

		std::cout << "\ncancellation, 1.0 - 1.05:\n";
		auto d = Tracked32(1.0) - Tracked32(1.05);
		d.report(std::cout);
	}
#endif  // MANUAL_TESTING

}  // anonymous namespace

#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "TrackedLNS<LNSType> logarithmic error tracking";
	std::string test_tag    = "tracked_lns";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	ReportTrackedLnsBehaviour();
	nrOfFailedTestCases += VerifyMultiplicationIsErrorFree(true);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore failures
#else  // !MANUAL_TESTING

	// A few hundred LNS operations, so it all belongs at level 1: CI configures
	// REGRESSION_LEVEL_1 only, and a contract that is not checked there does not gate.
#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplicationIsErrorFree(reportTestCases), test_tag, "multiplication is error free");
	nrOfFailedTestCases += ReportTestResult(VerifyExactPowersOfTwo(reportTestCases), test_tag, "exact powers of two");
	nrOfFailedTestCases += ReportTestResult(VerifyConstructorForms(reportTestCases), test_tag, "constructor forms");
	nrOfFailedTestCases += ReportTestResult(VerifyCounterAlgebra(reportTestCases), test_tag, "counter algebra");
	nrOfFailedTestCases += ReportTestResult(VerifyAdditionErrorMonotonicity(reportTestCases), test_tag, "addition error monotonicity");
	nrOfFailedTestCases += ReportTestResult(VerifyCancellationAndAbsorption(reportTestCases), test_tag, "cancellation and absorption");
	nrOfFailedTestCases += ReportTestResult(VerifyResetOnAssignment(reportTestCases), test_tag, "reset on assignment");
	nrOfFailedTestCases += ReportTestResult(VerifyNarrowerIsCoarser(reportTestCases), test_tag, "narrower is coarser");
	nrOfFailedTestCases += ReportTestResult(VerifyTypeTag(reportTestCases), test_tag, "type tag");
	nrOfFailedTestCases += ReportTestResult(VerifyKnownDefects(reportTestCases), test_tag, "pinned known defects");
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
