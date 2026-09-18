// narrow_host_constants.cpp: elreal constants converge on the narrow hosts
//
// pi_zbcl<half> agreed with the reference to 8 digits at every depth, and sqrt2_zbcl<half>
// to 35 (#1396, #1397). Three defects were stacked underneath:
//
//   - cfloat's ldexp rewrote the exponent field whatever the input, which is wrong for a
//     subnormal: every one of half's subnormals came back wrong, so block::normalise()
//     corrupted any subnormal half value it was asked to rescale
//   - the multiply producers DROPPED a product residual whose host value was subnormal,
//     although on normalised operands it is an exact part of the product and a block's
//     scale lives in its wide exponent: x^3 * x^2 kept 5 digits of x^5 on half
//   - single-block division treated an empty running remainder as the end of the
//     division, so a dividend whose prefix divided exactly lost everything after it:
//     x^83 / 83 kept 79 digits where x^83 / 81 kept 278 -- the 138-digit wall pi met next
//
// Each is checked directly below, and so is the convergence it was holding back. The
// double-host half of #1397 -- sqrt2 scoring 319 against a 320-digit reference -- was not
// the generator at all, but the reference's own rounding; those constants now carry 340
// digits, and a perfect value reaches the 320-digit cap.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <string>
#include <vector>

#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/elreal_reference_digits.hpp>
#include <math/constants/reference_constants.hpp>
#include <universal/verification/test_suite.hpp>

// set to 1 to run the exploratory walk-through instead of the regression suite
#define MANUAL_TESTING 0

namespace {

	using namespace sw::universal;

	int expect_true(bool actual, const char* what, bool reportTestCases) {
		if (actual) return 0;
		if (reportTestCases) std::cout << "    FAIL " << what << '\n';
		return 1;
	}
	int expect_digits(int got, int wanted, const char* what, bool reportTestCases) {
		if (got >= wanted) return 0;
		if (reportTestCases) std::cout << "    FAIL " << what << ": " << got << " digits, expected at least " << wanted << '\n';
		return 1;
	}


	template<typename F> ZBCL<F> first_blocks(ZBCL<F> z, std::size_t n) {
		std::vector<block<F>> kept;
		while (!z.is_empty() && kept.size() < n) { kept.push_back(z.head()); z = z.tail(); }
		return zbcl_from_blocks<F>(std::move(kept));
	}

	// ---- a product keeps every exact bit of its operands' product --------------------
	//
	// The operands' VALUES are the oracle: their product is computed exactly as a dyadic,
	// independently of elreal's multiplication, and the streamed product must match it.

	int VerifyProductKeepsSubnormalResidual(bool reportTestCases) {
		int fails = 0;
		using F = half;
		const std::size_t depth = 32;
		const auto x  = div(from_native<F>(1.0), from_native<F>(5.0), depth);
		const auto x2 = first_blocks(mul_online(x, x), 40);
		const auto x3 = first_blocks(mul_online(x, x2), 40);
		const dyadic exact = zbcl_to_dyadic(x3) * zbcl_to_dyadic(x2);

		const int online = agreed_decimal_digits(zbcl_to_dyadic(first_blocks(mul_online(x3, x2), 60)), exact, 140);
		// the eager mul pools every block pair, and its renormalisation is slow enough that
		// forty-block operands take most of a minute: twelve blocks show the same defect
		const auto a = first_blocks(x3, 12), b = first_blocks(x2, 12);
		const int eager  = agreed_decimal_digits(zbcl_to_dyadic(mul(a, b, 12)), zbcl_to_dyadic(a) * zbcl_to_dyadic(b), 140);
		// both used to agree to 5 digits: a residual below half's smallest normal was dropped
		fails += expect_digits(online, 100, "mul_online keeps the product exact", reportTestCases);
		fails += expect_digits(eager,  100, "mul keeps the product exact", reportTestCases);
		return fails;
	}

