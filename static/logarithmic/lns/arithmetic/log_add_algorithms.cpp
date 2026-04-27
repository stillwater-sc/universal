// log_add_algorithms.cpp: cross-validation of configurable lns add/sub algorithms
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Phase A regression for issue #777: validates that the alternate add/sub
// algorithm (DirectEvaluationAddSub, using sw::math::constexpr_math::log2/exp2)
// agrees with the historical DoubleTripAddSub baseline within a small ULP
// tolerance across representative lns configurations.
//
// Direct invocation of both policy classes lets us cross-validate them against
// the same lns instantiation without specializing the traits.

#include <universal/utility/directives.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_reporters.hpp>

namespace sw { namespace universal {

	// Cross-validate two add/sub policies against each other in the value
	// domain. Both encode through the same lns instantiation, so the upper
	// bound on disagreement is dominated by transcendental rounding plus the
	// encode rounding -- in practice a few percent relative error is generous
	// for small lns sizes where the encoding step itself is coarse.
	template<typename LnsType>
	int VerifyAddAlgorithmAgreement(bool reportTestCases, double relTol = 0.05) {
		constexpr unsigned nbits = LnsType::nbits;
		static_assert(nbits < 64, "exhaustive sweep requires nbits < 64 to avoid shift UB");
		constexpr size_t NR_ENCODINGS = (1ull << nbits);

		using Direct = DirectEvaluationAddSub<LnsType>;
		using DoubleTrip = DoubleTripAddSub<LnsType>;

		int nrOfFailedTestCases = 0;
		int reported = 0;

		for (size_t i = 0; i < NR_ENCODINGS; ++i) {
			LnsType a; a.setbits(i);
			if (a.isnan()) continue;
			for (size_t j = 0; j < NR_ENCODINGS; ++j) {
				LnsType b; b.setbits(j);
				if (b.isnan()) continue;

				LnsType cTrip = a; DoubleTrip::add_assign(cTrip, b);
				LnsType cDir  = a; Direct::add_assign(cDir, b);

				if (cTrip == cDir) continue;
				if (cTrip.isnan() && cDir.isnan()) continue;

				double vTrip = double(cTrip);
				double vDir  = double(cDir);
				double diff  = vTrip - vDir;
				if (diff < 0.0) diff = -diff;
				double mag = (vTrip < 0.0 ? -vTrip : vTrip);
				if (mag < 1.0) mag = 1.0;
				if (diff / mag <= relTol) continue;

				++nrOfFailedTestCases;
				if (reportTestCases && reported < 8) {
					std::cout << "MISMATCH a=" << double(a) << " b=" << double(b)
					          << " trip=" << vTrip << " dir=" << vDir
					          << " relErr=" << (diff / mag) << '\n';
					++reported;
				}
				if (nrOfFailedTestCases > 24) return nrOfFailedTestCases;
			}
		}
		return nrOfFailedTestCases;
	}

	template<typename LnsType>
	int VerifySubAlgorithmAgreement(bool reportTestCases, double relTol = 0.05) {
		constexpr unsigned nbits = LnsType::nbits;
		static_assert(nbits < 64, "exhaustive sweep requires nbits < 64 to avoid shift UB");
		constexpr size_t NR_ENCODINGS = (1ull << nbits);

		using Direct = DirectEvaluationAddSub<LnsType>;
		using DoubleTrip = DoubleTripAddSub<LnsType>;

		int nrOfFailedTestCases = 0;
		int reported = 0;

		for (size_t i = 0; i < NR_ENCODINGS; ++i) {
			LnsType a; a.setbits(i);
			if (a.isnan()) continue;
			for (size_t j = 0; j < NR_ENCODINGS; ++j) {
				LnsType b; b.setbits(j);
				if (b.isnan()) continue;

				LnsType cTrip = a; DoubleTrip::sub_assign(cTrip, b);
				LnsType cDir  = a; Direct::sub_assign(cDir, b);

				if (cTrip == cDir) continue;
				if (cTrip.isnan() && cDir.isnan()) continue;

				double vTrip = double(cTrip);
				double vDir  = double(cDir);
				double diff  = vTrip - vDir;
				if (diff < 0.0) diff = -diff;
				double mag = (vTrip < 0.0 ? -vTrip : vTrip);
				if (mag < 1.0) mag = 1.0;
				if (diff / mag <= relTol) continue;

				++nrOfFailedTestCases;
				if (reportTestCases && reported < 8) {
					std::cout << "MISMATCH a=" << double(a) << " b=" << double(b)
					          << " trip=" << vTrip << " dir=" << vDir
					          << " relErr=" << (diff / mag) << '\n';
					++reported;
				}
				if (nrOfFailedTestCases > 24) return nrOfFailedTestCases;
			}
		}
		return nrOfFailedTestCases;
	}

