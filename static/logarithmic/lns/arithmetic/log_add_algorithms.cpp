// log_add_algorithms.cpp: cross-validation of configurable lns add/sub algorithms
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Phase A and B regression for issue #777: validates that the alternate
// add/sub algorithms (DirectEvaluationAddSub from Phase A, LookupAddSub from
// Phase B / issue #780) agree with the historical DoubleTripAddSub baseline
// and with each other within a small ULP tolerance across representative
// lns configurations.
//
// Direct invocation of policy classes lets us cross-validate them against the
// same lns instantiation without specializing the traits.

#include <universal/utility/directives.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/verification/lns_test_suite.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_reporters.hpp>

namespace sw { namespace universal {

	// Shared helper for the corner-case / Tier 1 suites: compares an algorithm
	// output against an expected value, with explicit NaN-mismatch detection
	// (one-sided NaN fails) and a scaled absolute-difference tolerance.
	template<typename LnsType>
	void check_case(const char* algName, const char* name,
	                double expected, const LnsType& got,
	                double absTolBase, double tolScale,
	                bool reportTestCases, int& failed) {
		double absTol = absTolBase * tolScale;
		double g = double(got);
		bool g_nan = (g != g);
		bool e_nan = (expected != expected);
		if (g_nan && e_nan) return;
		if (g_nan != e_nan) {
			++failed;
			if (reportTestCases) {
				std::cout << "FAIL [" << algName << "] " << name
				          << " NaN mismatch  expected=" << expected
				          << "  got=" << g << '\n';
			}
			return;
		}
		double diff = g - expected;
		if (diff < 0.0) diff = -diff;
		if (diff > absTol) {
			++failed;
			if (reportTestCases) {
				std::cout << "FAIL [" << algName << "] " << name
				          << "  expected=" << expected
				          << "  got=" << g << "  diff=" << diff << '\n';
			}
		}
	}

	// Cross-validate two add/sub policies against each other in the value
	// domain. Both encode through the same lns instantiation, so the upper
	// bound on disagreement is dominated by transcendental rounding plus the
	// encode rounding -- in practice a few percent relative error is generous
	// for small lns sizes where the encoding step itself is coarse.
	template<typename LnsType, typename AlgA, typename AlgB>
	int VerifyAddAlgorithmsAgree(bool reportTestCases, double relTol = 0.05) {
		constexpr unsigned nbits = LnsType::nbits;
		static_assert(nbits < 64, "exhaustive sweep requires nbits < 64 to avoid shift UB");
		constexpr size_t NR_ENCODINGS = (1ull << nbits);

		int nrOfFailedTestCases = 0;
		int reported = 0;

		for (size_t i = 0; i < NR_ENCODINGS; ++i) {
			LnsType a; a.setbits(i);
			if (a.isnan()) continue;
			for (size_t j = 0; j < NR_ENCODINGS; ++j) {
				LnsType b; b.setbits(j);
				if (b.isnan()) continue;

				LnsType cA = a; AlgA::add_assign(cA, b);
				LnsType cB = a; AlgB::add_assign(cB, b);

				if (cA == cB) continue;
				if (cA.isnan() && cB.isnan()) continue;

				double vA = double(cA);
				double vB = double(cB);
				double diff = vA - vB;
				if (diff < 0.0) diff = -diff;
				double mag = (vA < 0.0 ? -vA : vA);
				if (mag < 1.0) mag = 1.0;
				if (diff / mag <= relTol) continue;

				++nrOfFailedTestCases;
				if (reportTestCases && reported < 8) {
					std::cout << "MISMATCH a=" << double(a) << " b=" << double(b)
					          << " A=" << vA << " B=" << vB
					          << " relErr=" << (diff / mag) << '\n';
					++reported;
				}
				if (nrOfFailedTestCases > 24) return nrOfFailedTestCases;
			}
		}
		return nrOfFailedTestCases;
	}

