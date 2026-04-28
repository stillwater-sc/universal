// addition.cpp: test suite runner for addition arithmetic on fixed-sized, arbitrary precision logarithmic number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/verification/lns_test_suite.hpp>

namespace sw::universal {
// The shipped algorithms:
//   - DoubleTripAddSub        -- default, preserves current behavior
//   - DirectEvaluationAddSub  -- uses sw::math::constexpr_math::log2/exp2
//   - LookupAddSub            -- Mitchell-style precomputed table + linear interp
//   - PolynomialAddSub        -- (1+x)/(1-x) substitution + degree-7 odd polynomial
//   - ArnoldBaileyAddSub      -- piecewise-linear, no transcendentals

	template<>
	struct lns_addsub_traits<lns<4, 1, std::uint8_t>> {
		using type = ArnoldBaileyAddSub<lns<4, 1, std::uint8_t>>;
	};
    template<>
    struct lns_addsub_traits<lns<4, 2, std::uint8_t>> {
	    using type = ArnoldBaileyAddSub<lns<4, 2, std::uint8_t>>;
    };
    template<>
    struct lns_addsub_traits<lns<5, 2, std::uint8_t>> {
	    using type = ArnoldBaileyAddSub<lns<5, 2, std::uint8_t>>;
    };
	template<>
	struct lns_addsub_traits<lns<8, 3, std::uint8_t>> {
		//using type = DirectEvaluationAddSub<lns<8, 3, std::uint8_t>>;
	    //using type = LookupAddSub<lns<8, 3, std::uint8_t>>;
	    using type = PolynomialAddSub<lns<8, 3, std::uint8_t>>;
	    //using type = ArnoldBaileyAddSub<lns<8, 3, std::uint8_t>>;
    };

}

/*

  Why ArnoldBailey fails the standard addition.cpp regression

  addition.cpp does a bit-exact comparison: c = a + b (through whatever lns algorithm is selected) vs cref = encode(double(a) + double(b)). 
  That comparison only passes when lns_encode(algorithm_result) == lns_encode(true_result) — which requires the algorithm error to 
  stay strictly below the ULP-flipping boundary.

  For lns<8, 2>:
  - rbits = 2 -> log-domain ULP = 2^-2 = 0.25
  - ULP-flipping happens when the algorithm result lands within +-0.02 (or so) of a 0.25-wide boundary

  ArnoldBailey's worst-case secant error on sb_add(d) is ~0.02 in the log domain (mid-interval, near d ~ -0.5 
  where curvature is highest). That's well below lns<8,2>'s ULP, but large enough that ~10-20% of true results 
  that happen to lie within +-0.02 of an encoding boundary will round to the adjacent ULP.
  Bit-exact regression flags those as failures.

  The math:
  - ArnoldBailey <= 2.5% relative error → ~0.02 absolute in log domain
  - lns<8,2> ULP = 25% relative -> ~0.25 in log domain
  - Algorithm error / ULP ~ 8% -> small enough to round-correctly most of the time, not small enough to round-correctly all of the time

  What the framework prescribes

  The sister test suite in log_add_algorithms.cpp was designed as the algorithm-correctness test bench for this exact reason — 
  it uses value-domain relative tolerance (5% in the algorithm-agreement sweeps, scaled per-algorithm in corner cases) instead of bit-exact comparison.

  The decision tree in docs/design/lns-add-sub.md flags this implicitly: ArnoldBailey is positioned as "lowest energy, ~2.5% rel error" — meaning users
  opting into it accept that bit-exact-against-double is not part of the contract.

  Recommendation for this regression suite

  Three options, ordered by how aggressively we want to lean into the framework:

  1. Per-algorithm tolerance: parameterize the regression's c == cref check on lns_addsub_algorithm_t<LnsType> 
     - use c == cref for Direct/DoubleTrip, allow +-1 ULP for Lookup/Polynomial, allow +-2-3 ULP (or relative 
       tolerance) for ArnoldBailey. This is the cleanest if we want one regression to handle all algorithms.
  2. Skip approximate algorithms in the bit-exact regression: gate addition.cpp/subtraction.cpp on 
     Direct/DoubleTrip only via a static_assert or if constexpr, and rely on log_add_algorithms.cpp for the approximate ones.
  3. Treat the failures as algorithm characterization data: log them, count them, and bound their frequency 
     e.g., assert that <5% of operand pairs differ from Direct by >1 ULP. 
     This is closer to what hardware vendors do when validating LNS approximators.

  Why log-domain bound, not flat value-domain bound

  A flat per-algorithm value-domain tolerance was wrong by orders of magnitude across configurations. ArnoldBailey's ~0.025 log-domain error becomes ~19%
  value-domain at rbits=2 (because one log-ULP is 2^0.25 - 1 = 19%) but only ~2% at rbits=8 and effectively saturates at the algorithm bound (~2.5%) for
  rbits >= 16. The helper computes:

  log_ulp = 2^-rbits
  ulp_shift = (E / log_ulp) + 2     // +2 for rounding noise
  rel_tol  = 2^(ulp_shift * log_ulp) - 1

  This auto-scales correctly: tight at high rbits, generously permissive at low rbits where one ULP = 19%.

 */

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

	std::string test_suite  = "lns addition validation";
	std::string test_tag    = "addition";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	using LNS4_1_sat = lns<4, 1, std::uint8_t>;
	using LNS4_2_sat = lns<4, 2, std::uint8_t>;
	using LNS5_2_sat = lns<5, 2, std::uint8_t>;
	using LNS8_3_sat = lns<8, 3, std::uint8_t>;
	using LNS9_4_sat = lns<9, 4, std::uint8_t>;
	using LNS16_5_sat = lns<16, 5, std::uint16_t>;

	// generate individual testcases to hand trace/debug
	TestCase< LNS16_5_sat, double>(TestCaseOperator::ADD, INFINITY, INFINITY);
	TestCase< LNS8_3_sat, float>(TestCaseOperator::ADD, 0.5f, -0.5f);

	// manual exhaustive test
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<LNS4_2_sat>(reportTestCases), "lns<4,2,uint8_t>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	using LNS4_1_sat = lns<4, 1, std::uint8_t>;
	using LNS4_2_sat = lns<4, 2, std::uint8_t>;
	using LNS5_2_sat = lns<5, 2, std::uint8_t>;
	using LNS8_3_sat = lns<8, 3, std::uint8_t>;

	nrOfFailedTestCases += ReportTestResult(VerifyAddition<LNS4_1_sat>(reportTestCases), "lns<4,1,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<LNS4_2_sat>(reportTestCases), "lns<4,2,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<LNS5_2_sat>(reportTestCases), "lns<5,2,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<LNS8_3_sat>(reportTestCases), "lns<8,3,uint8_t>", test_tag);

#endif

#if REGRESSION_LEVEL_2
	using LNS9_4_sat = lns<9, 4, std::uint8_t>;
	using LNS10_4_sat = lns<10, 4, std::uint8_t>;

	nrOfFailedTestCases += ReportTestResult(VerifyAddition<LNS9_4_sat>(reportTestCases), "lns<9,4,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<LNS10_4_sat>(reportTestCases), "lns<10,4,uint8_t>", test_tag);
#endif

#if REGRESSION_LEVEL_3
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