	// Spot-check the DirectEvaluationAddSub corner cases that the exhaustive
	// sweep above doesn't exercise (since exhaustive sweeps are restricted to
	// small lns sizes for runtime reasons). Use a high-precision LnsType so
	// encoding error doesn't confound the algorithm-correctness check.
	template<typename LnsType>
	int VerifyDirectEvalCornerCases(bool reportTestCases) {
		using Direct = DirectEvaluationAddSub<LnsType>;
		int failed = 0;

		auto check = [&](const char* name, double expected, const LnsType& got, double absTol) {
			double g = double(got);
			if (g != g && expected != expected) return;
			double diff = g - expected;
			if (diff < 0.0) diff = -diff;
			if (diff > absTol) {
				++failed;
				if (reportTestCases) {
					std::cout << "FAIL " << name << "  expected=" << expected
					          << "  got=" << g << "  diff=" << diff << '\n';
				}
			}
		};

		// a + (-a) -> 0 (exact cancellation)
		{
			LnsType a(2.5);
			LnsType minus_a = -a;
			LnsType r = a; Direct::add_assign(r, minus_a);
			check("2.5 + (-2.5)", 0.0, r, 1e-12);
		}
		// a - a -> 0 (exact cancellation)
		{
			LnsType a(3.75);
			LnsType r = a; Direct::sub_assign(r, a);
			check("3.75 - 3.75", 0.0, r, 1e-12);
		}
		// 1 + 1 -> 2
		{
			LnsType a(1.0);
			LnsType b(1.0);
			LnsType r = a; Direct::add_assign(r, b);
			check("1 + 1", 2.0, r, 1e-6);
		}
		// 1 + 0 -> 1 (zero short-circuit)
		{
			LnsType a(1.0);
			LnsType b(0.0);
			LnsType r = a; Direct::add_assign(r, b);
			check("1 + 0", 1.0, r, 1e-12);
		}
		// 0 + 2 -> 2 (zero short-circuit on lhs)
		{
			LnsType a(0.0);
			LnsType b(2.0);
			LnsType r = a; Direct::add_assign(r, b);
			check("0 + 2", 2.0, r, 1e-6);
		}
		// 2 + (-1) -> 1 (mixed sign, magnitude subtraction)
		{
			LnsType a(2.0);
			LnsType b(-1.0);
			LnsType r = a; Direct::add_assign(r, b);
			check("2 + (-1)", 1.0, r, 1e-6);
		}
		// (-3) + 5 -> 2
		{
			LnsType a(-3.0);
			LnsType b(5.0);
			LnsType r = a; Direct::add_assign(r, b);
			check("-3 + 5", 2.0, r, 1e-6);
		}
		// large dynamic range: 4 + tiny -> ~4 (Lb - La very negative => sb_add ~ 0)
		{
			LnsType a(4.0);
			LnsType tiny(0.001953125);  // 2^-9, well within lns dynamic range
			LnsType r = a; Direct::add_assign(r, tiny);
			// expected ~ 4.001953125; allow lns encoding error
			double expected = 4.001953125;
			check("4 + 2^-9", expected, r, 0.05);
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

	std::string test_suite  = "lns add/sub algorithm cross-validation (issue #777 Phase A)";
	std::string test_tag    = "log_add_algorithms";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if REGRESSION_LEVEL_1
	using LNS5_2_sat = lns<5, 2, std::uint8_t>;
	using LNS6_2_sat = lns<6, 2, std::uint8_t>;
	using LNS_HiPrec = lns<32, 24, std::uint32_t>;  // ~7 decimal digits in log domain

	// Algorithm cross-validation: DirectEvaluation vs DoubleTrip
	// Allow ~5% relative value-domain tolerance: the two paths use different
	// transcendental routines and round at different points; for small lns
	// sizes the encoding step itself is coarser than the algorithm gap.
	nrOfFailedTestCases += ReportTestResult(VerifyAddAlgorithmAgreement<LNS5_2_sat>(reportTestCases, 0.05),
	                                        "lns<5,2,uint8_t> add: Direct vs DoubleTrip", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubAlgorithmAgreement<LNS5_2_sat>(reportTestCases, 0.05),
	                                        "lns<5,2,uint8_t> sub: Direct vs DoubleTrip", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyAddAlgorithmAgreement<LNS6_2_sat>(reportTestCases, 0.05),
	                                        "lns<6,2,uint8_t> add: Direct vs DoubleTrip", test_tag);

	// Corner cases in DirectEvaluationAddSub: use high-precision lns so
	// encoding rounding doesn't drown out algorithm-correctness signal.
	nrOfFailedTestCases += ReportTestResult(VerifyDirectEvalCornerCases<LNS_HiPrec>(reportTestCases),
	                                        "lns<32,24,uint32_t> Direct corner cases", test_tag);
#endif

#if REGRESSION_LEVEL_2
	using LNS8_3_sat_l2 = lns<8, 3, std::uint8_t>;
	nrOfFailedTestCases += ReportTestResult(VerifyAddAlgorithmAgreement<LNS8_3_sat_l2>(reportTestCases, 0.05),
	                                        "lns<8,3,uint8_t> add: Direct vs DoubleTrip", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubAlgorithmAgreement<LNS8_3_sat_l2>(reportTestCases, 0.05),
	                                        "lns<8,3,uint8_t> sub: Direct vs DoubleTrip", test_tag);
#endif

#if REGRESSION_LEVEL_3
	using LNS9_4_sat = lns<9, 4, std::uint8_t>;
	using LNS16_8 = lns<16, 8, std::uint16_t>;
	nrOfFailedTestCases += ReportTestResult(VerifyAddAlgorithmAgreement<LNS9_4_sat>(reportTestCases, 0.05),
	                                        "lns<9,4,uint8_t> add: Direct vs DoubleTrip", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDirectEvalCornerCases<LNS16_8>(reportTestCases),
	                                        "lns<16,8,uint16_t> Direct corner cases", test_tag);
#endif

#if REGRESSION_LEVEL_4
	using LNS10_4_sat = lns<10, 4, std::uint8_t>;
	nrOfFailedTestCases += ReportTestResult(VerifyAddAlgorithmAgreement<LNS10_4_sat>(reportTestCases, 0.05),
	                                        "lns<10,4,uint8_t> add: Direct vs DoubleTrip", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySubAlgorithmAgreement<LNS10_4_sat>(reportTestCases, 0.05),
	                                        "lns<10,4,uint8_t> sub: Direct vs DoubleTrip", test_tag);
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