	template<typename LnsType, typename AlgA, typename AlgB>
	int VerifySubAlgorithmsAgree(bool reportTestCases, double relTol = 0.05) {
		constexpr unsigned nbits = LnsType::nbits;
		static_assert(nbits < 64, "exhaustive sweep requires nbits < 64 to avoid shift UB");
		constexpr size_t NR_ENCODINGS = (1ull << nbits);

		int nrOfFailedTestCases = 0;
		int reported = 0;

		for (size_t i = 0; i < NR_ENCODINGS; ++i) {
			LnsType a; a.setbits(i);
			if (a.isnan()) continue;
			for (size_t j = 0; j < NR_ENCODINGS; ++j) {
				LnsType b; b.setbits(j);
				if (b.isnan()) continue;

				LnsType cA = a; AlgA::sub_assign(cA, b);
				LnsType cB = a; AlgB::sub_assign(cB, b);

				if (cA == cB) continue;
				if (cA.isnan() && cB.isnan()) continue;

				double vA = double(cA);
				double vB = double(cB);
				double diff = vA - vB;
				if (diff < 0.0) diff = -diff;
				double mag = (vA < 0.0 ? -vA : vA);
				if (mag < 1.0) mag = 1.0;
				if (diff / mag <= relTol) continue;

				++nrOfFailedTestCases;
				if (reportTestCases && reported < 8) {
					std::cout << "MISMATCH a=" << double(a) << " b=" << double(b)
					          << " A=" << vA << " B=" << vB
					          << " relErr=" << (diff / mag) << '\n';
					++reported;
				}
				if (nrOfFailedTestCases > 24) return nrOfFailedTestCases;
			}
		}
		return nrOfFailedTestCases;
	}

	// Convenience wrappers for the original Phase A pairing (Direct vs DoubleTrip)
	template<typename LnsType>
	int VerifyAddAlgorithmAgreement(bool reportTestCases, double relTol = 0.05) {
		return VerifyAddAlgorithmsAgree<LnsType,
		                                DirectEvaluationAddSub<LnsType>,
		                                DoubleTripAddSub<LnsType>>(reportTestCases, relTol);
	}
	template<typename LnsType>
	int VerifySubAlgorithmAgreement(bool reportTestCases, double relTol = 0.05) {
		return VerifySubAlgorithmsAgree<LnsType,
		                                DirectEvaluationAddSub<LnsType>,
		                                DoubleTripAddSub<LnsType>>(reportTestCases, relTol);
	}

	// Spot-check algorithm corner cases that the exhaustive sweep above doesn't
	// exercise (sweeps are restricted to small lns sizes for runtime reasons).
	// Use a high-precision LnsType so encoding error doesn't confound the
	// algorithm-correctness check. Parameterized on Alg so Phase B's
	// LookupAddSub can be exercised with the same test bench.
	// tolScale lets the caller widen tolerances proportionally for less-accurate
	// algorithms (e.g., LookupAddSub at default IndexBits has ~100x the
	// per-operation error of DirectEvaluationAddSub for mixed-sign cases).
	template<typename LnsType, typename Alg>
	int VerifyAlgorithmCornerCases(bool reportTestCases, const char* algName, double tolScale = 1.0) {
		int failed = 0;
		auto check = [&](const char* name, double expected, const LnsType& got, double absTolBase) {
			check_case<LnsType>(algName, name, expected, got, absTolBase, tolScale, reportTestCases, failed);
		};

		// a + (-a) -> 0 (exact cancellation)
		{
			LnsType a(2.5);
			LnsType minus_a = -a;
			LnsType r = a; Alg::add_assign(r, minus_a);
			check("2.5 + (-2.5)", 0.0, r, 1e-12);
		}
		// a - a -> 0 (exact cancellation)
		{
			LnsType a(3.75);
			LnsType r = a; Alg::sub_assign(r, a);
			check("3.75 - 3.75", 0.0, r, 1e-12);
		}
		// 1 + 1 -> 2
		{
			LnsType a(1.0);
			LnsType b(1.0);
			LnsType r = a; Alg::add_assign(r, b);
			check("1 + 1", 2.0, r, 1e-6);
		}
		// 1 + 0 -> 1 (zero short-circuit)
		{
			LnsType a(1.0);
			LnsType b(0.0);
			LnsType r = a; Alg::add_assign(r, b);
			check("1 + 0", 1.0, r, 1e-12);
		}
		// 0 + 2 -> 2 (zero short-circuit on lhs)
		{
			LnsType a(0.0);
			LnsType b(2.0);
			LnsType r = a; Alg::add_assign(r, b);
			check("0 + 2", 2.0, r, 1e-6);
		}
		// 2 + (-1) -> 1 (mixed sign, magnitude subtraction)
		{
			LnsType a(2.0);
			LnsType b(-1.0);
			LnsType r = a; Alg::add_assign(r, b);
			check("2 + (-1)", 1.0, r, 1e-6);
		}
		// (-3) + 5 -> 2
		{
			LnsType a(-3.0);
			LnsType b(5.0);
			LnsType r = a; Alg::add_assign(r, b);
			check("-3 + 5", 2.0, r, 1e-6);
		}
		// large dynamic range: 4 + tiny -> ~4 (Lb - La very negative => sb_add ~ 0)
		{
			LnsType a(4.0);
			LnsType tiny(0.001953125);  // 2^-9, well within lns dynamic range
			LnsType r = a; Alg::add_assign(r, tiny);
			// expected ~ 4.001953125; allow lns encoding error
			double expected = 4.001953125;
			check("4 + 2^-9", expected, r, 0.05);
		}

		return failed;
	}

