// limb_budget.cpp: arithmetic results are bounded at ereal's limb budget
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// ereal never used to cap an arithmetic result. The limb count of a result was whatever
// the expansion algorithms produced, and the only thing that ever pruned it was underflow:
// a component below the limb type's smallest normal becomes zero and renormalization drops
// it. With double limbs that accident looks like a cap -- 1/3 settles at 16 limbs. A
// wide-exponent limb has no such floor, so every x87 quotient grew to ~250 limbs (250 * 64
// bits is x87's exponent range) whatever maxlimbs said, and a single division cost seconds
// (#1572).
//
// Arithmetic is now bounded at  limb_budget = min(2 * maxlimbs, max_safe_limbs),  and the
// bound is applied inside the Newton reciprocal as well as to the result -- capping only
// the result still paid for the 250-limb intermediate.
//
// What this suite pins:
//   - the budget formula, on every limb type the host has
//   - no operation, however iterated, returns more limbs than the budget
//   - the truncation contract: what a bounded result dropped lies below the last limb it
//     kept, checked against an exact dyadic oracle rather than through a double
//   - the budget is not a precision regression for double limbs: the guard limbs that the
//     existing suites were tuned against (1/3 at 16 limbs for an ereal<8>) are all still
//     there, which is why the budget is 2 * maxlimbs and not maxlimbs
//   - a result still carries at least the precision maxlimbs promises
#include <universal/utility/directives.hpp>
#include <cmath>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <vector>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/dyadic_exact.hpp>
#include <universal/verification/ereal_reference_digits.hpp>

namespace {

	// The exact value of an expansion is the sum of its limbs: they are non-overlapping,
	// so the sum is a dyadic rational with no rounding anywhere.
	template<unsigned maxlimbs, typename FpType>
	sw::universal::dyadic exact_value(const sw::universal::ereal<maxlimbs, FpType>& x) {
		using namespace sw::universal;
		dyadic v;
		for (FpType limb : x.limbs()) v = v + dyadic::from_fp(limb);
		return v;
	}

	int expect(bool condition, const std::string& what, bool reportTestCases) {
		if (!condition && reportTestCases) std::cout << "    FAIL " << what << '\n';
		return condition ? 0 : 1;
	}

	// the budget is what the header says it is, on this limb type
	template<unsigned maxlimbs, typename FpType>
	int VerifyBudgetFormula(const std::string& tag, bool reportTestCases) {
		using namespace sw::universal;
		using Real = ereal<maxlimbs, FpType>;
		constexpr unsigned twice  = 2 * maxlimbs;
		constexpr unsigned expected = (twice < Real::max_safe_limbs) ? twice : Real::max_safe_limbs;
		int fails = 0;
		fails += expect(Real::limb_budget == expected, tag + ": budget is min(2*maxlimbs, max_safe_limbs)", reportTestCases);
		// a budget below maxlimbs would mean the type cannot hold what it promises
		fails += expect(Real::limb_budget >= maxlimbs, tag + ": budget is at least maxlimbs", reportTestCases);
		// and never past the point where a limb would be subnormal
		fails += expect(Real::limb_budget <= Real::max_safe_limbs, tag + ": budget is within max_safe_limbs", reportTestCases);
		return fails;
	}

	// No sequence of operations returns more limbs than the budget, and every result that
	// was truncated dropped only what lies below the last limb it kept.
	template<unsigned maxlimbs, typename FpType>
	int VerifyBounded(const std::string& tag, unsigned nrIterations, bool reportTestCases) {
		using namespace sw::universal;
		using Real = ereal<maxlimbs, FpType>;
		std::mt19937_64 rng(0xB0'1D'BEEFULL + maxlimbs);
		std::uniform_real_distribution<double> unit(0.5, 2.0);
		int fails = 0;

		auto bounded = [&](const Real& r, const char* op, unsigned i) {
			if (r.limbs().size() > Real::limb_budget) {
				if (reportTestCases) std::cout << "    FAIL " << tag << ' ' << op << " (iter " << i
					<< ") returned " << r.limbs().size() << " limbs, budget " << Real::limb_budget << '\n';
				++fails;
			}
		};

		// the truncation contract: |exact - got| < |last limb kept|
		auto contract = [&](const Real& got, const dyadic& want, const char* op, unsigned i) {
			const dyadic g = exact_value(got);
			if (g == want) return;                              // nothing was dropped
			if (got.limbs().size() >= Real::limb_budget
			    && less_in_magnitude(want - g, dyadic::from_fp(got.limbs().back()))) return;
			if (reportTestCases) std::cout << "    FAIL " << tag << ' ' << op
				<< " truncation contract (iter " << i << ")\n";
			++fails;
		};

		// Space the components by more than the limb's significand so they stay
		// non-overlapping, and centre the band on 2^0. The spacing has to come from the
		// limb type: a fixed 55 (right for double) put the top component at 2^137 for
		// float limbs, past float's range, and the infinity that produced took the exact
		// oracle with it.
		constexpr int spacing = std::numeric_limits<FpType>::digits + 2;
		constexpr int top     = spacing * (static_cast<int>(maxlimbs) - 1) / 2;
		static_assert(top + spacing < std::numeric_limits<FpType>::max_exponent,
		              "operand band must fit the limb type's exponent range");

		for (unsigned i = 0; i < nrIterations; ++i) {
			// operands that already span several limbs
			Real a(0.0), b(0.0);
			int e = top;
			for (unsigned k = 0; k < maxlimbs; ++k) {
				a += Real(unit(rng) * std::ldexp(1.0, e));
				b += Real(unit(rng) * std::ldexp(1.0, e - 3));
				e -= spacing;
			}
			bounded(a, "operand build", i);
			bounded(b, "operand build", i);

			const dyadic da = exact_value(a), db = exact_value(b);
			const Real sum = a + b, diff = a - b, prod = a * b;
			bounded(sum,  "a+b", i);
			bounded(diff, "a-b", i);
			bounded(prod, "a*b", i);
			contract(sum,  da + db, "a+b", i);
			contract(diff, da - db, "a-b", i);
			contract(prod, da * db, "a*b", i);

			if (!b.iszero()) bounded(a / b, "a/b", i);
		}

		// the growth case from the issue: eight Newton steps for sqrt(2). Uncapped this
		// reached the limb type's whole exponent range -- 250 limbs on x87.
		Real x(1.5), two(2.0), half(0.5);
		for (int k = 0; k < 8; ++k) {
			x = (x + two / x) * half;
			bounded(x, "newton sqrt(2)", static_cast<unsigned>(k));
		}
		// and a long accumulation, which grows through addition rather than division
		Real acc(0.0);
		for (int k = 0; k < 200 && k < -std::numeric_limits<FpType>::min_exponent; ++k) {
			acc += Real(std::ldexp(unit(rng), -k));
			bounded(acc, "accumulation", static_cast<unsigned>(k));
		}
		return fails;
	}

