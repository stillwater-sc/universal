// exponent.cpp: test suite runner for exponential functions for ereal adaptive-precision
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
	// here reaches at least 472/896/1020 bits (a result may carry up to twice its limbs, so
	// it can exceed the bits held). The smallest surplus is 13 bits, the fuzz identities at
	// ereal<19>. A margin of 16 bits keeps every check passing with room for platform
	// differences in the last limb, while asking for 408/832/991 bits: far beyond the 53 a
	// double-only result has (VerifyDoubleTruncationIsVisible shows the check can tell).
	constexpr int kMarginBits = 16;

	template<typename Real>
	constexpr int wanted_bits() { return ereal_bits_of<Real>::value - kMarginBits; }

	// The widest double-limb configuration (maxlimbs <= 19), the reference for values that
	// have no stored constant.
	using WideReal = ereal<19, double>;

	// References computed with mpmath at 360 digits, printed to 330 significant digits.
	// The arguments are the doubles the tests pass, converted exactly (mpf(0.01) is the
	// double 0.01, not the decimal).
	//   kExp2      = exp(2)
	//   kExpm1_001 = expm1(mpf(0.01))
	constexpr std::string_view kExp2 =
		"7.389056098930650227230427460575007813180315570551847324087127822522573796079057"
		"76338431248507912179477375316126547886612388460369278127337447839221339807777490"
		"01228956074107537023913309475506820865818202696478682084042209822552348757424625"
		"41414679928129331888070763301019337899740729986960095303307515320818823684694793"
		"02991355877";
	constexpr std::string_view kExpm1_001 =
		"0.010050167084168057752424385313484369731243899318130831789141650500379617021528"
		"21323393832472698424349332296688425646999811003182182612935750644675456895975297"
		"26357692415538274527865378764919862726361409577016896886580900822581782495827000"
		"03611364268279638639853872063450641671901548477176557239626404924342213029117041"
		"3614336416360";

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

		// Verify exp function
		template<typename Real>
		int VerifyExp(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			constexpr int bits = wanted_bits<Real>();

			// Test: exp(0) = 1 (mathematically exact)
			Real x(0.0), expected(1.0);
			Real result = exp(x);
			if (!check_exact_value(result, expected)) {
				if (reportTestCases) std::cout << "    FAIL exp(0) != 1 (exact)\n";
				++nrOfFailedTestCases;
			}

			// exp(1) = e, exp(-1) = 1/e: stored references
			nrOfFailedTestCases += expect_bits(exp(Real(1.0)), s_e, bits, "exp(1) vs e", reportTestCases);
			nrOfFailedTestCases += expect_bits(exp(Real(-1.0)), s_inv_e, bits, "exp(-1) vs 1/e", reportTestCases);
			// exp(2) = e^2: mpmath reference
			nrOfFailedTestCases += expect_bits(exp(Real(2.0)), kExp2, bits, "exp(2) vs e^2", reportTestCases);

			return nrOfFailedTestCases;
		}

		// Sensitivity of the check itself: exp(1) truncated to a double must FAIL the check the
		// real result passes. Without this, a check that could not see past 53 bits would pass
		// the double-only results it exists to catch (#2008).
		template<typename Real>
		int VerifyDoubleTruncationIsVisible(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			constexpr int bits = wanted_bits<Real>();
			Real full = exp(Real(1.0));
			Real truncated = Real(double(full));
			if (!agrees_to_bits(full, s_e, bits)) {
				if (reportTestCases) std::cout << "    FAIL exp(1) does not reach " << bits << " bits\n";
				++nrOfFailedTestCases;
			}
			if (agrees_to_bits(truncated, s_e, bits)) {
				if (reportTestCases) std::cout << "    FAIL a 53-bit exp(1) passes the " << bits << "-bit check: the check is blind\n";
				++nrOfFailedTestCases;
			}
			// and the truncated value is a correct double: the check fails it for precision, not for being wrong
			if (!agrees_to_bits(truncated, s_e, 50)) {
				if (reportTestCases) std::cout << "    FAIL double(exp(1)) is not e to 50 bits\n";
				++nrOfFailedTestCases;
			}
			return nrOfFailedTestCases;
		}

		// Verify exp2 function
		template<typename Real>
		int VerifyExp2(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			constexpr int bits = wanted_bits<Real>();

			// exact powers of two; exp2 goes through exp(x ln2), so the result need not be
			// bit-exact, only correct to the configuration
			nrOfFailedTestCases += expect_bits(exp2(Real(3.0)), Real(8.0), bits, "exp2(3) vs 8", reportTestCases);
			nrOfFailedTestCases += expect_bits(exp2(Real(10.0)), Real(1024.0), bits, "exp2(10) vs 1024", reportTestCases);
			nrOfFailedTestCases += expect_bits(exp2(Real(-1.0)), Real(0.5), bits, "exp2(-1) vs 0.5", reportTestCases);

			return nrOfFailedTestCases;
		}

		// Verify exp10 function
		template<typename Real>
		int VerifyExp10(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			constexpr int bits = wanted_bits<Real>();

			nrOfFailedTestCases += expect_bits(exp10(Real(2.0)), Real(100.0), bits, "exp10(2) vs 100", reportTestCases);
			nrOfFailedTestCases += expect_bits(exp10(Real(3.0)), Real(1000.0), bits, "exp10(3) vs 1000", reportTestCases);
			// 0.1 has no finite binary value: compare against the decimal itself, not double(0.1)
			nrOfFailedTestCases += expect_bits(exp10(Real(-1.0)), std::string_view("0.1"), bits, "exp10(-1) vs 0.1", reportTestCases);

			return nrOfFailedTestCases;
		}

		// Verify expm1 function
		template<typename Real>
		int VerifyExpm1(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			constexpr int bits = wanted_bits<Real>();

			// Test: expm1(0) = 0 (e^0 - 1 = 1 - 1 = 0, mathematically exact)
			Real x(0.0), expected(0.0);
			Real result = expm1(x);
			if (!check_exact_value(result, expected)) {
				if (reportTestCases) std::cout << "    FAIL expm1(0) != 0 (exact)\n";
				++nrOfFailedTestCases;
			}

			// expm1(0.01): small-x accuracy, mpmath reference
			nrOfFailedTestCases += expect_bits(expm1(Real(0.01)), kExpm1_001, bits, "expm1(0.01)", reportTestCases);

			// expm1(1) = e - 1: the stored e with its leading digit lowered by one
			const std::string e_minus_1 = "1" + std::string(s_e.substr(1));
			nrOfFailedTestCases += expect_bits(expm1(Real(1.0)), std::string_view(e_minus_1), bits, "expm1(1) vs e-1", reportTestCases);

			return nrOfFailedTestCases;
		}

		// Verify exp/log roundtrip
		template<typename Real>
		int VerifyExpLogRoundtrip(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			constexpr int bits = wanted_bits<Real>();

			// Test: log(exp(x)) ~= x for various x, in ereal itself
			double test_values[] = {0.1, 0.5, 1.0, 2.0, 3.0};

			for (double val : test_values) {
				Real x(val);
				nrOfFailedTestCases += expect_bits(log(exp(x)), x, bits, "log(exp(" + std::to_string(val) + ")) roundtrip", reportTestCases);
			}

			return nrOfFailedTestCases;
		}

		// Property fuzzer: exp/log inverse, the exp(a+b)==exp(a)*exp(b) addition
		// theorem, exp(-x)==1/exp(x), expm1(x)==exp(x)-1, and monotonicity. Identities are
		// checked in ereal itself; since a self-consistent loss of precision would satisfy
		// them, exp(x) is also checked against exp at the widest configuration, on every
		// fourth sample: the ereal<19> exp dominates the cost of an iteration.
		template<typename Real>
		int VerifyExponentFuzz(bool reportTestCases, unsigned nrIterations) {
			int nrOfFailedTestCases = 0;
			constexpr int bits = wanted_bits<Real>();
			constexpr bool hasWiderReference = ereal_bits_of<Real>::value < ereal_bits_of<WideReal>::value;
			std::mt19937_64 rng(0xC1A55'1FFEULL);
			std::uniform_real_distribution<double> dist(-5.0, 5.0);
			Real one(1.0);
			for (unsigned i = 0; i < nrIterations; ++i) {
				double dx = dist(rng);
				Real x(dx);
				const std::string at = " at x=" + std::to_string(dx);
				Real ex = exp(x);
				nrOfFailedTestCases += expect_bits(log(ex), x, bits, "log(exp(x))" + at, reportTestCases);
				nrOfFailedTestCases += expect_bits(exp(-x), one / ex, bits, "exp(-x)==1/exp(x)" + at, reportTestCases);
				nrOfFailedTestCases += expect_bits(expm1(x), ex - one, bits, "expm1==exp-1" + at, reportTestCases);
				if constexpr (hasWiderReference) {
					if (i % 4 == 0) nrOfFailedTestCases += expect_bits(ex, exp(WideReal(dx)), bits, "exp(x) vs ereal<19>" + at, reportTestCases);
				}
				Real y(dist(rng));
				nrOfFailedTestCases += expect_bits(exp(x + y), ex * exp(y), bits, "exp(a+b)==exp(a)exp(b)" + at, reportTestCases);
				if (!(ex < exp(x + one))) {
					if (reportTestCases) std::cout << "    FAIL exp monotonic at x=" << dx << '\n';
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

	std::string test_suite  = "ereal mathlib exponential function validation";
	std::string test_tag    = "exponential";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Manual test cases for visual verification
	std::cout << "Manual testing of exponential functions:\n";
	std::cout << std::setprecision(34);
	std::cout << "exp(1.0) = " << exp(ereal<>(1.0)) << " (reference: " << std::exp(1.0) << ")\n";
	std::cout << "exp2(3.0) = " << exp2(ereal<>(3.0)) << " (reference: " << std::exp2(3.0) << ")\n";
	std::cout << "exp10(2.0) = " << exp10(ereal<>(2.0)) << " (reference: " << std::pow(10.0, 2.0) << ")\n";
	std::cout << "expm1(0.01) = " << expm1(ereal<>(0.01)) << " (reference: " << std::expm1(0.01) << ")\n";

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	// Phase 4a functions: exp, exp2, exp10, expm1
	test_tag = "exp";
	nrOfFailedTestCases += ReportTestResult(VerifyExp<ereal<>>(reportTestCases), "exp(ereal)", test_tag);

	test_tag = "exp precision check sensitivity";
	nrOfFailedTestCases += ReportTestResult(VerifyDoubleTruncationIsVisible<ereal<>>(reportTestCases), "double-truncated exp(1) is caught", test_tag);

	test_tag = "exp2";
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<ereal<>>(reportTestCases), "exp2(ereal)", test_tag);

	test_tag = "exp10";
	nrOfFailedTestCases += ReportTestResult(VerifyExp10<ereal<>>(reportTestCases), "exp10(ereal)", test_tag);

	test_tag = "expm1";
	nrOfFailedTestCases += ReportTestResult(VerifyExpm1<ereal<>>(reportTestCases), "expm1(ereal)", test_tag);

	test_tag = "exp/log roundtrip";
	nrOfFailedTestCases += ReportTestResult(VerifyExpLogRoundtrip<ereal<>>(reportTestCases), "log(exp(x)) roundtrip", test_tag);

	test_tag = "exp/log fuzz";
	// L1 (sanity tier; run by CI's Debug-instrumented ASan/UBSan/coverage jobs) takes
	// only a small fuzz smoke sample. The count scales up for the on-demand stress
	// tiers L2-L4 so QA depth is preserved where runtime is not a concern (#1007).
	unsigned expFuzzCount = 20;
#if REGRESSION_LEVEL_4
	expFuzzCount = 2000;
#elif REGRESSION_LEVEL_3
	expFuzzCount = 500;
#elif REGRESSION_LEVEL_2
	expFuzzCount = 100;
#endif
	nrOfFailedTestCases += ReportTestResult(VerifyExponentFuzz<ereal<>>(reportTestCases, expFuzzCount), "exp/log property fuzz", test_tag);
#endif

#if REGRESSION_LEVEL_2
	// Extended precision tests at 512 bits (~=154 decimal digits)
	test_tag = "exp high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyExp<ereal<8>>(reportTestCases), "exp(ereal<8>)", test_tag);

	test_tag = "exp2 high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyExp2<ereal<8>>(reportTestCases), "exp2(ereal<8>)", test_tag);

	test_tag = "exp10 high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyExp10<ereal<8>>(reportTestCases), "exp10(ereal<8>)", test_tag);

	test_tag = "exp/log roundtrip high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyExpLogRoundtrip<ereal<8>>(reportTestCases), "log(exp(x)) roundtrip ereal<8>", test_tag);
#endif

#if REGRESSION_LEVEL_3
	// High precision tests at 1024 bits (~=308 decimal digits)
	test_tag = "exp very high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyExp<ereal<16>>(reportTestCases), "exp(ereal<16>)", test_tag);

	test_tag = "exp/log roundtrip very high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyExpLogRoundtrip<ereal<16>>(reportTestCases), "log(exp(x)) roundtrip ereal<16>", test_tag);
#endif

#if REGRESSION_LEVEL_4
	// Maximum precision tests at ereal<19> (~=303 decimal digits, maximum algorithmically valid)
	test_tag = "exp maximum precision";
	nrOfFailedTestCases += ReportTestResult(VerifyExp<ereal<19>>(reportTestCases), "exp(ereal<19>)", test_tag);

	test_tag = "exp/log roundtrip maximum precision";
	nrOfFailedTestCases += ReportTestResult(VerifyExpLogRoundtrip<ereal<19>>(reportTestCases), "log(exp(x)) roundtrip ereal<19>", test_tag);
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