	// Backward-compat wrapper for the original Phase A direct-eval corner suite
	template<typename LnsType>
	int VerifyDirectEvalCornerCases(bool reportTestCases) {
		return VerifyAlgorithmCornerCases<LnsType, DirectEvaluationAddSub<LnsType>>(
		    reportTestCases, "DirectEval");
	}

	// Tier 1 catastrophic-cancellation cases: stress the regions where log-add
	// algorithms are most likely to diverge -- exact cancellation, near-zero
	// 1 + epsilon, and large dynamic range in mixed-sign arithmetic. Run these
	// against any algorithm; the Lookup policy's special-cased near-d=0 fallback
	// is what makes it agree with DirectEval here within tight tolerance.
	template<typename LnsType, typename Alg>
	int VerifyTier1CancellationCases(bool reportTestCases, const char* algName, double tolScale = 1.0) {
		int failed = 0;
		auto check = [&](const char* name, double expected, const LnsType& got, double absTol) {
			check_case<LnsType>(algName, name, expected, got, absTol, tolScale, reportTestCases, failed);
		};

		// Exact cancellation a + (-a): trivial cases
		{
			LnsType a(7.0);
			LnsType b(-7.0);
			LnsType r = a; Alg::add_assign(r, b);
			check("Tier1: 7 + (-7) exact cancel", 0.0, r, 1e-12);
		}
		// 1 + epsilon -> 1 + epsilon (preserves the small addend)
		// epsilon = 2^-6 = 1/64 = 0.015625 -- representable in lns precision >= 6
		{
			LnsType one(1.0);
			LnsType eps(0.015625);
			LnsType r = one; Alg::add_assign(r, eps);
			check("Tier1: 1 + 2^-6", 1.015625, r, 0.01);
		}
		// 1 - (1 - epsilon) = epsilon: classic catastrophic cancellation
		// Use lns-representable values: 1.0 and 0.984375 (= 63/64 = 1 - 2^-6)
		{
			LnsType a(1.0);
			LnsType b(0.984375);
			LnsType r = a; Alg::sub_assign(r, b);
			// Expected ~0.015625; lns encoding rounds. Allow generous tol.
			check("Tier1: 1 - 0.984375 (cancel)", 0.015625, r, 0.01);
		}
		// Large dynamic range mixed-sign: huge - small_huge = small
		// 1024 + (-1023) = 1 (both operands are powers-of-2-ish)
		{
			LnsType a(1024.0);
			LnsType b(-1023.0);
			LnsType r = a; Alg::add_assign(r, b);
			check("Tier1: 1024 + (-1023)", 1.0, r, 0.05);
		}
		// Large dynamic range same-sign: large + tiny -> large
		// 2^20 + 2^-10 -> 2^20 (the small addend is below useful precision)
		{
			LnsType a(1048576.0);   // 2^20
			LnsType b(0.0009765625); // 2^-10
			LnsType r = a; Alg::add_assign(r, b);
			// In high-precision lns, the small addend should still register.
			// Expected ~= 1048576.0009765625
			check("Tier1: 2^20 + 2^-10 (large dyn range)", 1048576.0009765625, r, 1.0);
		}
		// Mixed signs near equality: 1.0 + (-0.999) = 0.001 (near-zero magnitude
		// subtraction, the sb_sub catastrophic-cancellation regime)
		// 0.999 isn't lns-exact; pick 0.9990234375 = 1023/1024 = 1 - 2^-10.
		{
			LnsType a(1.0);
			LnsType b(-0.9990234375);  // = -(1 - 2^-10)
			LnsType r = a; Alg::add_assign(r, b);
			check("Tier1: 1 + (-(1 - 2^-10))", 0.0009765625, r, 0.001);
		}

		return failed;
	}

} }  // namespace sw::universal

