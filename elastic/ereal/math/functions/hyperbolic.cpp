// hyperbolic.cpp: test suite runner for hyperbolic functions for ereal adaptive-precision
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
	// configuration (ereal<19>, the widest a double limb supports). An inverse is also
	// checked by applying the forward function, which the stored constants pin down
	// independently: sinh(asinh(2)) == 2.
	//
	// Each check asks for the bits the configuration holds less a margin. The margins
	// were measured (compile with -DEREAL_PRECISION_TRACE to print every agreement) and
	// carry slack over the worst value seen at ereal<8>, <16> and <19>: every check measured
	// at or above the configuration's own bits (results carry spare limbs).
	constexpr int kFunctionMargin = 16;   // a function at a stored constant, an exact value or the wide configuration
	constexpr int kIdentityMargin = 24;   // an identity or a round trip: cosh^2 - sinh^2 at |x| = 3 can cancel ~7 bits
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

		// Verify sinh function
		template<typename Real>
		int VerifySinh(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			const int wanted = wanted_bits<Real>(kFunctionMargin);

			// Test: sinh(0) = 0 (mathematically exact)
			Real x(0.0), expected(0.0);
			Real result = sinh(x);
			if (!check_exact_value(result, expected)) {
				if (reportTestCases) std::cout << "    FAIL sinh(0) != 0 (exact)\n";
				++nrOfFailedTestCases;
			}

			// Test: sinh(1/2) against its stored value, sinh(1) against the wide configuration
			nrOfFailedTestCases += expect_bits(bits_vs(sinh(Real(0.5)), s_sinh_half), wanted, "sinh(1/2)", reportTestCases);
			nrOfFailedTestCases += expect_vs_wide<Real>([](const auto& v) { return sinh(v); }, 1.0, "sinh(1)", reportTestCases);

			// Test: sinh(-x) = -sinh(x) (odd function identity, exact)
			x = 2.0;
			if (!(sinh(-x) == -sinh(x))) {
				if (reportTestCases) std::cout << "    FAIL sinh(-2) != -sinh(2) (exact)\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify cosh function
		template<typename Real>
		int VerifyCosh(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			const int wanted = wanted_bits<Real>(kFunctionMargin);

			// Test: cosh(0) = 1 (mathematically exact)
			Real x(0.0), expected(1.0);
			Real result = cosh(x);
			if (!check_exact_value(result, expected)) {
				if (reportTestCases) std::cout << "    FAIL cosh(0) != 1 (exact)\n";
				++nrOfFailedTestCases;
			}

			// Test: cosh(1/2) against its stored value, cosh(1) against the wide configuration
			nrOfFailedTestCases += expect_bits(bits_vs(cosh(Real(0.5)), s_cosh_half), wanted, "cosh(1/2)", reportTestCases);
			nrOfFailedTestCases += expect_vs_wide<Real>([](const auto& v) { return cosh(v); }, 1.0, "cosh(1)", reportTestCases);

			// Test: cosh(-x) = cosh(x) (even function identity, exact)
			x = 2.0;
			if (!(cosh(-x) == cosh(x))) {
				if (reportTestCases) std::cout << "    FAIL cosh(-2) != cosh(2) (exact)\n";
				++nrOfFailedTestCases;
			}

			// Test: cosh^2(x) - sinh^2(x) = 1 (fundamental hyperbolic identity)
			x = 1.5;
			Real cosh_x = cosh(x);
			Real sinh_x = sinh(x);
			Real identity = cosh_x * cosh_x - sinh_x * sinh_x;
			nrOfFailedTestCases += expect_bits(bits_vs(identity, Real(1.0)), wanted_bits<Real>(kIdentityMargin), "cosh^2(1.5) - sinh^2(1.5) == 1", reportTestCases);

			return nrOfFailedTestCases;
		}

		// Verify tanh function
		template<typename Real>
		int VerifyTanh(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			const int wanted = wanted_bits<Real>(kFunctionMargin);

			// Test: tanh(0) = 0 (mathematically exact)
			Real x(0.0), expected(0.0);
			Real result = tanh(x);
			if (!check_exact_value(result, expected)) {
				if (reportTestCases) std::cout << "    FAIL tanh(0) != 0 (exact)\n";
				++nrOfFailedTestCases;
			}

			// Test: tanh(1/2) against its stored value, tanh(1) against the wide configuration
			nrOfFailedTestCases += expect_bits(bits_vs(tanh(Real(0.5)), s_tanh_half), wanted, "tanh(1/2)", reportTestCases);
			nrOfFailedTestCases += expect_vs_wide<Real>([](const auto& v) { return tanh(v); }, 1.0, "tanh(1)", reportTestCases);

			// Test: tanh(-x) = -tanh(x) (odd function identity, exact)
			x = 2.0;
			if (!(tanh(-x) == -tanh(x))) {
				if (reportTestCases) std::cout << "    FAIL tanh(-2) != -tanh(2) (exact)\n";
				++nrOfFailedTestCases;
			}

			// Test: |tanh(x)| < 1 for all x (mathematical bound, in ereal: tanh(10) = 1 - 4e-9)
			x = 10.0;
			result = tanh(x);
			if (!(abs(result) < Real(1.0))) {
				if (reportTestCases) std::cout << "    FAIL |tanh(x)| >= 1 (bound violation)\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify asinh function
		template<typename Real>
		int VerifyAsinh(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: asinh(0) = 0 (mathematically exact)
			Real x(0.0), expected(0.0);
			Real result = asinh(x);
			if (!check_exact_value(result, expected)) {
				if (reportTestCases) std::cout << "    FAIL asinh(0) != 0 (exact)\n";
				++nrOfFailedTestCases;
			}

			// Test: asinh(sinh(x)) = x (round trip)
			x = 1.5;
			nrOfFailedTestCases += expect_bits(bits_vs(asinh(sinh(x)), x), wanted_bits<Real>(kIdentityMargin), "asinh(sinh(1.5)) == 1.5", reportTestCases);

			// Test: asinh(2), through sinh(asinh(2)) == 2 and against the wide configuration
			x = 2.0;
			nrOfFailedTestCases += expect_bits(bits_vs(sinh(asinh(x)), x), wanted_bits<Real>(kIdentityMargin), "sinh(asinh(2)) == 2", reportTestCases);
			nrOfFailedTestCases += expect_vs_wide<Real>([](const auto& v) { return asinh(v); }, 2.0, "asinh(2)", reportTestCases);

			return nrOfFailedTestCases;
		}

		// Verify acosh function
		template<typename Real>
		int VerifyAcosh(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: acosh(1) = 0 (mathematically exact)
			Real x(1.0), expected(0.0);
			Real result = acosh(x);
			if (!check_exact_value(result, expected)) {
				if (reportTestCases) std::cout << "    FAIL acosh(1) != 0 (exact)\n";
				++nrOfFailedTestCases;
			}

			// Test: acosh(cosh(x)) = x for x > 0 (round trip)
			x = 1.5;
			nrOfFailedTestCases += expect_bits(bits_vs(acosh(cosh(x)), x), wanted_bits<Real>(kIdentityMargin), "acosh(cosh(1.5)) == 1.5", reportTestCases);

			// Test: acosh(2), through cosh(acosh(2)) == 2 and against the wide configuration
			x = 2.0;
			nrOfFailedTestCases += expect_bits(bits_vs(cosh(acosh(x)), x), wanted_bits<Real>(kIdentityMargin), "cosh(acosh(2)) == 2", reportTestCases);
			nrOfFailedTestCases += expect_vs_wide<Real>([](const auto& v) { return acosh(v); }, 2.0, "acosh(2)", reportTestCases);

			return nrOfFailedTestCases;
		}

		// Verify atanh function
		template<typename Real>
		int VerifyAtanh(bool reportTestCases) {
			int nrOfFailedTestCases = 0;

			// Test: atanh(0) = 0 (mathematically exact)
			Real x(0.0), expected(0.0);
			Real result = atanh(x);
			if (!check_exact_value(result, expected)) {
				if (reportTestCases) std::cout << "    FAIL atanh(0) != 0 (exact)\n";
				++nrOfFailedTestCases;
			}

			// Test: atanh(tanh(x)) = x (round trip, against tanh(1/2)'s stored value too)
			x = 0.5;
			nrOfFailedTestCases += expect_bits(bits_vs(atanh(tanh(x)), x), wanted_bits<Real>(kIdentityMargin), "atanh(tanh(1/2)) == 1/2", reportTestCases);

			// Test: atanh(1/2), through tanh(atanh(1/2)) == 1/2 and against the wide configuration
			nrOfFailedTestCases += expect_bits(bits_vs(tanh(atanh(x)), x), wanted_bits<Real>(kIdentityMargin), "tanh(atanh(1/2)) == 1/2", reportTestCases);
			nrOfFailedTestCases += expect_vs_wide<Real>([](const auto& v) { return atanh(v); }, 0.5, "atanh(1/2)", reportTestCases);

			return nrOfFailedTestCases;
		}

		// Property fuzzer: sinh and cosh against a wider configuration, the
		// cosh^2-sinh^2==1 identity, tanh==sinh/cosh, sinh/cosh parity, the inverse
		// relations asinh(sinh(x))==x and atanh(tanh(x))==x, and sinh monotonicity.
		template<typename Real, typename FuzzWide>
		int VerifyHyperbolicFuzz(bool reportTestCases, unsigned nrIterations) {
			int nrOfFailedTestCases = 0;
			std::mt19937_64 rng(0xC1A55'1FFEULL);
			std::uniform_real_distribution<double> dist(-3.0, 3.0);
			const Real one(1.0);
			const int wanted   = wanted_bits<Real>(kFunctionMargin);
			const int identity = wanted_bits<Real>(kIdentityMargin);
			for (unsigned i = 0; i < nrIterations; ++i) {
				double dx = dist(rng);
				Real x(dx);
				const std::string at = " at x=" + std::to_string(dx);
				const Real sh = sinh(x), ch = cosh(x), th = tanh(x);
				const FuzzWide wx(dx);
				nrOfFailedTestCases += expect_bits(bits_vs(sh, sinh(wx)), wanted, "sinh vs wide" + at, reportTestCases);
				nrOfFailedTestCases += expect_bits(bits_vs(ch, cosh(wx)), wanted, "cosh vs wide" + at, reportTestCases);
				nrOfFailedTestCases += expect_bits(bits_vs(ch * ch - sh * sh, one), identity, "cosh^2-sinh^2==1" + at, reportTestCases);
				nrOfFailedTestCases += expect_bits(bits_vs(th, sh / ch), identity, "tanh==sinh/cosh" + at, reportTestCases);
				if (sinh(-x) != -sh || cosh(-x) != ch) {
					if (reportTestCases) std::cout << "    FAIL parity" << at << '\n';
					++nrOfFailedTestCases;
				}
				nrOfFailedTestCases += expect_bits(bits_vs(asinh(sh), x), identity, "asinh(sinh(x))==x" + at, reportTestCases);
				nrOfFailedTestCases += expect_bits(bits_vs(atanh(th), x), identity, "atanh(tanh(x))==x" + at, reportTestCases);
				if (!(sh < sinh(x + one))) {
					if (reportTestCases) std::cout << "    FAIL sinh monotonic" << at << '\n';
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

	std::string test_suite  = "ereal mathlib hyperbolic function validation";
	std::string test_tag    = "hyperbolic";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Manual test cases for visual verification
	std::cout << "Manual testing of hyperbolic functions:\n";
	std::cout << "sinh(1) = " << double(sinh(ereal<>(1.0))) << " (expected: " << std::sinh(1.0) << ")\n";
	std::cout << "cosh(1) = " << double(cosh(ereal<>(1.0))) << " (expected: " << std::cosh(1.0) << ")\n";
	std::cout << "tanh(1) = " << double(tanh(ereal<>(1.0))) << " (expected: " << std::tanh(1.0) << ")\n";
	std::cout << "asinh(2) = " << double(asinh(ereal<>(2.0))) << " (expected: " << std::asinh(2.0) << ")\n";
	std::cout << "acosh(2) = " << double(acosh(ereal<>(2.0))) << " (expected: " << std::acosh(2.0) << ")\n";
	std::cout << "atanh(0.5) = " << double(atanh(ereal<>(0.5))) << " (expected: " << std::atanh(0.5) << ")\n";

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	// Phase 5 functions: sinh, cosh, tanh, asinh, acosh, atanh
	test_tag = "sinh";
	nrOfFailedTestCases += ReportTestResult(VerifySinh<ereal<>>(reportTestCases), "sinh(ereal)", test_tag);

	test_tag = "cosh";
	nrOfFailedTestCases += ReportTestResult(VerifyCosh<ereal<>>(reportTestCases), "cosh(ereal)", test_tag);

	test_tag = "tanh";
	nrOfFailedTestCases += ReportTestResult(VerifyTanh<ereal<>>(reportTestCases), "tanh(ereal)", test_tag);

	test_tag = "asinh";
	nrOfFailedTestCases += ReportTestResult(VerifyAsinh<ereal<>>(reportTestCases), "asinh(ereal)", test_tag);

	test_tag = "acosh";
	nrOfFailedTestCases += ReportTestResult(VerifyAcosh<ereal<>>(reportTestCases), "acosh(ereal)", test_tag);

	test_tag = "atanh";
	nrOfFailedTestCases += ReportTestResult(VerifyAtanh<ereal<>>(reportTestCases), "atanh(ereal)", test_tag);

	test_tag = "hyperbolic fuzz";
	// L1 (sanity tier; run by CI's Debug-instrumented ASan/UBSan/coverage jobs) takes
	// only a small fuzz smoke sample. The count scales up for the on-demand stress
	// tiers L2-L4 so QA depth is preserved where runtime is not a concern (#1007).
	unsigned hyperbolicFuzzCount = 15;
#if REGRESSION_LEVEL_4
	hyperbolicFuzzCount = 2000;
#elif REGRESSION_LEVEL_3
	hyperbolicFuzzCount = 500;
#elif REGRESSION_LEVEL_2
	hyperbolicFuzzCount = 100;
#endif
	nrOfFailedTestCases += ReportTestResult(VerifyHyperbolicFuzz<ereal<>, ereal<16>>(reportTestCases, hyperbolicFuzzCount), "hyperbolic property fuzz", test_tag);
#endif

#if REGRESSION_LEVEL_2
	// Extended precision tests at 512 bits (~=154 decimal digits)
	test_tag = "sinh high precision";
	nrOfFailedTestCases += ReportTestResult(VerifySinh<ereal<8>>(reportTestCases), "sinh(ereal<8>)", test_tag);

	test_tag = "cosh high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyCosh<ereal<8>>(reportTestCases), "cosh(ereal<8>)", test_tag);

	test_tag = "tanh high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyTanh<ereal<8>>(reportTestCases), "tanh(ereal<8>)", test_tag);

	test_tag = "asinh high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyAsinh<ereal<8>>(reportTestCases), "asinh(ereal<8>)", test_tag);

	test_tag = "acosh high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyAcosh<ereal<8>>(reportTestCases), "acosh(ereal<8>)", test_tag);

	test_tag = "atanh high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyAtanh<ereal<8>>(reportTestCases), "atanh(ereal<8>)", test_tag);
#endif

#if REGRESSION_LEVEL_3
	// High precision tests at 1024 bits (~=308 decimal digits)
	test_tag = "sinh very high precision";
	nrOfFailedTestCases += ReportTestResult(VerifySinh<ereal<16>>(reportTestCases), "sinh(ereal<16>)", test_tag);

	test_tag = "cosh very high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyCosh<ereal<16>>(reportTestCases), "cosh(ereal<16>)", test_tag);

	test_tag = "tanh very high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyTanh<ereal<16>>(reportTestCases), "tanh(ereal<16>)", test_tag);
#endif

#if REGRESSION_LEVEL_4
	// Extreme precision tests at 1216 bits (~=366 decimal digits, maximum algorithmically valid)
	test_tag = "sinh extreme precision";
	nrOfFailedTestCases += ReportTestResult(VerifySinh<ereal<19>>(reportTestCases), "sinh(ereal<19>)", test_tag);

	test_tag = "cosh extreme precision";
	nrOfFailedTestCases += ReportTestResult(VerifyCosh<ereal<19>>(reportTestCases), "cosh(ereal<19>)", test_tag);
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
