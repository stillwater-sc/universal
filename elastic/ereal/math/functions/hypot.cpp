// hypot.cpp: test suite runner for hypot function for ereal adaptive-precision
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <limits>
#include <random>
#include <string>
#include <string_view>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>
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
		// above that floor: ereal<4> shows 432-441 bits against 212, ereal<8> 865-880 against
		// 424.  At 16 and 19 limbs every value saturates near 1027-1044 bits, where the last
		// double limb reaches 2^-1074; the worst case is the fuzzer's hypot^2 == x^2 + y^2 at
		// 1027 bits against ereal<19>'s 1007.  16 bits of margin leaves it 36 bits of slack.
		constexpr int kMarginBits = 16;

		template<typename Real>
		constexpr int wanted_bits() { return ereal_bits_of<Real>::value - kMarginBits; }

		template<typename A, typename B>
		int expect_bits(const std::string& what, const A& got, const B& ref, int bits, bool reportTestCases) {
			if (agrees_to_bits(got, ref, bits)) return 0;
			if (reportTestCases) {
				std::cout << "    FAIL " << what << ": ";
				if (got.isnan() || got.isinf()) std::cout << "not finite";
				else std::cout << agreed_bits(got, ref) << " bits";
				std::cout << ", wanted at least " << bits << '\n';
			}
			return 1;
		}

		template<typename A>
		int expect_bits(const std::string& what, const A& got, std::string_view ref, int bits, bool reportTestCases) {
			if (agrees_to_bits(got, ref, bits)) return 0;
			if (reportTestCases) std::cout << "    FAIL " << what << ": " << agreed_bits(got, ref) << " bits, wanted at least " << bits << '\n';
			return 1;
		}

		// exact: the dyadic values are equal, whatever the limb structure
		template<typename Real>
		int expect_exact(const std::string& what, const Real& got, const Real& ref, bool reportTestCases) {
			if (agreed_bits_absolute(got, ref) == kExactAgreement) return 0;
			if (reportTestCases) std::cout << "    FAIL " << what << " is not exact\n";
			return 1;
		}

		// Verify hypot 2D function
		template<typename Real>
		int VerifyHypot2D(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			const int bits = wanted_bits<Real>();

			// Pythagorean triples: the squares and their sum are exact, so the result is the
			// correctly rounded sqrt of a perfect square
			nrOfFailedTestCases += expect_bits("hypot(3, 4) == 5", hypot(Real(3.0), Real(4.0)), Real(5.0), bits, reportTestCases);
			nrOfFailedTestCases += expect_bits("hypot(5, 12) == 13", hypot(Real(5.0), Real(12.0)), Real(13.0), bits, reportTestCases);
			nrOfFailedTestCases += expect_bits("hypot(8, 15) == 17", hypot(Real(8.0), Real(15.0)), Real(17.0), bits, reportTestCases);

			// hypot(1, 1) == sqrt(2), against the 340-digit reference, and its square
			Real h = hypot(Real(1.0), Real(1.0));
			nrOfFailedTestCases += expect_bits("hypot(1, 1) vs sqrt(2) reference", h, s_sqrt2, bits, reportTestCases);
			nrOfFailedTestCases += expect_bits("hypot(1,1)^2 == 2", h * h, Real(2.0), bits, reportTestCases);

			// Test: hypot(0, 0) = 0 (mathematically exact)
			Real result = hypot(Real(0.0), Real(0.0));
			if (!result.iszero()) {
				if (reportTestCases) std::cout << "    FAIL hypot(0, 0) != 0 (not zero)\n";
				++nrOfFailedTestCases;
			}

			// hypot(x, 0) == |x|
			nrOfFailedTestCases += expect_bits("hypot(3, 0) == 3", hypot(Real(3.0), Real(0.0)), Real(3.0), bits, reportTestCases);
			nrOfFailedTestCases += expect_bits("hypot(-3, 0) == 3", hypot(Real(-3.0), Real(0.0)), Real(3.0), bits, reportTestCases);

			// C99 F.10.4.3: hypot(+-inf, y) = +inf even for a NaN y
			const double inf = std::numeric_limits<double>::infinity();
			result = hypot(Real(inf), Real(std::numeric_limits<double>::quiet_NaN()));
			if (!result.isinf() || result.signbit()) {
				if (reportTestCases) std::cout << "    FAIL hypot(inf, NaN) != +inf\n";
				++nrOfFailedTestCases;
			}
			result = hypot(Real(1.0), Real(-inf));
			if (!result.isinf() || result.signbit()) {
				if (reportTestCases) std::cout << "    FAIL hypot(1, -inf) != +inf\n";
				++nrOfFailedTestCases;
			}
			// no overflow in the squares: hypot(1e200, 1e200) == 1e200 sqrt(2)
			Real big(1.0e200);
			nrOfFailedTestCases += expect_bits("hypot(1e200, 1e200) == 1e200 sqrt(2)", hypot(big, big), big * sqrt(Real(2.0)), bits, reportTestCases);

			return nrOfFailedTestCases;
		}

		// Verify hypot 3D function
		template<typename Real>
		int VerifyHypot3D(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			const int bits = wanted_bits<Real>();

			// Test: hypot(0, 0, 0) = 0 (mathematically exact)
			Real result = hypot(Real(0.0), Real(0.0), Real(0.0));
			if (!result.iszero()) {
				if (reportTestCases) std::cout << "    FAIL hypot(0, 0, 0) != 0 (not zero)\n";
				++nrOfFailedTestCases;
			}

			// Pythagorean quadruple: hypot(2, 3, 6) == 7
			nrOfFailedTestCases += expect_bits("hypot(2, 3, 6) == 7", hypot(Real(2.0), Real(3.0), Real(6.0)), Real(7.0), bits, reportTestCases);
			nrOfFailedTestCases += expect_bits("hypot(1, 4, 8) == 9", hypot(Real(1.0), Real(4.0), Real(8.0)), Real(9.0), bits, reportTestCases);

			// hypot(1, 1, 1) == sqrt(3), the unit cube diagonal, against the 340-digit reference
			nrOfFailedTestCases += expect_bits("hypot(1, 1, 1) vs sqrt(3) reference", hypot(Real(1.0), Real(1.0), Real(1.0)), s_sqrt3, bits, reportTestCases);

			return nrOfFailedTestCases;
		}

		// Property fuzzer: hypot(x,y)^2==x^2+y^2, hypot(x,0)==|x|, symmetry, and
		// the hypot(x,y) >= max(|x|,|y|) lower bound.  The identity cannot see a result that
		// is self-consistently wrong, so hypot is also measured against Wide, a wider
		// configuration.
		template<typename Real, typename Wide>
		int VerifyHypotFuzz(bool reportTestCases, unsigned nrIterations) {
			static_assert(ereal_bits_of<Wide>::value > ereal_bits_of<Real>::value, "the reference must be wider");
			int nrOfFailedTestCases = 0;
			const int bits = wanted_bits<Real>();
			std::mt19937_64 rng(0xC1A55'1FFEULL);
			std::uniform_real_distribution<double> dist(-1.0e3, 1.0e3);
			for (unsigned i = 0; i < nrIterations; ++i) {
				const double dx = dist(rng), dy = dist(rng);
				Real x(dx), y(dy);
				const std::string at = " at x=" + std::to_string(dx) + " y=" + std::to_string(dy);
				Real h = hypot(x, y);
				nrOfFailedTestCases += expect_bits("hypot^2==x^2+y^2" + at, h * h, x * x + y * y, bits, reportTestCases);
				nrOfFailedTestCases += expect_bits("hypot vs wide" + at, h, hypot(Wide(dx), Wide(dy)), bits, reportTestCases);
				nrOfFailedTestCases += expect_bits("hypot(x,0)==|x|" + at, hypot(x, Real(0.0)), abs(x), bits, reportTestCases);
				// symmetry: the operands are sorted by magnitude first, so it is exact
				nrOfFailedTestCases += expect_exact("hypot symmetry" + at, h, hypot(y, x), reportTestCases);
				// lower bound, compared in ereal
				Real m = (abs(x) < abs(y)) ? abs(y) : abs(x);
				if (h < m) {
					if (reportTestCases) std::cout << "    FAIL hypot lower bound" << at << '\n';
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

	std::string test_suite  = "ereal mathlib hypot function validation";
	std::string test_tag    = "hypot";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Manual test cases for visual verification
	std::cout << "Manual testing of hypot functions:\n";
	std::cout << "hypot(3, 4) = " << double(hypot(ereal<>(3.0), ereal<>(4.0))) << " (expected: 5.0)\n";
	std::cout << "hypot(2, 3, 6) = " << double(hypot(ereal<>(2.0), ereal<>(3.0), ereal<>(6.0))) << " (expected: 7.0)\n";

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	// Phase 3 functions: hypot 2D and 3D
	test_tag = "hypot 2D";
	nrOfFailedTestCases += ReportTestResult(VerifyHypot2D<ereal<>>(reportTestCases), "hypot(ereal, ereal)", test_tag);

	test_tag = "hypot 3D";
	nrOfFailedTestCases += ReportTestResult(VerifyHypot3D<ereal<>>(reportTestCases), "hypot(ereal, ereal, ereal)", test_tag);

	test_tag = "hypot fuzz";
	// L1 (sanity tier; run by CI's Debug-instrumented ASan/UBSan/coverage jobs) takes
	// only a small fuzz smoke sample. The count scales up for the on-demand stress
	// tiers L2-L4 so QA depth is preserved where runtime is not a concern (#1007).
	unsigned hypotFuzzCount = 15;
#if REGRESSION_LEVEL_4
	hypotFuzzCount = 2000;
#elif REGRESSION_LEVEL_3
	hypotFuzzCount = 500;
#elif REGRESSION_LEVEL_2
	hypotFuzzCount = 100;
#endif
	nrOfFailedTestCases += ReportTestResult(VerifyHypotFuzz<ereal<>, ereal<19>>(reportTestCases, hypotFuzzCount), "hypot property fuzz", test_tag);
#endif

#if REGRESSION_LEVEL_2
	// a narrower configuration: 4 limbs, 212 bits (ereal<> is ereal<8>, covered at level 1)
	test_tag = "hypot 2D narrow precision";
	nrOfFailedTestCases += ReportTestResult(VerifyHypot2D<ereal<4>>(reportTestCases), "hypot(ereal<4>, ereal<4>)", test_tag);

	test_tag = "hypot 3D narrow precision";
	nrOfFailedTestCases += ReportTestResult(VerifyHypot3D<ereal<4>>(reportTestCases), "hypot(ereal<4>, ereal<4>, ereal<4>)", test_tag);
#endif

#if REGRESSION_LEVEL_3
	// High precision tests at 16 limbs, 848 bits
	test_tag = "hypot 2D very high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyHypot2D<ereal<16>>(reportTestCases), "hypot(ereal<16>, ereal<16>)", test_tag);

	test_tag = "hypot 3D very high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyHypot3D<ereal<16>>(reportTestCases), "hypot(ereal<16>, ereal<16>, ereal<16>)", test_tag);
#endif

#if REGRESSION_LEVEL_4
	// Extreme precision tests at 19 limbs, 1007 bits (the most double limbs can hold)
	test_tag = "hypot 2D extreme precision";
	nrOfFailedTestCases += ReportTestResult(VerifyHypot2D<ereal<19>>(reportTestCases), "hypot(ereal<19>, ereal<19>)", test_tag);

	test_tag = "hypot 3D extreme precision";
	nrOfFailedTestCases += ReportTestResult(VerifyHypot3D<ereal<19>>(reportTestCases), "hypot(ereal<19>, ereal<19>, ereal<19>)", test_tag);
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