// Regression testing guards
#define MANUAL_TESTING 0
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

	std::string test_suite  = "lns add/sub algorithm cross-validation (issue #777 Phase A/B/C)";
	std::string test_tag    = "log_add_algorithms";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// manual exhaustive test
	using LNS4_2_sat = lns<4, 2, std::uint8_t>;
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<LNS4_2_sat>(reportTestCases), "lns<4,2,uint8_t>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#else

#if REGRESSION_LEVEL_1
	using LNS5_2_sat = lns<5, 2, std::uint8_t>;
	using LNS6_2_sat = lns<6, 2, std::uint8_t>;
	using LNS8_2_sat = lns<8, 2, std::uint8_t>;
	using LNS_HiPrec = lns<32, 24, std::uint32_t>;  // ~7 decimal digits in log domain

	// Phase A cross-validation: DirectEvaluation vs DoubleTrip baseline
	// Allow ~5% relative value-domain tolerance: the two paths use different
	// transcendental routines and round at different points; for small lns
	// sizes the encoding step itself is coarser than the algorithm gap.
	nrOfFailedTestCases += ReportTestResult(VerifyAddAlgorithmAgreement<LNS5_2_sat>(reportTestCases, 0.05),
	                                        "lns<5,2,uint8_t> add: Direct vs DoubleTrip", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubAlgorithmAgreement<LNS5_2_sat>(reportTestCases, 0.05),
	                                        "lns<5,2,uint8_t> sub: Direct vs DoubleTrip", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyAddAlgorithmAgreement<LNS6_2_sat>(reportTestCases, 0.05),
	                                        "lns<6,2,uint8_t> add: Direct vs DoubleTrip", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyAddAlgorithmAgreement<LNS8_2_sat>(reportTestCases, 0.05),
	                                        "lns<8,2,uint8_t> add: Direct vs DoubleTrip", test_tag);
	// Phase B cross-validation: LookupAddSub vs DirectEvaluation (the new
	// table-based algorithm against the constexpr_math reference)
	nrOfFailedTestCases += ReportTestResult(
	    (VerifyAddAlgorithmsAgree<LNS5_2_sat,
	                              LookupAddSub<LNS5_2_sat>,
	                              DirectEvaluationAddSub<LNS5_2_sat>>(reportTestCases, 0.05)),
	    "lns<5,2,uint8_t> add: Lookup vs Direct", test_tag);
	nrOfFailedTestCases += ReportTestResult(
	    (VerifyAddAlgorithmsAgree<LNS6_2_sat,
	                              LookupAddSub<LNS6_2_sat>,
	                              DirectEvaluationAddSub<LNS6_2_sat>>(reportTestCases, 0.05)),
	    "lns<6,2,uint8_t> add: Lookup vs Direct", test_tag);
	nrOfFailedTestCases += ReportTestResult(
	    (VerifyAddAlgorithmsAgree<LNS8_2_sat, 
                                  LookupAddSub<LNS8_2_sat>, 
                                  DirectEvaluationAddSub<LNS8_2_sat>>(reportTestCases, 0.05)),
	    "lns<8,2,uint8_t> add: Lookup vs Direct", test_tag);

	// Corner cases: high-precision lns so encoding rounding doesn't drown out
	// algorithm-correctness signal. Run for both Direct and Lookup variants.
	nrOfFailedTestCases += ReportTestResult(VerifyDirectEvalCornerCases<LNS_HiPrec>(reportTestCases),
	                                        "lns<32,24,uint32_t> Direct corner cases", test_tag);
	// Lookup at default IndexBits has linear-interpolation error ~ step^2
	// in sb_sub, dominated near d ~ -1 (the "2 + (-1)" / "(-3) + 5" cases).
	// Scale tolerance up 200x to match -- this still catches algorithm bugs.
	nrOfFailedTestCases += ReportTestResult(
	    (VerifyAlgorithmCornerCases<LNS_HiPrec, LookupAddSub<LNS_HiPrec>>(reportTestCases, "Lookup", 200.0)),
	    "lns<32,24,uint32_t> Lookup corner cases", test_tag);

	// Tier 1 catastrophic-cancellation suite: stresses sb_sub near d=0,
	// large dynamic range, and exact-cancellation paths. Both algorithms.
	nrOfFailedTestCases += ReportTestResult(
	    (VerifyTier1CancellationCases<LNS_HiPrec,
	                                  DirectEvaluationAddSub<LNS_HiPrec>>(reportTestCases, "DirectEval")),
	    "lns<32,24,uint32_t> Direct Tier 1 cancellation", test_tag);
	nrOfFailedTestCases += ReportTestResult(
	    (VerifyTier1CancellationCases<LNS_HiPrec,
	                                  LookupAddSub<LNS_HiPrec>>(reportTestCases, "Lookup")),
	    "lns<32,24,uint32_t> Lookup Tier 1 cancellation", test_tag);

	// Phase C cross-validation: Polynomial vs Direct (~6e-5 absolute error)
	nrOfFailedTestCases += ReportTestResult(
	    (VerifyAddAlgorithmsAgree<LNS5_2_sat,
	                              PolynomialAddSub<LNS5_2_sat>,
	                              DirectEvaluationAddSub<LNS5_2_sat>>(reportTestCases, 0.05)),
	    "lns<5,2,uint8_t> add: Polynomial vs Direct", test_tag);
	nrOfFailedTestCases += ReportTestResult(
	    (VerifySubAlgorithmsAgree<LNS6_2_sat,
	                              PolynomialAddSub<LNS6_2_sat>,
	                              DirectEvaluationAddSub<LNS6_2_sat>>(reportTestCases, 0.05)),
	    "lns<6,2,uint8_t> sub: Polynomial vs Direct", test_tag);
	nrOfFailedTestCases += ReportTestResult(
	    (VerifySubAlgorithmsAgree<LNS8_2_sat, 
                                  PolynomialAddSub<LNS8_2_sat>, 
                                  DirectEvaluationAddSub<LNS8_2_sat>>(reportTestCases, 0.05)),
	    "lns<8,2,uint8_t> sub: Polynomial vs Direct", test_tag);

	// Phase C cross-validation: ArnoldBailey vs Direct (~10% relative error;
	// the piecewise-linear approximation is the coarsest of the family).
	// The sub variant exercises ArnoldBailey's mixed-sign / sb_sub path,
	// which uses the same piecewise-linear knots plus a direct-eval fallback
	// in the cancellation regime u > 0.5.
	nrOfFailedTestCases += ReportTestResult(
	    (VerifyAddAlgorithmsAgree<LNS5_2_sat,
	                              ArnoldBaileyAddSub<LNS5_2_sat>,
	                              DirectEvaluationAddSub<LNS5_2_sat>>(reportTestCases, 0.10)),
	    "lns<5,2,uint8_t> add: ArnoldBailey vs Direct", test_tag);
	nrOfFailedTestCases += ReportTestResult(
	    (VerifySubAlgorithmsAgree<LNS6_2_sat,
	                              ArnoldBaileyAddSub<LNS6_2_sat>,
	                              DirectEvaluationAddSub<LNS6_2_sat>>(reportTestCases, 0.20)),
	    "lns<6,2,uint8_t> sub: ArnoldBailey vs Direct", test_tag);
	nrOfFailedTestCases += ReportTestResult(
	    (VerifySubAlgorithmsAgree<LNS8_2_sat, 
                                  ArnoldBaileyAddSub<LNS8_2_sat>, 
                                  DirectEvaluationAddSub<LNS8_2_sat>>(reportTestCases, 0.50)),
	    "lns<8,2,uint8_t> sub: ArnoldBailey vs Direct", test_tag);

	// Corner cases for the new policies. Polynomial (degree-7) has
	// theoretical sb_add truncation error ~5.6e-6 over u in (0, 1], but the
	// surrounding exp2(Lresult) amplifies into ~2.5e-5 worst-case in the
	// value domain (observed empirically). tolScale=50 (5e-5 absolute) sits
	// just above the envelope -- tight enough to catch a regression that
	// triples the error, loose enough to absorb cm::log2/cm::exp2 jitter.
	nrOfFailedTestCases += ReportTestResult(
	    (VerifyAlgorithmCornerCases<LNS_HiPrec, PolynomialAddSub<LNS_HiPrec>>(reportTestCases, "Polynomial", 50.0)),
	    "lns<32,24,uint32_t> Polynomial corner cases", test_tag);
	// ArnoldBailey corner cases: every case in the suite happens to land
	// on a piecewise-linear knot (exact), in the cancellation-regime
	// direct-eval fallback (exact), or in a zero short-circuit. None
	// stresses the secant-interpolation region, so the actual error is
	// indistinguishable from DirectEvaluation. The Tier 1 suite below
	// exercises the secant region. tolScale=10 here keeps the corner-case
	// suite a strict regression check.
	nrOfFailedTestCases += ReportTestResult(
	    (VerifyAlgorithmCornerCases<LNS_HiPrec, ArnoldBaileyAddSub<LNS_HiPrec>>(reportTestCases, "ArnoldBailey", 10.0)),
	    "lns<32,24,uint32_t> ArnoldBailey corner cases", test_tag);

	// Tier 1 cancellation for both new algorithms.
	nrOfFailedTestCases += ReportTestResult(
	    (VerifyTier1CancellationCases<LNS_HiPrec,
	                                  PolynomialAddSub<LNS_HiPrec>>(reportTestCases, "Polynomial")),
	    "lns<32,24,uint32_t> Polynomial Tier 1 cancellation", test_tag);
	// ArnoldBailey on Tier 1: most cases hit either an integer-d knot
	// (exact), the cancellation-regime direct-eval fallback (exact), or a
	// zero short-circuit. The lone case stressing the secant-interpolation
	// region is "1 + 2^-6": d = -6 sits at the tail-ramp endpoint where the
	// piecewise-linear approximation rounds to 0 (true sb_add(-6) ~= 0.022),
	// giving ~0.016 absolute value-domain error. tolScale=5 (5e-2 absolute
	// against the 1e-2 base) sits just above the observed envelope.
	nrOfFailedTestCases += ReportTestResult(
	    (VerifyTier1CancellationCases<LNS_HiPrec,
	                                  ArnoldBaileyAddSub<LNS_HiPrec>>(reportTestCases, "ArnoldBailey", 5.0)),
	    "lns<32,24,uint32_t> ArnoldBailey Tier 1 cancellation", test_tag);