	// The budget is 2 * maxlimbs rather than maxlimbs because the guard limbs are load
	// bearing: the double suites were tuned against results that carry them. These are the
	// measured digit counts, which the budget must not move.
	int VerifyDoublePrecisionUnchanged(bool reportTestCases) {
		using namespace sw::universal;
		// 1/3 to as many digits as the oracle will read. Generated rather than written out:
		// a reference one digit short silently certifies one digit less, which is exactly
		// the kind of off-by-one an accuracy claim should not rest on.
		const std::string third_ref = "0." + std::string(400, '3');
		int fails = 0;
		{
			// the guard limbs are load bearing: an ereal<8> quotient has carried 16 limbs
			// and 260 digits all along, and the budget is 2 * maxlimbs so that it still does
			using Real = ereal<8>;
			Real third = Real(1.0) / Real(3.0);
			fails += expect(third.limbs().size() == 16, "ereal<8> 1/3 keeps its 16 limbs", reportTestCases);
			fails += expect(agreed_decimal_digits(third, third_ref) >= 260,
			                "ereal<8> 1/3 keeps its 260 digits", reportTestCases);
		}
		{
			// and a result still carries at least the precision maxlimbs promises
			using Real = ereal<4>;
			Real third = Real(1.0) / Real(3.0);
			const int promised = static_cast<int>(4 * Real::digits10_per_limb);
			fails += expect(agreed_decimal_digits(third, third_ref) >= promised,
			                "ereal<4> 1/3 carries the promised precision", reportTestCases);
		}
		return fails;
	}

}  // anonymous namespace

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
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

	std::string test_suite  = "ereal arithmetic is bounded at the limb budget";
	std::string test_tag    = "limb budget";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyBounded<8, double>("ereal<8>", 10, true), test_tag, "double bounded");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyBudgetFormula<3, float>("ereal<3, float>", reportTestCases), test_tag, "float budget");
	nrOfFailedTestCases += ReportTestResult(VerifyBudgetFormula<5, float>("ereal<5, float>", reportTestCases), test_tag, "float budget, clamped");
	nrOfFailedTestCases += ReportTestResult(VerifyBudgetFormula<8, double>("ereal<8>", reportTestCases), test_tag, "double budget");
	nrOfFailedTestCases += ReportTestResult(VerifyBudgetFormula<16, double>("ereal<16>", reportTestCases), test_tag, "double budget, clamped");
	nrOfFailedTestCases += ReportTestResult(VerifyBounded<5, float>("ereal<5, float>", 50, reportTestCases), test_tag, "float bounded");
	nrOfFailedTestCases += ReportTestResult(VerifyBounded<8, double>("ereal<8>", 50, reportTestCases), test_tag, "double bounded");
	nrOfFailedTestCases += ReportTestResult(VerifyDoublePrecisionUnchanged(reportTestCases), test_tag, "double precision unchanged");
	if constexpr (is_expansion_limb_v<long double>) {
		nrOfFailedTestCases += ReportTestResult(VerifyBudgetFormula<8, long double>("ereal<8, long double>", reportTestCases), test_tag, "long double budget");
		// the case the issue is about: uncapped this ran for tens of seconds
		nrOfFailedTestCases += ReportTestResult(VerifyBounded<8, long double>("ereal<8, long double>", 20, reportTestCases), test_tag, "long double bounded");
	}
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyBounded<16, double>("ereal<16>", 100, reportTestCases), test_tag, "double bounded, 16 limbs");
	if constexpr (is_expansion_limb_v<long double> && ereal<8, long double>::max_safe_limbs >= 24) {
		nrOfFailedTestCases += ReportTestResult(VerifyBudgetFormula<24, long double>("ereal<24, long double>", reportTestCases), test_tag, "long double budget, 24 limbs");
	}
#endif

#if REGRESSION_LEVEL_3
	if constexpr (is_expansion_limb_v<long double> && ereal<8, long double>::max_safe_limbs >= 24) {
		nrOfFailedTestCases += ReportTestResult(VerifyBounded<24, long double>("ereal<24, long double>", 10, reportTestCases), test_tag, "long double bounded, 24 limbs");
	}
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