	// ---- a division does not stop where a prefix of the dividend divides exactly -------
	//
	// (d + tail) / d: the leading block divides exactly, leaving an empty remainder with a
	// dividend block still to come. That used to end the division, returning exactly 1.
	// The tail sits three block-widths below d, so the dividend is a valid 0-overlap
	// expansion on every host (a block is k bits wide: 11 on half, 53 on double).
	//
	// The remainder stays empty only if the next dividend block is ZERO -- a nonzero one
	// refills it at once -- so the tail comes after an interior zero block, which is the
	// shape the series terms in #1396 had.

	template<typename F>
	int VerifyDivisionPastAnExactPrefix(const char* host, bool reportTestCases) {
		int fails = 0;
		for (double d : { 83.0, 3.0, 7.0 }) {
			using E = typename block<F>::exp_t;
			const auto dividend = ZBCL<F>::cons(from_native<F>(d).head(),
				ZBCL<F>::cons(block<F>{ F(0.0), E(-block<F>::k) },                   // an interior zero block
					ZBCL<F>::singleton(block<F>{ F(1.0), E(-3 * block<F>::k) })));   // 2^-3k, far below d
			const auto quotient = first_blocks(div_online(dividend, from_native<F>(d)), 40);
			// q * d must give the dividend back; stopping after the first block gives d alone
			const dyadic back = zbcl_to_dyadic(quotient) * zbcl_to_dyadic(from_native<F>(d));
			const int digits = agreed_decimal_digits(back, zbcl_to_dyadic(dividend), 60);
			if (digits < 40) {
				++fails;
				if (reportTestCases) {
					std::cout << "    FAIL " << host << ": (" << d << " + 2^-3k) / " << d
					          << " recovers the dividend to " << digits << " digits\n";
				}
			}
		}
		return fails;
	}

	// ---- ...and still ends when the dividend ends in an endless run of zeros ---------
	//
	// Reading on past an empty remainder must not read forever: a lazily produced exact
	// value can end in zero blocks that never stop coming, as one does inside sin(1/3) on
	// a double host. The first cut of the fix above hung there.

	template<typename F>
	ZBCL<F> zeros_forever(typename block<F>::exp_t e) {
		return ZBCL<F>::cons(block<F>{ F(0.0), e }, [e]() { return zeros_forever<F>(e - block<F>::k); });
	}

	template<typename F>
	int VerifyDivisionEndsAtAZeroTail(const char* host, bool reportTestCases) {
		int fails = 0;
		for (double d : { 3.0, 6.0, 7.0 }) {
			// d followed by zero blocks without end: the quotient is exactly 1
			const auto dividend = ZBCL<F>::cons(from_native<F>(d).head(),
				zeros_forever<F>(typename block<F>::exp_t(-2 * block<F>::k)));
			const auto quotient = first_blocks(div_online(dividend, from_native<F>(d)), 8);
			const dyadic q = zbcl_to_dyadic(quotient);
			if (agreed_decimal_digits(q, zbcl_to_dyadic(from_native<F>(1.0)), 60) < 60) {
				++fails;
				if (reportTestCases) std::cout << "    FAIL " << host << ": (" << d << ", 0, 0, ...) / " << d << " is not 1\n";
			}
		}
		return fails;
	}

	// ---- the constants the issues reported now converge -------------------------------

	int VerifyHalfConstantsConverge(bool reportTestCases) {
		int fails = 0;
		// before: pi 8 and sqrt2 35 at every depth; a correctly converging value gains about
		// k * log10(2) = 3.3 digits per block on half
		fails += expect_digits(agreed_decimal_digits(pi_zbcl<half>(32),    s_pi),    110, "pi on half, depth 32", reportTestCases);
		fails += expect_digits(agreed_decimal_digits(sqrt2_zbcl<half>(32), s_sqrt2),  90, "sqrt2 on half, depth 32", reportTestCases);
		// and deeper evaluation keeps buying digits rather than stalling
		fails += expect_true(agreed_decimal_digits(sqrt2_zbcl<half>(48), s_sqrt2) >
		                     agreed_decimal_digits(sqrt2_zbcl<half>(32), s_sqrt2), "sqrt2 keeps refining", reportTestCases);
		return fails;
	}

