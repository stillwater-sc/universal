// sqrt.cpp: test suite runner for sqrt/cbrt functions for ereal adaptive-precision
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <algorithm>
#include <cmath>
#include <limits>
#include <random>
#include <string>
#include <string_view>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/test_suite_mathlib_adaptive.hpp>
#include <universal/verification/ereal_precision.hpp>
#include <math/constants/reference_constants.hpp>

namespace {
	using namespace sw::universal;

		// The checks below are exact (ereal_precision.hpp: both sides become dyadic rationals),
		// never through double: a double comparison passes a result correct to only 53 bits,
		// and so cannot see a loss of ereal's precision (#2008).
		//
		// Each check asks for ereal_bits_of<Real>::value less kMarginBits.  A result may carry
		// up to twice the limbs asked for (the limb budget), so the measured agreement is well
		// above that floor: ereal<4> shows 419-441 bits against 212, ereal<8> 840-880 against
		// 424.  At 16 and 19 limbs every value saturates near 1024-1044 bits, where the last
		// double limb reaches 2^-1074; the worst case there is cbrt(2)^3 at ereal<19>, 1023 bits
		// against 1007.  16 bits of margin leaves that case 32 bits of slack.
		constexpr int kMarginBits = 16;

		template<typename Real>
		constexpr int wanted_bits() { return ereal_bits_of<Real>::value - kMarginBits; }

		template<typename Real>
		bool exactly(const Real& a, const Real& b) { return agreed_bits_absolute(a, b) == kExactAgreement; }

		template<typename A, typename B>
		int expect_bits(const std::string& what, const A& got, const B& ref, int bits, bool reportTestCases) {
			if (agrees_to_bits(got, ref, bits)) return 0;
			if (reportTestCases) std::cout << "    FAIL " << what << ": " << measured_bits(got, ref) << " bits, wanted at least " << bits << '\n';
			return 1;
		}

		template<typename A>
		int expect_bits(const std::string& what, const A& got, std::string_view ref, int bits, bool reportTestCases) {
			if (agrees_to_bits(got, ref, bits)) return 0;
			if (reportTestCases) std::cout << "    FAIL " << what << ": " << measured_bits(got, ref) << " bits, wanted at least " << bits << '\n';
			return 1;
		}

		template<typename Real>
		int expect_exact(const std::string& what, const Real& got, const Real& ref, bool reportTestCases) {
			if (exactly(got, ref)) return 0;
			if (reportTestCases) std::cout << "    FAIL " << what << " is not exact\n";
			return 1;
		}