#endif

#if REGRESSION_LEVEL_2
	using LNS8_3_sat_l2 = lns<8, 3, std::uint8_t>;
	nrOfFailedTestCases += ReportTestResult(VerifyAddAlgorithmAgreement<LNS8_3_sat_l2>(reportTestCases, 0.05),
	                                        "lns<8,3,uint8_t> add: Direct vs DoubleTrip", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubAlgorithmAgreement<LNS8_3_sat_l2>(reportTestCases, 0.05),
	                                        "lns<8,3,uint8_t> sub: Direct vs DoubleTrip", test_tag);
	nrOfFailedTestCases += ReportTestResult(
	    (VerifyAddAlgorithmsAgree<LNS8_3_sat_l2,
	                              LookupAddSub<LNS8_3_sat_l2>,
	                              DirectEvaluationAddSub<LNS8_3_sat_l2>>(reportTestCases, 0.05)),
	    "lns<8,3,uint8_t> add: Lookup vs Direct", test_tag);
	nrOfFailedTestCases += ReportTestResult(
	    (VerifySubAlgorithmsAgree<LNS8_3_sat_l2,
	                              LookupAddSub<LNS8_3_sat_l2>,
	                              DirectEvaluationAddSub<LNS8_3_sat_l2>>(reportTestCases, 0.05)),
	    "lns<8,3,uint8_t> sub: Lookup vs Direct", test_tag);