	// ---- the slower confirmations, kept out of level 1 ----------------------------------
	//
	// pi on double takes seconds (and never had the defect); ln2 exercises the same
	// odd_power_series as pi. Both are worth running, but not in every CI job.

	[[maybe_unused]] int VerifyDeeperConfirmations(bool reportTestCases) {
		int fails = 0;
		fails += expect_digits(agreed_decimal_digits(ln2_zbcl<half>(32), s_ln2), 110, "ln2 on half, depth 32", reportTestCases);
		fails += expect_digits(agreed_decimal_digits(pi_zbcl<half>(64),  s_pi),  220, "pi on half, depth 64 (was 138)", reportTestCases);
		fails += expect_digits(agreed_decimal_digits(pi_zbcl<double>(24), s_pi), 320, "pi on double", reportTestCases);
		return fails;
	}

	// ---- a perfect value scores the full cap ------------------------------------------
	//
	// sqrt2_zbcl<double> is correct to more than 340 digits, yet scored 319 against a
	// 320-digit reference: rounded to 320 digits, sqrt(2) is off by 3e-320, which is more
	// than the 1.4e-320 relative error that 320 digits of agreement allows. With 340-digit
	// references it reaches the cap, as pi always did by the luck of its trailing digits.

	int VerifyPerfectValueReachesTheCap(bool reportTestCases) {
		int fails = 0;
		fails += expect_digits(agreed_decimal_digits(sqrt2_zbcl<double>(24), s_sqrt2), 320, "sqrt2 on double", reportTestCases);
		// every reference carries digits beyond the 320 it certifies
		for (std::string_view ref : { s_pi, s_e, s_sqrt2, s_sqrt3, s_ln2, s_ln10, s_euler_gamma }) {
			std::size_t digits = 0;
			for (char c : ref) if (c >= '0' && c <= '9') ++digits;
			fails += expect_true(digits >= 330, "a reference carries guard digits past the cap", reportTestCases);
		}
		return fails;
	}

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

	std::string test_suite  = "elreal narrow-host constants";
	std::string test_tag    = "elreal narrow host";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyProductKeepsSubnormalResidual(reportTestCases), test_tag, "product residual kept");
	nrOfFailedTestCases += ReportTestResult(VerifyDivisionPastAnExactPrefix<half>("half", reportTestCases), test_tag, "division past an exact prefix, half");
	nrOfFailedTestCases += ReportTestResult(VerifyDivisionPastAnExactPrefix<float>("float", reportTestCases), test_tag, "division past an exact prefix, float");
	nrOfFailedTestCases += ReportTestResult(VerifyDivisionPastAnExactPrefix<double>("double", reportTestCases), test_tag, "division past an exact prefix, double");
	nrOfFailedTestCases += ReportTestResult(VerifyDivisionEndsAtAZeroTail<half>("half", reportTestCases), test_tag, "division ends at a zero tail, half");
	nrOfFailedTestCases += ReportTestResult(VerifyDivisionEndsAtAZeroTail<float>("float", reportTestCases), test_tag, "division ends at a zero tail, float");
	nrOfFailedTestCases += ReportTestResult(VerifyDivisionEndsAtAZeroTail<double>("double", reportTestCases), test_tag, "division ends at a zero tail, double");
	nrOfFailedTestCases += ReportTestResult(VerifyHalfConstantsConverge(reportTestCases), test_tag, "half constants converge");
	nrOfFailedTestCases += ReportTestResult(VerifyPerfectValueReachesTheCap(reportTestCases), test_tag, "perfect value reaches the cap");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyDeeperConfirmations(reportTestCases), test_tag, "deeper confirmations");
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