		// Verify sqrt function
		template<typename Real>
		int VerifySqrt(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			const int bits = wanted_bits<Real>();

			// perfect squares: sqrt is correctly rounded, so these are exact
			nrOfFailedTestCases += expect_exact("sqrt(4) == 2", sqrt(Real(4.0)), Real(2.0), reportTestCases);
			nrOfFailedTestCases += expect_exact("sqrt(9) == 3", sqrt(Real(9.0)), Real(3.0), reportTestCases);
			nrOfFailedTestCases += expect_exact("sqrt(0.25) == 0.5", sqrt(Real(0.25)), Real(0.5), reportTestCases);

			// against the 340-digit references
			nrOfFailedTestCases += expect_bits("sqrt(2) vs reference", sqrt(Real(2.0)), s_sqrt2, bits, reportTestCases);
			nrOfFailedTestCases += expect_bits("sqrt(3) vs reference", sqrt(Real(3.0)), s_sqrt3, bits, reportTestCases);
			nrOfFailedTestCases += expect_bits("sqrt(5) vs reference", sqrt(Real(5.0)), s_sqrt5, bits, reportTestCases);
			nrOfFailedTestCases += expect_bits("sqrt(0.5) vs reference", sqrt(Real(0.5)), s_inv_sqrt2, bits, reportTestCases);

			// identities: (sqrt(2))^2 == 2, (sqrt(3))^2 == 3
			Real s2 = sqrt(Real(2.0)), s3 = sqrt(Real(3.0));
			nrOfFailedTestCases += expect_bits("sqrt(2)^2 == 2", s2 * s2, Real(2.0), bits, reportTestCases);
			nrOfFailedTestCases += expect_bits("sqrt(3)^2 == 3", s3 * s3, Real(3.0), bits, reportTestCases);

			// Test: sqrt(0.0) = 0.0 (mathematically exact)
			Real zero(0.0);
			Real result = sqrt(zero);
			Real expected = 0.0;
			if (!check_exact_value(result, expected)) {
				if (reportTestCases) std::cout << "    FAIL sqrt(0.0) != 0.0 (exact)\n";
				++nrOfFailedTestCases;
			}

			// C99 F.10.4.5 special values: sqrt(-0) = -0, sqrt(+inf) = +inf, sqrt(x < 0) = NaN
			result = sqrt(-zero);
			if (!result.iszero() || !result.signbit()) {
				if (reportTestCases) std::cout << "    FAIL sqrt(-0) != -0\n";
				++nrOfFailedTestCases;
			}
			result = sqrt(Real(std::numeric_limits<double>::infinity()));
			if (!result.isinf() || result.signbit()) {
				if (reportTestCases) std::cout << "    FAIL sqrt(+inf) != +inf\n";
				++nrOfFailedTestCases;
			}
			if (!sqrt(Real(-1.0)).isnan()) {
				if (reportTestCases) std::cout << "    FAIL sqrt(-1) != NaN\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify cbrt function
		template<typename Real>
		int VerifyCbrt(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			const int bits = wanted_bits<Real>();

			// perfect cubes (cbrt is not correctly rounded, so to the configuration's bits)
			nrOfFailedTestCases += expect_bits("cbrt(8) == 2", cbrt(Real(8.0)), Real(2.0), bits, reportTestCases);
			nrOfFailedTestCases += expect_bits("cbrt(27) == 3", cbrt(Real(27.0)), Real(3.0), bits, reportTestCases);
			nrOfFailedTestCases += expect_bits("cbrt(-8) == -2", cbrt(Real(-8.0)), Real(-2.0), bits, reportTestCases);
			nrOfFailedTestCases += expect_bits("cbrt(-27) == -3", cbrt(Real(-27.0)), Real(-3.0), bits, reportTestCases);

			// identity: (cbrt(2))^3 == 2
			Real c = cbrt(Real(2.0));
			nrOfFailedTestCases += expect_bits("cbrt(2)^3 == 2", c * c * c, Real(2.0), bits, reportTestCases);
			// cbrt(2 * 2^30) == cbrt(2) * 2^10: the scaling is exact
			nrOfFailedTestCases += expect_bits("cbrt(2^31) == 2^10 cbrt(2)", cbrt(ldexp(Real(2.0), 30)), ldexp(c, 10), bits, reportTestCases);

			// Test: cbrt(0.0) = 0.0 (mathematically exact)
			Real zero(0.0);
			Real result = cbrt(zero);
			Real expected = 0.0;
			if (!check_exact_value(result, expected)) {
				if (reportTestCases) std::cout << "    FAIL cbrt(0.0) != 0.0 (exact)\n";
				++nrOfFailedTestCases;
			}
			// C99 F.10.4.1: cbrt(-0) = -0, cbrt(+-inf) = +-inf
			result = cbrt(-zero);
			if (!result.iszero() || !result.signbit()) {
				if (reportTestCases) std::cout << "    FAIL cbrt(-0) != -0\n";
				++nrOfFailedTestCases;
			}
			result = cbrt(Real(-std::numeric_limits<double>::infinity()));
			if (!result.isinf() || !result.signbit()) {
				if (reportTestCases) std::cout << "    FAIL cbrt(-inf) != -inf\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Property fuzzer over the positive domain: sqrt(x)^2==x, sqrt(x*x)==x,
		// cbrt(x)^3==x, and sqrt monotonicity.  An identity cannot see a result that is
		// self-consistently wrong, so sqrt and cbrt are also measured against the same
		// function at a wider configuration, Wide.
		template<typename Real, typename Wide>
		int VerifySqrtFuzz(bool reportTestCases, unsigned nrIterations) {
			static_assert(ereal_bits_of<Wide>::value > ereal_bits_of<Real>::value, "the reference must be wider");
			int nrOfFailedTestCases = 0;
			const int bits = wanted_bits<Real>();
			std::mt19937_64 rng(0xC1A55'1FFEULL);
			std::uniform_real_distribution<double> dist(1.0e-3, 1.0e4);
			Real one(1.0);
			for (unsigned i = 0; i < nrIterations; ++i) {
				double dx = dist(rng);
				Real x(dx);
				const std::string at = " at x=" + std::to_string(dx);
				Real s = sqrt(x);
				nrOfFailedTestCases += expect_bits("sqrt(x)^2==x" + at, s * s, x, bits, reportTestCases);
				nrOfFailedTestCases += expect_bits("sqrt(x*x)==x" + at, sqrt(x * x), x, bits, reportTestCases);
				nrOfFailedTestCases += expect_bits("sqrt(x) vs wide" + at, s, sqrt(Wide(dx)), bits, reportTestCases);
				Real c = cbrt(x);
				nrOfFailedTestCases += expect_bits("cbrt(x)^3==x" + at, c * c * c, x, bits, reportTestCases);
				nrOfFailedTestCases += expect_bits("cbrt(x) vs wide" + at, c, cbrt(Wide(dx)), bits, reportTestCases);
				if (!(sqrt(x) < sqrt(x + one))) {
					if (reportTestCases) std::cout << "    FAIL sqrt monotonic" << at << '\n';
					++nrOfFailedTestCases;
				}
			}
			return nrOfFailedTestCases;
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
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "ereal mathlib sqrt/cbrt function validation";
	std::string test_tag    = "sqrt/cbrt";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Manual test cases for visual verification
	std::cout << "Manual testing of sqrt/cbrt functions:\n";
	std::cout << "sqrt(4.0) = " << double(sqrt(ereal<>(4.0))) << " (expected: 2.0)\n";
	std::cout << "cbrt(8.0) = " << double(cbrt(ereal<>(8.0))) << " (expected: 2.0)\n";
	std::cout << "cbrt(-8.0) = " << double(cbrt(ereal<>(-8.0))) << " (expected: -2.0)\n";

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	// Phase 3 functions: sqrt, cbrt
	test_tag = "sqrt";
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<ereal<>>(reportTestCases), "sqrt(ereal)", test_tag);

	test_tag = "cbrt";
	nrOfFailedTestCases += ReportTestResult(VerifyCbrt<ereal<>>(reportTestCases), "cbrt(ereal)", test_tag);

	test_tag = "sqrt fuzz";
	// L1 (sanity tier; run by CI's Debug-instrumented ASan/UBSan/coverage jobs) takes
	// only a small fuzz smoke sample. The count scales up for the on-demand stress
	// tiers L2-L4 so QA depth is preserved where runtime is not a concern (#1007).
	unsigned sqrtFuzzCount = 15;
#if REGRESSION_LEVEL_4
	sqrtFuzzCount = 2000;
#elif REGRESSION_LEVEL_3
	sqrtFuzzCount = 500;
#elif REGRESSION_LEVEL_2
	sqrtFuzzCount = 100;
#endif
	nrOfFailedTestCases += ReportTestResult(VerifySqrtFuzz<ereal<>, ereal<19>>(reportTestCases, sqrtFuzzCount), "sqrt/cbrt property fuzz", test_tag);
#endif

#if REGRESSION_LEVEL_2
	// a narrower configuration: 4 limbs, 212 bits (ereal<> is ereal<8>, covered at level 1)
	test_tag = "sqrt narrow precision";
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<ereal<4>>(reportTestCases), "sqrt(ereal<4>)", test_tag);

	test_tag = "cbrt narrow precision";
	nrOfFailedTestCases += ReportTestResult(VerifyCbrt<ereal<4>>(reportTestCases), "cbrt(ereal<4>)", test_tag);
#endif

#if REGRESSION_LEVEL_3
	// High precision tests at 16 limbs, 848 bits
	test_tag = "sqrt very high precision";
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<ereal<16>>(reportTestCases), "sqrt(ereal<16>)", test_tag);

	test_tag = "cbrt very high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyCbrt<ereal<16>>(reportTestCases), "cbrt(ereal<16>)", test_tag);
#endif

#if REGRESSION_LEVEL_4
	// Extreme precision tests at 19 limbs, 1007 bits (the most double limbs can hold)
	test_tag = "sqrt extreme precision";
	nrOfFailedTestCases += ReportTestResult(VerifySqrt<ereal<19>>(reportTestCases), "sqrt(ereal<19>)", test_tag);

	test_tag = "cbrt extreme precision";
	nrOfFailedTestCases += ReportTestResult(VerifyCbrt<ereal<19>>(reportTestCases), "cbrt(ereal<19>)", test_tag);
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
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
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