#endif

#if REGRESSION_LEVEL_3
	using LNS9_4_sat = lns<9, 4, std::uint8_t>;
	using LNS16_8 = lns<16, 8, std::uint16_t>;
	nrOfFailedTestCases += ReportTestResult(VerifyAddAlgorithmAgreement<LNS9_4_sat>(reportTestCases, 0.05),
	                                        "lns<9,4,uint8_t> add: Direct vs DoubleTrip", test_tag);
	nrOfFailedTestCases += ReportTestResult(
	    (VerifyAddAlgorithmsAgree<LNS9_4_sat,
	                              LookupAddSub<LNS9_4_sat>,
	                              DirectEvaluationAddSub<LNS9_4_sat>>(reportTestCases, 0.05)),
	    "lns<9,4,uint8_t> add: Lookup vs Direct", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDirectEvalCornerCases<LNS16_8>(reportTestCases),
	                                        "lns<16,8,uint16_t> Direct corner cases", test_tag);
	nrOfFailedTestCases += ReportTestResult(
	    (VerifyAlgorithmCornerCases<LNS16_8, LookupAddSub<LNS16_8>>(reportTestCases, "Lookup", 200.0)),
	    "lns<16,8,uint16_t> Lookup corner cases", test_tag);
#endif

#if REGRESSION_LEVEL_4
	using LNS10_4_sat = lns<10, 4, std::uint8_t>;
	nrOfFailedTestCases += ReportTestResult(VerifyAddAlgorithmAgreement<LNS10_4_sat>(reportTestCases, 0.05),
	                                        "lns<10,4,uint8_t> add: Direct vs DoubleTrip", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubAlgorithmAgreement<LNS10_4_sat>(reportTestCases, 0.05),
	                                        "lns<10,4,uint8_t> sub: Direct vs DoubleTrip", test_tag);
	nrOfFailedTestCases += ReportTestResult(
	    (VerifyAddAlgorithmsAgree<LNS10_4_sat,
	                              LookupAddSub<LNS10_4_sat>,
	                              DirectEvaluationAddSub<LNS10_4_sat>>(reportTestCases, 0.05)),
	    "lns<10,4,uint8_t> add: Lookup vs Direct", test_tag);
	nrOfFailedTestCases += ReportTestResult(
	    (VerifySubAlgorithmsAgree<LNS10_4_sat,
	                              LookupAddSub<LNS10_4_sat>,
	                              DirectEvaluationAddSub<LNS10_4_sat>>(reportTestCases, 0.05)),
	    "lns<10,4,uint8_t> sub: Lookup vs Direct", test_tag);
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
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
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
