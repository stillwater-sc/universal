// logarithm.cpp: test suite runner for logarithm functions for ereal adaptive-precision
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

	// Precision (#2008, #2006). Every value is checked exactly, against a decimal reference
	// or another ereal, by the dyadic oracle in ereal_precision.hpp: nothing passes through a
	// double, so a result correct to only 53 bits fails. Each check asks for the bits the
	// configuration holds, ereal_bits_of<Real>::value, less kMarginBits.
	//
	// Measured (2026-11, gcc -O2) at ereal<8>/<16>/<19>, bits held 424/848/1007: every check
	// here reaches at least 434/876/1018 bits (a result may carry up to twice its limbs, so
	// it can exceed the bits held). The smallest surplus is 10/28/11 bits, log(e) of an e
	// parsed from its decimal string. A margin of 16 bits keeps every check passing with
	// room for platform differences in the last limb, while asking for 408/832/991 bits:
	// far beyond the 53 a double-only result has.
	constexpr int kMarginBits = 16;

	template<typename Real>
	constexpr int wanted_bits() { return ereal_bits_of<Real>::value - kMarginBits; }

	// The widest double-limb configuration (maxlimbs <= 19), the reference for values that
	// have no stored constant.
	using WideReal = ereal<19, double>;

	// Reference computed with mpmath at 360 digits, printed to 330 significant digits; the
	// argument is the double 0.01 converted exactly (mpf(0.01)), not the decimal.
	//   kLog1p_001 = log1p(mpf(0.01))
	constexpr std::string_view kLog1p_001 =
		"0.009950330853168083054321117066257624171826884982057320887631653238805916412350"
		"12176473800978001516346529341753387268814793094664450336821419048326011273166613"
		"79485523865212490814513769960225581412353835423574199689728855668229158003758467"
		"24830708814066285999340362602697635240153690515932529357434131016022466115692935"
		"82302809433484";

	// Bits measured for a failure message; -1 where the oracle has no value to compare.
	template<typename Real>
	int measured_bits(const Real& got, std::string_view ref) {
		if (got.isnan() || got.isinf() || got.iszero()) return -1;
		return agreed_bits(got, ref);
	}
	template<typename Real, typename Ref>
	int measured_bits(const Real& got, const Ref& ref) {
		if (got.isnan() || got.isinf() || ref.isnan() || ref.isinf() || ref.iszero()) return -1;
		return agreed_bits(got, ref);
	}

	template<typename Real>
	int expect_bits(const Real& got, std::string_view ref, int bits, const std::string& what, bool reportTestCases) {
		if (agrees_to_bits(got, ref, bits)) return 0;
		if (reportTestCases) std::cout << "    FAIL " << what << ": " << measured_bits(got, ref) << " bits, wanted " << bits << '\n';
		return 1;
	}
	template<typename Real, typename Ref>
	int expect_bits(const Real& got, const Ref& ref, int bits, const std::string& what, bool reportTestCases) {
		if (agrees_to_bits(got, ref, bits)) return 0;
		if (reportTestCases) std::cout << "    FAIL " << what << ": " << measured_bits(got, ref) << " bits, wanted " << bits << '\n';
		return 1;
	}

		// Verify log function
		template<typename Real>
		int VerifyLog(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			constexpr int bits = wanted_bits<Real>();

			// Test: log(1) = 0 (mathematically exact)
			Real x(1.0), expected(0.0);
			Real result = log(x);
			if (!check_exact_value(result, expected)) {
				if (reportTestCases) std::cout << "    FAIL log(1) != 0 (exact)\n";
				++nrOfFailedTestCases;
			}

			// log(e) = 1. The argument must be e to the configuration's precision: the double
			// std::exp(1.0) is e only to 53 bits, and so is its logarithm's distance from 1.
			Real e;
			e.parse(std::string(s_e));
			nrOfFailedTestCases += expect_bits(log(e), Real(1.0), bits, "log(e) vs 1", reportTestCases);

			// log(2), log(10): stored references
			nrOfFailedTestCases += expect_bits(log(Real(2.0)), s_ln2, bits, "log(2) vs ln2", reportTestCases);
			nrOfFailedTestCases += expect_bits(log(Real(10.0)), s_ln10, bits, "log(10) vs ln10", reportTestCases);

			return nrOfFailedTestCases;
		}

		// Verify log2 function
		template<typename Real>
		int VerifyLog2(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			constexpr int bits = wanted_bits<Real>();

			// exact results; checked to the configuration, as log2 need not be bit-exact
			nrOfFailedTestCases += expect_bits(log2(Real(2.0)), Real(1.0), bits, "log2(2) vs 1", reportTestCases);
			nrOfFailedTestCases += expect_bits(log2(Real(8.0)), Real(3.0), bits, "log2(8) vs 3", reportTestCases);
			nrOfFailedTestCases += expect_bits(log2(Real(1024.0)), Real(10.0), bits, "log2(1024) vs 10", reportTestCases);

			return nrOfFailedTestCases;
		}

		// Verify log10 function
		template<typename Real>
		int VerifyLog10(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			constexpr int bits = wanted_bits<Real>();

			nrOfFailedTestCases += expect_bits(log10(Real(10.0)), Real(1.0), bits, "log10(10) vs 1", reportTestCases);
			nrOfFailedTestCases += expect_bits(log10(Real(100.0)), Real(2.0), bits, "log10(100) vs 2", reportTestCases);
			nrOfFailedTestCases += expect_bits(log10(Real(1000.0)), Real(3.0), bits, "log10(1000) vs 3", reportTestCases);

			return nrOfFailedTestCases;
		}

		// Verify log1p function
		template<typename Real>
		int VerifyLog1p(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			constexpr int bits = wanted_bits<Real>();

			// Test: log1p(0) = 0 (log(1+0) = log(1) = 0, exact)
			Real x(0.0), expected(0.0);
			Real result = log1p(x);
			if (!check_exact_value(result, expected)) {
				if (reportTestCases) std::cout << "    FAIL log1p(0) != 0 (exact)\n";
				++nrOfFailedTestCases;
			}

			// log1p(0.01): small-x accuracy, mpmath reference. (This used to be checked to 3e-5
			// for a Taylor-series log1p; it now reaches the configuration.)
			nrOfFailedTestCases += expect_bits(log1p(Real(0.01)), kLog1p_001, bits, "log1p(0.01)", reportTestCases);

			// log1p(1) = ln2: stored reference
			nrOfFailedTestCases += expect_bits(log1p(Real(1.0)), s_ln2, bits, "log1p(1) vs ln2", reportTestCases);

			return nrOfFailedTestCases;
		}

		// Verify log/exp roundtrip (identity-based validation)
		template<typename Real>
		int VerifyLogExpRoundtrip(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			constexpr int bits = wanted_bits<Real>();

			// Test: exp(log(x)) = x for various x (mathematical identity), in ereal itself
			double test_values[] = {0.1, 0.5, 1.0, 2.0, 5.0, 10.0};

			for (double val : test_values) {
				Real x(val);
				nrOfFailedTestCases += expect_bits(exp(log(x)), x, bits, "exp(log(" + std::to_string(val) + ")) roundtrip", reportTestCases);
			}

			return nrOfFailedTestCases;
		}

		// Property fuzzer over the positive domain: exp/log inverse, the
		// log(a*b)==log(a)+log(b) product rule, log1p(x)==log(1+x), and monotonicity.
		// Identities are checked in ereal itself; since a self-consistent loss of precision
		// would satisfy them, log(x) is also checked against log at the widest configuration,
		// on every fourth sample (the ereal<19> log dominates an iteration's cost).
		template<typename Real>
		int VerifyLogarithmFuzz(bool reportTestCases, unsigned nrIterations) {
			int nrOfFailedTestCases = 0;
			constexpr int bits = wanted_bits<Real>();
			constexpr bool hasWiderReference = ereal_bits_of<Real>::value < ereal_bits_of<WideReal>::value;
			std::mt19937_64 rng(0xC1A55'1FFEULL);
			std::uniform_real_distribution<double> dist(1.0e-2, 1.0e3);
			Real one(1.0);
			for (unsigned i = 0; i < nrIterations; ++i) {
				double dx = dist(rng);
				Real x(dx);
				const std::string at = " at x=" + std::to_string(dx);
				Real lx = log(x);
				nrOfFailedTestCases += expect_bits(exp(lx), x, bits, "exp(log(x))" + at, reportTestCases);
				if constexpr (hasWiderReference) {
					if (i % 4 == 0) nrOfFailedTestCases += expect_bits(lx, log(WideReal(dx)), bits, "log(x) vs ereal<19>" + at, reportTestCases);
				}
				Real y(dist(rng));
				nrOfFailedTestCases += expect_bits(log(x * y), lx + log(y), bits, "log(a*b)==log(a)+log(b)" + at, reportTestCases);
				if (!(lx < log(x + one))) {
					if (reportTestCases) std::cout << "    FAIL log monotonic at x=" << dx << '\n';
					++nrOfFailedTestCases;
				}
				nrOfFailedTestCases += expect_bits(log1p(x), log(one + x), bits, "log1p(x)==log(1+x)" + at, reportTestCases);
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

	std::string test_suite  = "ereal mathlib logarithm function validation";
	std::string test_tag    = "logarithm";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Manual test cases for visual verification
	std::cout << "Manual testing of logarithm functions:\n";
	std::cout << "log(e) = " << double(log(ereal<>(std::exp(1.0)))) << " (expected: 1.0)\n";
	std::cout << "log2(8.0) = " << double(log2(ereal<>(8.0))) << " (expected: 3.0)\n";
	std::cout << "log10(100.0) = " << double(log10(ereal<>(100.0))) << " (expected: 2.0)\n";
	std::cout << "log1p(0.01) = " << double(log1p(ereal<>(0.01))) << " (expected: " << std::log1p(0.01) << ")\n";

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	// Phase 4a functions: log, log2, log10, log1p
	test_tag = "log";
	nrOfFailedTestCases += ReportTestResult(VerifyLog<ereal<>>(reportTestCases), "log(ereal)", test_tag);

	test_tag = "log2";
	nrOfFailedTestCases += ReportTestResult(VerifyLog2<ereal<>>(reportTestCases), "log2(ereal)", test_tag);

	test_tag = "log10";
	nrOfFailedTestCases += ReportTestResult(VerifyLog10<ereal<>>(reportTestCases), "log10(ereal)", test_tag);

	test_tag = "log1p";
	nrOfFailedTestCases += ReportTestResult(VerifyLog1p<ereal<>>(reportTestCases), "log1p(ereal)", test_tag);

	test_tag = "log/exp roundtrip";
	nrOfFailedTestCases += ReportTestResult(VerifyLogExpRoundtrip<ereal<>>(reportTestCases), "exp(log(x)) roundtrip", test_tag);

	test_tag = "log property fuzz";
	// L1 (sanity tier; run by CI's Debug-instrumented ASan/UBSan/coverage jobs) takes
	// only a small fuzz smoke sample. The count scales up for the on-demand stress
	// tiers L2-L4 so QA depth is preserved where runtime is not a concern (#1007).
	unsigned logFuzzCount = 15;
#if REGRESSION_LEVEL_4
	logFuzzCount = 2000;
#elif REGRESSION_LEVEL_3
	logFuzzCount = 500;
#elif REGRESSION_LEVEL_2
	logFuzzCount = 100;
#endif
	nrOfFailedTestCases += ReportTestResult(VerifyLogarithmFuzz<ereal<>>(reportTestCases, logFuzzCount), "log property fuzz", test_tag);
#endif

#if REGRESSION_LEVEL_2
	// Extended precision tests at 512 bits (~=154 decimal digits)
	test_tag = "log high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyLog<ereal<8>>(reportTestCases), "log(ereal<8>)", test_tag);

	test_tag = "log2 high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyLog2<ereal<8>>(reportTestCases), "log2(ereal<8>)", test_tag);

	test_tag = "log10 high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyLog10<ereal<8>>(reportTestCases), "log10(ereal<8>)", test_tag);

	test_tag = "exp/log roundtrip high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyLogExpRoundtrip<ereal<8>>(reportTestCases), "exp(log(x)) roundtrip ereal<8>", test_tag);
#endif

#if REGRESSION_LEVEL_3
	// High precision tests at 1024 bits (~=308 decimal digits)
	test_tag = "log very high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyLog<ereal<16>>(reportTestCases), "log(ereal<16>)", test_tag);

	test_tag = "exp/log roundtrip very high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyLogExpRoundtrip<ereal<16>>(reportTestCases), "exp(log(x)) roundtrip ereal<16>", test_tag);
#endif

#if REGRESSION_LEVEL_4
	// Maximum precision tests at ereal<19> (~=303 decimal digits, maximum algorithmically valid)
	test_tag = "log maximum precision";
	nrOfFailedTestCases += ReportTestResult(VerifyLog<ereal<19>>(reportTestCases), "log(ereal<19>)", test_tag);

	test_tag = "exp/log roundtrip maximum precision";
	nrOfFailedTestCases += ReportTestResult(VerifyLogExpRoundtrip<ereal<19>>(reportTestCases), "exp(log(x)) roundtrip ereal<19>", test_tag);
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
