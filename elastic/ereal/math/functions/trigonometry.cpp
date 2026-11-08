// trigonometry.cpp: test suite runner for trigonometric functions for ereal adaptive-precision
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <algorithm>
#include <cmath>
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

	// The precision checks below are exact (ereal_precision.hpp): every value is compared
	// with its reference as a dyadic rational and the agreement is counted in bits. The
	// tolerances of 1e-14 they replace went through double and so passed a result correct
	// to only 53 bits (#2008).
	//
	// References are, in order of preference, a stored decimal constant
	// (reference_constants.hpp), an exact value, or the same function at a wider
	// configuration (ereal<19>, the widest a double limb supports). An identity such as
	// sin^2 + cos^2 == 1 is checked as well, but cannot by itself see a value that is
	// consistently wrong in both of its terms.
	//
	// Each check asks for the bits the configuration holds less a margin. The margins
	// were measured (compile with -DEREAL_PRECISION_TRACE to print every agreement) and
	// carry slack over the worst value seen at ereal<8>, <16> and <19>: every check measured
	// at or above the configuration's own bits (results carry spare limbs).
	constexpr int kFunctionMargin = 16;   // a function at a stored constant, an exact value or the wide configuration
	constexpr int kIdentityMargin = 16;   // an identity or a round trip through two functions
	using Wide = ereal<19>;               // the wide reference configuration

	int expect_bits(int got, int wanted, const std::string& what, bool reportTestCases) {
#ifdef EREAL_PRECISION_TRACE
		std::cout << "    " << what << ": " << got << " bits (wanted " << wanted << ")\n";
#endif
		if (got >= wanted) return 0;
		if (reportTestCases) std::cout << "    FAIL " << what << ": " << got << " bits, wanted at least " << wanted << '\n';
		return 1;
	}

	// bits to which v agrees with a decimal reference, which may carry a sign; -1 for a
	// value that cannot agree (NaN, inf, zero)
	template<typename Real>
	int bits_vs(const Real& v, std::string_view ref) {
		if (v.isnan() || v.isinf() || v.iszero()) return -1;
		Real w = v;
		if (!ref.empty() && ref.front() == '-') { ref.remove_prefix(1); w = -v; }
		// digits enough to certify the configuration, within the 320 the references certify
		const int cap = std::min(320, static_cast<int>(ereal_bits_of<Real>::value * 0.30103) + 4);
		return agreed_bits(w, ref, cap);
	}

	// bits to which a agrees with b, relative to b; -1 for NaN or inf
	template<unsigned m, typename F, unsigned n, typename G>
	int bits_vs(const ereal<m, F>& a, const ereal<n, G>& b) {
		if (a.isnan() || a.isinf() || b.isnan() || b.isinf()) return -1;
		if (b.iszero()) return a.iszero() ? kExactAgreement : -1;
		return agreed_bits(a, b);
	}

	// the configuration's precision less a margin
	template<typename Real>
	constexpr int wanted_bits(int margin) { return ereal_bits_of<Real>::value - margin; }

	// f(x) at Real against f(x) at the wide configuration, when Real is narrower
	template<typename Real, typename Fn>
	int expect_vs_wide(Fn f, double x, const std::string& what, bool reportTestCases) {
		if constexpr (ereal_bits_of<Real>::value < ereal_bits_of<Wide>::value) {
			return expect_bits(bits_vs(f(Real(x)), f(Wide(x))), wanted_bits<Real>(kFunctionMargin), what + " vs ereal<19>", reportTestCases);
		}
		else {
			return 0;
		}
	}

		// Verify sin function
		template<typename Real>
		int VerifySin(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			constexpr unsigned N = Real::maxNrLimbs;
			const int wanted = wanted_bits<Real>(kFunctionMargin);

			// Test: sin(0) = 0 (mathematically exact)
			Real x(0.0), expected(0.0);
			Real result = sin(x);
			if (!check_exact_value(result, expected)) {
				if (reportTestCases) std::cout << "    FAIL sin(0) != 0 (exact)\n";
				++nrOfFailedTestCases;
			}

			// Test: sin(1/2) against its stored value
			nrOfFailedTestCases += expect_bits(bits_vs(sin(Real(0.5)), s_sin_half), wanted, "sin(1/2)", reportTestCases);

			// Test: sin(pi/6) = 1/2, with pi/6 at the configuration's precision
			x = ereal_pi<N, double>() / Real(6.0);
			nrOfFailedTestCases += expect_bits(bits_vs(sin(x), Real(0.5)), wanted, "sin(pi/6) == 1/2", reportTestCases);

			// Test: sin(pi/2) = 1
			nrOfFailedTestCases += expect_bits(bits_vs(sin(ereal_pi_2<N, double>()), Real(1.0)), wanted, "sin(pi/2) == 1", reportTestCases);

			// Test: sin(1) against the wide configuration
			nrOfFailedTestCases += expect_vs_wide<Real>([](const auto& v) { return sin(v); }, 1.0, "sin(1)", reportTestCases);

			// Test: sin(-x) = -sin(x) (odd function identity, exact)
			x = 1.0;
			if (!(sin(-x) == -sin(x))) {
				if (reportTestCases) std::cout << "    FAIL sin(-1) != -sin(1) (exact)\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify cos function
		template<typename Real>
		int VerifyCos(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			constexpr unsigned N = Real::maxNrLimbs;
			const int wanted = wanted_bits<Real>(kFunctionMargin);

			// Test: cos(0) = 1 (mathematically exact)
			Real x(0.0), expected(1.0);
			Real result = cos(x);
			if (!check_exact_value(result, expected)) {
				if (reportTestCases) std::cout << "    FAIL cos(0) != 1 (exact)\n";
				++nrOfFailedTestCases;
			}

			// Test: cos(1/2) against its stored value
			nrOfFailedTestCases += expect_bits(bits_vs(cos(Real(0.5)), s_cos_half), wanted, "cos(1/2)", reportTestCases);

			// Test: cos(pi/3) = 1/2, with pi/3 at the configuration's precision
			nrOfFailedTestCases += expect_bits(bits_vs(cos(ereal_pi_3<N, double>()), Real(0.5)), wanted, "cos(pi/3) == 1/2", reportTestCases);

			// Test: cos(1) against the wide configuration
			nrOfFailedTestCases += expect_vs_wide<Real>([](const auto& v) { return cos(v); }, 1.0, "cos(1)", reportTestCases);

			// Test: cos(-x) = cos(x) (even function identity, exact)
			x = 1.0;
			if (!(cos(-x) == cos(x))) {
				if (reportTestCases) std::cout << "    FAIL cos(-1) != cos(1) (exact)\n";
				++nrOfFailedTestCases;
			}

			// Test: sin^2(x) + cos^2(x) = 1 (Pythagorean identity)
			x = 0.7;
			Real sin_x = sin(x);
			Real cos_x = cos(x);
			Real identity = sin_x * sin_x + cos_x * cos_x;
			nrOfFailedTestCases += expect_bits(bits_vs(identity, Real(1.0)), wanted_bits<Real>(kIdentityMargin), "sin^2(0.7) + cos^2(0.7) == 1", reportTestCases);

			return nrOfFailedTestCases;
		}

		// Verify tan function
		template<typename Real>
		int VerifyTan(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			constexpr unsigned N = Real::maxNrLimbs;
			const int wanted = wanted_bits<Real>(kFunctionMargin);

			// Test: tan(0) = 0 (mathematically exact)
			Real x(0.0), expected(0.0);
			Real result = tan(x);
			if (!check_exact_value(result, expected)) {
				if (reportTestCases) std::cout << "    FAIL tan(0) != 0 (exact)\n";
				++nrOfFailedTestCases;
			}

			// Test: tan(1/2) against its stored value
			nrOfFailedTestCases += expect_bits(bits_vs(tan(Real(0.5)), s_tan_half), wanted, "tan(1/2)", reportTestCases);

			// Test: tan(pi/4) = 1, with pi/4 at the configuration's precision
			nrOfFailedTestCases += expect_bits(bits_vs(tan(ereal_pi_4<N, double>()), Real(1.0)), wanted, "tan(pi/4) == 1", reportTestCases);

			// Test: tan(1) against the wide configuration
			nrOfFailedTestCases += expect_vs_wide<Real>([](const auto& v) { return tan(v); }, 1.0, "tan(1)", reportTestCases);

			// Test: tan(-x) = -tan(x) (odd function identity, exact)
			x = 0.5;
			if (!(tan(-x) == -tan(x))) {
				if (reportTestCases) std::cout << "    FAIL tan(-0.5) != -tan(0.5) (exact)\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify atan function
		template<typename Real>
		int VerifyAtan(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			const int wanted = wanted_bits<Real>(kFunctionMargin);

			// Test: atan(0) = 0 (mathematically exact)
			Real x(0.0), expected(0.0);
			Real result = atan(x);
			if (!check_exact_value(result, expected)) {
				if (reportTestCases) std::cout << "    FAIL atan(0) != 0 (exact)\n";
				++nrOfFailedTestCases;
			}

			// Test: atan(1) = pi/4 (this used to be allowed a relative error of 4e-3)
			nrOfFailedTestCases += expect_bits(bits_vs(atan(Real(1.0)), s_pi_4), wanted, "atan(1) == pi/4", reportTestCases);

			// Test: atan(1/2) against the wide configuration
			nrOfFailedTestCases += expect_vs_wide<Real>([](const auto& v) { return atan(v); }, 0.5, "atan(1/2)", reportTestCases);

			// Test: atan(tan(x)) = x for |x| < pi/2 (round trip)
			x = 0.5;
			nrOfFailedTestCases += expect_bits(bits_vs(atan(tan(x)), x), wanted_bits<Real>(kIdentityMargin), "atan(tan(1/2)) == 1/2", reportTestCases);

			return nrOfFailedTestCases;
		}

		// Verify asin function
		template<typename Real>
		int VerifyAsin(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			const int wanted = wanted_bits<Real>(kFunctionMargin);

			// Test: asin(0) = 0 (mathematically exact)
			Real x(0.0), expected(0.0);
			Real result = asin(x);
			if (!check_exact_value(result, expected)) {
				if (reportTestCases) std::cout << "    FAIL asin(0) != 0 (exact)\n";
				++nrOfFailedTestCases;
			}

			// Test: asin(1) = pi/2, asin(1/2) = pi/6
			nrOfFailedTestCases += expect_bits(bits_vs(asin(Real(1.0)), s_pi_2), wanted, "asin(1) == pi/2", reportTestCases);
			nrOfFailedTestCases += expect_bits(bits_vs(asin(Real(0.5)), s_pi_6), wanted, "asin(1/2) == pi/6", reportTestCases);

			// The top of the series range. asin switched to its sqrt identity at 0.8, and the
			// series stopped at its iteration cap before converging for 0.7 < |x| <= 0.8: asin and
			// acos lost up to 100 bits at ereal<8> (#2006). The switch is at 1/sqrt(2) now.
			for (double v : { 0.7, 0.7071, 0.7072, 0.75, 0.78, 0.8, -0.79 }) {
				nrOfFailedTestCases += expect_vs_wide<Real>([](const auto& a) { return asin(a); }, v, "asin(" + std::to_string(v) + ")", reportTestCases);
				nrOfFailedTestCases += expect_vs_wide<Real>([](const auto& a) { return acos(a); }, v, "acos(" + std::to_string(v) + ")", reportTestCases);
			}

			// Test: asin(sin(x)) = x for |x| <= pi/2 (round trip; this used to be allowed 4e-3)
			x = 0.5;
			nrOfFailedTestCases += expect_bits(bits_vs(asin(sin(x)), x), wanted_bits<Real>(kIdentityMargin), "asin(sin(1/2)) == 1/2", reportTestCases);

			return nrOfFailedTestCases;
		}

		// Verify acos function
		template<typename Real>
		int VerifyAcos(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			const int wanted = wanted_bits<Real>(kFunctionMargin);

			// Test: acos(1) = 0 (exact)
			Real x(1.0);
			Real result = acos(x);
			if (!result.iszero()) {
				if (reportTestCases) std::cout << "    FAIL acos(1) != 0 (exact)\n";
				++nrOfFailedTestCases;
			}

			// Test: acos(0) = pi/2, acos(1/2) = pi/3
			nrOfFailedTestCases += expect_bits(bits_vs(acos(Real(0.0)), s_pi_2), wanted, "acos(0) == pi/2", reportTestCases);
			nrOfFailedTestCases += expect_bits(bits_vs(acos(Real(0.5)), s_pi_3), wanted, "acos(1/2) == pi/3", reportTestCases);

			// Test: acos(cos(x)) = x for 0 <= x <= pi (round trip; this used to be allowed 4e-3)
			x = 0.5;
			nrOfFailedTestCases += expect_bits(bits_vs(acos(cos(x)), x), wanted_bits<Real>(kIdentityMargin), "acos(cos(1/2)) == 1/2", reportTestCases);

			return nrOfFailedTestCases;
		}

		// Verify atan2 function
		template<typename Real>
		int VerifyAtan2(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			const int wanted = wanted_bits<Real>(kFunctionMargin);

			// Test: atan2(1, 1) = pi/4 and atan2(1, -1) = 3pi/4 (these used to be allowed 4e-3)
			Real y(1.0), x(1.0);
			nrOfFailedTestCases += expect_bits(bits_vs(atan2(y, x), s_pi_4), wanted, "atan2(1, 1) == pi/4", reportTestCases);
			x = -1.0;
			nrOfFailedTestCases += expect_bits(bits_vs(atan2(y, x), s_three_pi_4), wanted, "atan2(1, -1) == 3pi/4", reportTestCases);
			// and the other two quadrants
			y = -1.0;
			nrOfFailedTestCases += expect_bits(bits_vs(-atan2(y, x), s_three_pi_4), wanted, "atan2(-1, -1) == -3pi/4", reportTestCases);
			x = 1.0;
			nrOfFailedTestCases += expect_bits(bits_vs(-atan2(y, x), s_pi_4), wanted, "atan2(-1, 1) == -pi/4", reportTestCases);

			// Test: atan2(0, 1) = 0 (mathematically exact)
			y = 0.0; x = 1.0;
			Real result = atan2(y, x);
			Real expected = 0.0;
			if (!check_exact_value(result, expected)) {
				if (reportTestCases) std::cout << "    FAIL atan2(0, 1) != 0 (exact)\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Property fuzzer over the principal domain: sin and cos against a wider
		// configuration, the Pythagorean identity, the tan==sin/cos quotient, sin/cos
		// parity, and the inverse relations asin(sin(x))==x, atan(tan(x))==x,
		// atan2(sin x, cos x)==x.
		template<typename Real, typename FuzzWide>
		int VerifyTrigonometryFuzz(bool reportTestCases, unsigned nrIterations) {
			int nrOfFailedTestCases = 0;
			std::mt19937_64 rng(0xC1A55'1FFEULL);
			std::uniform_real_distribution<double> dist(-1.4, 1.4);  // within (-pi/2, pi/2)
			const Real one(1.0);
			const int wanted   = wanted_bits<Real>(kFunctionMargin);
			const int identity = wanted_bits<Real>(kIdentityMargin);
			for (unsigned i = 0; i < nrIterations; ++i) {
				double dx = dist(rng);
				Real x(dx);
				const std::string at = " at x=" + std::to_string(dx);
				const Real s = sin(x), c = cos(x);
				const FuzzWide wx(dx);
				nrOfFailedTestCases += expect_bits(bits_vs(s, sin(wx)), wanted, "sin vs wide" + at, reportTestCases);
				nrOfFailedTestCases += expect_bits(bits_vs(c, cos(wx)), wanted, "cos vs wide" + at, reportTestCases);
				nrOfFailedTestCases += expect_bits(bits_vs(s * s + c * c, one), identity, "sin^2+cos^2==1" + at, reportTestCases);
				nrOfFailedTestCases += expect_bits(bits_vs(tan(x), s / c), identity, "tan==sin/cos" + at, reportTestCases);
				if (sin(-x) != -s || cos(-x) != c) {
					if (reportTestCases) std::cout << "    FAIL parity" << at << '\n';
					++nrOfFailedTestCases;
				}
				nrOfFailedTestCases += expect_bits(bits_vs(asin(s), x), identity, "asin(sin(x))==x" + at, reportTestCases);
				nrOfFailedTestCases += expect_bits(bits_vs(atan(tan(x)), x), identity, "atan(tan(x))==x" + at, reportTestCases);
				nrOfFailedTestCases += expect_bits(bits_vs(atan2(s, c), x), identity, "atan2(sin,cos)==x" + at, reportTestCases);
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

	std::string test_suite  = "ereal mathlib trigonometric function validation";
	std::string test_tag    = "trigonometric";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Manual test cases for visual verification
	std::cout << "Manual testing of trigonometric functions:\n";
	std::cout << "sin(1) = " << double(sin(ereal<>(1.0))) << " (expected: " << std::sin(1.0) << ")\n";
	std::cout << "cos(1) = " << double(cos(ereal<>(1.0))) << " (expected: " << std::cos(1.0) << ")\n";
	std::cout << "tan(1) = " << double(tan(ereal<>(1.0))) << " (expected: " << std::tan(1.0) << ")\n";
	std::cout << "asin(0.5) = " << double(asin(ereal<>(0.5))) << " (expected: " << std::asin(0.5) << ")\n";
	std::cout << "acos(0.5) = " << double(acos(ereal<>(0.5))) << " (expected: " << std::acos(0.5) << ")\n";
	std::cout << "atan(1.0) = " << double(atan(ereal<>(1.0))) << " (expected: " << std::atan(1.0) << ")\n";
	std::cout << "atan2(1, 1) = " << double(atan2(ereal<>(1.0), ereal<>(1.0))) << " (expected: " << std::atan2(1.0, 1.0) << ")\n";

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	// Phase 6 functions: sin, cos, tan, asin, acos, atan, atan2
	test_tag = "sin";
	nrOfFailedTestCases += ReportTestResult(VerifySin<ereal<>>(reportTestCases), "sin(ereal)", test_tag);

	test_tag = "cos";
	nrOfFailedTestCases += ReportTestResult(VerifyCos<ereal<>>(reportTestCases), "cos(ereal)", test_tag);

	test_tag = "tan";
	nrOfFailedTestCases += ReportTestResult(VerifyTan<ereal<>>(reportTestCases), "tan(ereal)", test_tag);

	test_tag = "atan";
	nrOfFailedTestCases += ReportTestResult(VerifyAtan<ereal<>>(reportTestCases), "atan(ereal)", test_tag);

	test_tag = "asin";
	nrOfFailedTestCases += ReportTestResult(VerifyAsin<ereal<>>(reportTestCases), "asin(ereal)", test_tag);

	test_tag = "acos";
	nrOfFailedTestCases += ReportTestResult(VerifyAcos<ereal<>>(reportTestCases), "acos(ereal)", test_tag);

	test_tag = "atan2";
	nrOfFailedTestCases += ReportTestResult(VerifyAtan2<ereal<>>(reportTestCases), "atan2(ereal)", test_tag);

	test_tag = "trig fuzz";
	// L1 (sanity tier; run by CI's Debug-instrumented ASan/UBSan/coverage jobs) takes
	// only a small fuzz smoke sample. The count scales up for the on-demand stress
	// tiers L2-L4 so QA depth is preserved where runtime is not a concern (#1007).
	unsigned trigFuzzCount = 15;
#if REGRESSION_LEVEL_4
	trigFuzzCount = 2000;
#elif REGRESSION_LEVEL_3
	trigFuzzCount = 500;
#elif REGRESSION_LEVEL_2
	trigFuzzCount = 100;
#endif
	nrOfFailedTestCases += ReportTestResult(VerifyTrigonometryFuzz<ereal<>, ereal<16>>(reportTestCases, trigFuzzCount), "trig property fuzz", test_tag);
#endif

#if REGRESSION_LEVEL_2
	// Extended precision tests at 512 bits (~=154 decimal digits)
	test_tag = "sin high precision";
	nrOfFailedTestCases += ReportTestResult(VerifySin<ereal<8>>(reportTestCases), "sin(ereal<8>)", test_tag);

	test_tag = "cos high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyCos<ereal<8>>(reportTestCases), "cos(ereal<8>)", test_tag);

	test_tag = "tan high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyTan<ereal<8>>(reportTestCases), "tan(ereal<8>)", test_tag);

	test_tag = "atan high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyAtan<ereal<8>>(reportTestCases), "atan(ereal<8>)", test_tag);

	test_tag = "asin high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyAsin<ereal<8>>(reportTestCases), "asin(ereal<8>)", test_tag);

	test_tag = "acos high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyAcos<ereal<8>>(reportTestCases), "acos(ereal<8>)", test_tag);

	test_tag = "atan2 high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyAtan2<ereal<8>>(reportTestCases), "atan2(ereal<8>)", test_tag);
#endif

#if REGRESSION_LEVEL_3
	// High precision tests at 1024 bits (~=308 decimal digits)
	test_tag = "sin very high precision";
	nrOfFailedTestCases += ReportTestResult(VerifySin<ereal<16>>(reportTestCases), "sin(ereal<16>)", test_tag);

	test_tag = "cos very high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyCos<ereal<16>>(reportTestCases), "cos(ereal<16>)", test_tag);

	test_tag = "tan very high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyTan<ereal<16>>(reportTestCases), "tan(ereal<16>)", test_tag);
#endif

#if REGRESSION_LEVEL_4
	// Extreme precision tests at 1216 bits (~=366 decimal digits, maximum algorithmically valid)
	test_tag = "sin extreme precision";
	nrOfFailedTestCases += ReportTestResult(VerifySin<ereal<19>>(reportTestCases), "sin(ereal<19>)", test_tag);

	test_tag = "cos extreme precision";
	nrOfFailedTestCases += ReportTestResult(VerifyCos<ereal<19>>(reportTestCases), "cos(ereal<19>)", test_tag);
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
