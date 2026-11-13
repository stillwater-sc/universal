// expansion_ops.cpp: Unit tests for expansion operations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/internal/expansion/expansion_ops.hpp>
#include <universal/verification/dyadic_exact.hpp>
#include <random>

namespace sw { namespace universal {

	// Test TWO-SUM error-free transformation
	int test_two_sum() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing TWO-SUM error-free transformation\n";

		// Test case 1: Large + small (classic catastrophic cancellation case)
		{
			double a = 1.0e16;
			double b = 1.0;
			double sum, error;

			two_sum(a, b, sum, error);

			// The floating-point sum loses the 1.0
			if (sum != a) ++nrOfFailedTests;
			// But TWO-SUM captures it in the error term
			if (error != b) ++nrOfFailedTests;
		}

		// Test case 2: Opposite signs (cancellation)
		{
			double a = 1.0;
			double b = -1.0;
			double sum, error;

			two_sum(a, b, sum, error);

			// Should get exact zero
			if (sum != 0.0) ++nrOfFailedTests;
			if (error != 0.0) ++nrOfFailedTests;
		}

		// Test case 3: Values that round
		{
			double a = 1.0;
			double b = 1.0e-20;
			double sum, error;

			two_sum(a, b, sum, error);

			// sum + error should exactly equal a + b (no rounding in double-double)
			// We verify by converting to higher precision
			long double check = static_cast<long double>(sum) + static_cast<long double>(error);
			long double exact = static_cast<long double>(a) + static_cast<long double>(b);

			// They should be equal in long double precision
			if (check != exact) ++nrOfFailedTests;
		}

		// Test case 4: Both positive, close in magnitude
		{
			double a = 3.0;
			double b = 2.0;
			double sum, error;

			two_sum(a, b, sum, error);

			if (sum != 5.0) ++nrOfFailedTests;
			if (error != 0.0) ++nrOfFailedTests;  // Exact in this case
		}

		return nrOfFailedTests;
	}

	// Test FAST-TWO-SUM (when |a| >= |b|)
	int test_fast_two_sum() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing FAST-TWO-SUM error-free transformation\n";

		// Test case 1: |a| > |b| (precondition satisfied)
		{
			double a = 100.0;
			double b = 0.5;
			double sum, error;

			fast_two_sum(a, b, sum, error);

			if (sum != 100.5) ++nrOfFailedTests;
			if (error != 0.0) ++nrOfFailedTests;
		}

		// Test case 2: Large + small
		{
			double a = 1.0e16;
			double b = 1.0;
			double sum, error;

			fast_two_sum(a, b, sum, error);

			// Same behavior as TWO-SUM for this case
			if (sum != a) ++nrOfFailedTests;
			if (error != b) ++nrOfFailedTests;
		}

		// Test case 3: Negative larger magnitude
		{
			double a = -1000.0;
			double b = 1.0;
			double sum, error;

			fast_two_sum(a, b, sum, error);

			if (sum != -999.0) ++nrOfFailedTests;
			if (error != 0.0) ++nrOfFailedTests;
		}

		return nrOfFailedTests;
	}

	// Test TWO-PROD error-free multiplication
	int test_two_prod() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing TWO-PROD error-free multiplication\n";

		// Test case 1: Simple multiplication
		{
			double a = 2.0;
			double b = 3.0;
			double product, error;

			two_prod(a, b, product, error);

			if (product != 6.0) ++nrOfFailedTests;
			if (error != 0.0) ++nrOfFailedTests;
		}

		// Test case 2: Multiplication that rounds
		{
			double a = 1.5;
			double b = 0.3;
			double product, error;

			two_prod(a, b, product, error);

			// product + error should equal a * b exactly
			// The error captures rounding in the multiplication
			double check = product + error;
			double expected = a * b;

			// In double precision, these should be equal
			if (check != expected) ++nrOfFailedTests;
		}

		// Test case 3: Small values
		{
			double a = 1.0e-10;
			double b = 1.0e-10;
			double product, error;

			two_prod(a, b, product, error);

			// Product is 1.0e-20
			// Error captures any rounding
			if (std::abs(product - 1.0e-20) > 1.0e-30) ++nrOfFailedTests;
		}

		return nrOfFailedTests;
	}

	// Test GROW-EXPANSION
	int test_grow_expansion() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing GROW-EXPANSION\n";

		// Test case 1: Grow a 2-component expansion
		{
			std::vector<double> e = { 3.0, 5.0e-16 };
			double b = 1.0;

			std::vector<double> h = grow_expansion(e, b);

			// Should have 3 components
			if (h.size() != 3) ++nrOfFailedTests;

			// Verify value is preserved: h should equal e + b
			double e_sum = 0.0, h_sum = 0.0;
			for (auto v : e) e_sum += v;
			for (auto v : h) h_sum += v;

			// h_sum should equal e_sum + b
			double expected = e_sum + b;
			if (std::abs(h_sum - expected) > 1.0e-14) ++nrOfFailedTests;

			// Verify decreasing magnitude
			if (!is_decreasing_magnitude(h)) ++nrOfFailedTests;
		}

		// Test case 2: Grow empty expansion
		{
			std::vector<double> e;
			double b = 42.0;

			std::vector<double> h = grow_expansion(e, b);

			if (h.size() != 1) ++nrOfFailedTests;
			if (h[0] != b) ++nrOfFailedTests;
		}

		// Test case 3: Grow with zero
		{
			std::vector<double> e = { 1.0 };
			double b = 0.0;

			std::vector<double> h = grow_expansion(e, b);

			// Should still work, may have zeros
			if (h.empty()) ++nrOfFailedTests;
		}

		return nrOfFailedTests;
	}

	// the exact value of an expansion, as a dyadic rational
	dyadic exact_sum(const std::vector<double>& e) {
		dyadic acc;
		for (double v : e) acc = acc + dyadic::from_double(v);
		return acc;
	}

	// Test FAST-EXPANSION-SUM: the sum is EXACT, and the output is in decreasing magnitude. A tolerance of 1e-14 on a double sum of the components let
	// an implementation that dropped components pass ({3, 1e-20} + {2, 1e-30} lost both
	// tails, #2054); every check here is exact dyadic equality.
	int test_fast_expansion_sum() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing FAST-EXPANSION-SUM\n";

		auto check = [&](const std::vector<double>& e, const std::vector<double>& f, const char* label) {
			const std::vector<double> h = fast_expansion_sum(e, f);
			// Shewchuk's output is non-overlapping bit-wise, not in Priest's canonical form
			// (is_nonoverlapping): the callers renormalize when they need that
			if (exact_sum(h) != exact_sum(e) + exact_sum(f) || !is_decreasing_magnitude(h)) {
				++nrOfFailedTests;
				std::cout << "FAIL: fast_expansion_sum " << label << '\n';
			}
			return h;
		};

		check({ 3.0, 1.0e-20 }, { 2.0, 1.0e-30 }, "{3, 1e-20} + {2, 1e-30}");
		check({ 3.0, 5.0e-16 }, { 2.0, 3.0e-16 }, "{3, 5e-16} + {2, 3e-16}");
		check({ 1.0, 1.0e-200 }, { -1.0 }, "{1, 1e-200} + {-1}");
		check({ 1.0e300, 1.0e100 }, { -1.0e300, 1.0e-100 }, "cancellation of the leading components");
		check({ 0.0 }, { 0.0 }, "zero + zero");

		// adding an empty expansion returns the other unchanged
		{
			std::vector<double> e = { 1.0, 2.0 };
			std::vector<double> h = fast_expansion_sum(e, std::vector<double>{});
			if (h != e) ++nrOfFailedTests;
		}

		// (a + b) - a == b, exactly
		{
			std::vector<double> a = { 1.5, 1.5e-17 };
			std::vector<double> b = { 0.5, 5.0e-18 };
			std::vector<double> neg_a = a;
			for (auto& v : neg_a) v = -v;
			const std::vector<double> recovered_b = fast_expansion_sum(fast_expansion_sum(a, b), neg_a);
			if (exact_sum(recovered_b) != exact_sum(b)) {
				++nrOfFailedTests;
				std::cout << "FAIL: fast_expansion_sum (a + b) - a != b\n";
			}
		}

		// random expansions, made non-overlapping by renormalization
		{
			std::mt19937_64 rng(2054);
			std::uniform_real_distribution<double> mantissa(-1.0, 1.0);
			std::uniform_int_distribution<int> exponent(-80, 80), length(1, 8);
			auto random_expansion = [&]() {
				std::vector<double> v(static_cast<size_t>(length(rng)));
				for (auto& c : v) c = std::ldexp(mantissa(rng), exponent(rng));
				return renormalize_expansion(v);
			};
			for (int t = 0; t < 2000; ++t) {
				const std::vector<double> e = random_expansion(), f = random_expansion();
				if (e.empty() || f.empty()) continue;
				check(e, f, "random");
				if (nrOfFailedTests > 10) break;
			}
		}

		return nrOfFailedTests;
	}

	// the special values and zeros of scale_expansion, compress_expansion and expansion_quotient
	int test_expansion_edge_cases() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;
		const double inf = std::numeric_limits<double>::infinity();
		const double nan = std::numeric_limits<double>::quiet_NaN();
		auto fail = [&](const char* label) { ++nrOfFailedTests; std::cout << "FAIL: " << label << '\n'; };

		// a zero scales to zero, not to the empty expansion
		{
			const std::vector<double> h = scale_expansion(std::vector<double>{ 0.0 }, 5.0);
			if (h.size() != 1 || h[0] != 0.0) fail("scale_expansion({0}, 5) != {0}");
			const std::vector<double> k = scale_expansion(std::vector<double>{ 3.0, 1e-20 }, 0.0);
			if (k.size() != 1 || k[0] != 0.0) fail("scale_expansion(e, 0) != {0}");
			const std::vector<double> m = scale_expansion(std::vector<double>{ 3.0, 1e-20 }, 7.0);
			if (exact_sum(m) != dyadic::from_double(21.0) + dyadic::from_double(7.0) * dyadic::from_double(1e-20)) fail("scale_expansion({3, 1e-20}, 7) is not exact");
		}
		// a non-finite component is the value of the compressed expansion
		{
			const std::vector<double> h = compress_expansion(std::vector<double>{ 1.0, nan, 1e-20 }, 0.0);
			if (h.size() != 1 || !std::isnan(h[0])) fail("compress_expansion dropped a NaN");
			const std::vector<double> k = compress_expansion(std::vector<double>{ inf, 1.0 }, 0.0);
			if (k.size() != 1 || k[0] != inf) fail("compress_expansion({inf, 1}) != {inf}");
			const std::vector<double> m = compress_expansion(std::vector<double>{ -inf, 1.0 }, 1e-10);
			if (m.size() != 1 || m[0] != -inf) fail("compress_expansion({-inf, 1}) != {-inf}");
			const std::vector<double> n = compress_expansion(std::vector<double>{ inf, -inf }, 0.0);
			if (n.size() != 1 || !std::isnan(n[0])) fail("compress_expansion({inf, -inf}) != {NaN}");
		}
		// the IEEE quotient for a zero or non-finite divisor
		{
			auto q = [](double a, double b) { return expansion_quotient(std::vector<double>{ a }, std::vector<double>{ b }); };
			std::vector<double> r = q(1.0, 0.0);
			if (r.size() != 1 || r[0] != inf) fail("expansion_quotient({1}, {0}) != {inf}");
			r = q(-1.0, 0.0);
			if (r.size() != 1 || r[0] != -inf) fail("expansion_quotient({-1}, {0}) != {-inf}");
			r = q(1.0, -0.0);
			if (r.size() != 1 || r[0] != -inf) fail("expansion_quotient({1}, {-0}) != {-inf}");
			r = q(0.0, 0.0);
			if (r.size() != 1 || !std::isnan(r[0])) fail("expansion_quotient({0}, {0}) != {NaN}");
			r = q(3.0, inf);
			if (r.size() != 1 || r[0] != 0.0 || std::signbit(r[0])) fail("expansion_quotient({3}, {inf}) != {+0}");
			r = q(-3.0, inf);
			if (r.size() != 1 || r[0] != 0.0 || !std::signbit(r[0])) fail("expansion_quotient({-3}, {inf}) != {-0}");
		}
		return nrOfFailedTests;
	}

	// Test LINEAR-EXPANSION-SUM
	int test_linear_expansion_sum() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing LINEAR-EXPANSION-SUM\n";

		// Test case 1: Same as FAST but using LINEAR
		{
			std::vector<double> e = { 10.0, 1.0e-15 };
			std::vector<double> f = { 5.0, 2.0e-15 };

			std::vector<double> h = linear_expansion_sum(e, f);

			// Verify value
			double e_sum = 0.0, f_sum = 0.0, h_sum = 0.0;
			for (auto v : e) e_sum += v;
			for (auto v : f) f_sum += v;
			for (auto v : h) h_sum += v;

			double expected = e_sum + f_sum;
			if (std::abs(h_sum - expected) > 1.0e-13) ++nrOfFailedTests;

			// Verify properties
			if (!is_decreasing_magnitude(h)) ++nrOfFailedTests;
		}

		// Test case 2: Compare LINEAR vs FAST results
		{
			std::vector<double> e = { 7.0, 3.5e-16 };
			std::vector<double> f = { 3.0, 1.2e-16 };

			std::vector<double> h_fast = fast_expansion_sum(e, f);
			std::vector<double> h_linear = linear_expansion_sum(e, f);

			// Both should give same value
			double fast_sum = 0.0, linear_sum = 0.0;
			for (auto v : h_fast) fast_sum += v;
			for (auto v : h_linear) linear_sum += v;

			if (std::abs(fast_sum - linear_sum) > 1.0e-14) ++nrOfFailedTests;
		}

		return nrOfFailedTests;
	}

	// Test invariant verification functions
	int test_invariants() {
		using namespace expansion_ops;
		int nrOfFailedTests = 0;

		std::cout << "Testing invariant verification functions\n";

		// Test is_decreasing_magnitude
		{
			std::vector<double> e1 = { 10.0, 1.0, 0.1 };
			std::vector<double> e2 = { 10.0, 0.1, 1.0 };
			std::vector<double> e3 = { -10.0, -1.0, -0.1 };  // Negative but decreasing magnitude

			if (!is_decreasing_magnitude(e1)) ++nrOfFailedTests;
			if (is_decreasing_magnitude(e2)) ++nrOfFailedTests;  // Should fail
			if (!is_decreasing_magnitude(e3)) ++nrOfFailedTests;
		}

		// Test estimate
		{
			std::vector<double> e = { 1.0, 5.0e-16, 3.0e-32, 1.0e-48 };
			double est = estimate(e);

			// Should be close to 1.0 (dominated by first component)
			if (std::abs(est - 1.0) > 1.0e-14) ++nrOfFailedTests;
		}

		// Test is_nonoverlapping (issue #999: the predicate used to be inverted).
		// Non-overlapping means |e[i]| <= ulp(e[i-1])/2 == 2^(ilogb(e[i-1]) - 53).
		{
			// genuinely non-overlapping: small component well below the ulp boundary
			if (!is_nonoverlapping({ 2.0, std::ldexp(1.0, -60) })) ++nrOfFailedTests;
			// heavily overlapping: 0.5 is far above ulp(1.0)/2
			if ( is_nonoverlapping({ 1.0, 0.5 })) ++nrOfFailedTests;
			// exactly at the half-ulp boundary is allowed (non-overlapping)
			if (!is_nonoverlapping({ 1.0, std::ldexp(1.0, -53) })) ++nrOfFailedTests;
			// one bit above the boundary overlaps
			if ( is_nonoverlapping({ 1.0, std::ldexp(1.0, -52) })) ++nrOfFailedTests;
			// multi-component canonical expansion is non-overlapping
			if (!is_nonoverlapping({ 1.0, std::ldexp(1.0, -60), std::ldexp(1.0, -120) })) ++nrOfFailedTests;
			// a single component (and empty) is trivially non-overlapping
			if (!is_nonoverlapping({ 3.0 })) ++nrOfFailedTests;
			if (!is_nonoverlapping({})) ++nrOfFailedTests;
			// negative components: magnitudes obey the same gap rule
			if (!is_nonoverlapping({ -8.0, std::ldexp(1.0, -55) })) ++nrOfFailedTests;
		}

		// Test is_strongly_nonoverlapping (Shewchuk's strict property).
		{
			if (!is_strongly_nonoverlapping({ 1.0, std::ldexp(1.0, -53) })) ++nrOfFailedTests; // power-of-two at boundary
			if (!is_strongly_nonoverlapping({ 1.0, std::ldexp(1.0, -60) })) ++nrOfFailedTests; // well separated
			if ( is_strongly_nonoverlapping({ 1.0, 0.5 })) ++nrOfFailedTests;                  // overlapping
		}

		return nrOfFailedTests;
	}

}} // namespace sw::universal

// Main test driver
#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 1
#	define REGRESSION_LEVEL_3 0
#	define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "expansion operations (EFTs, grow, sums, invariants)";
	std::string test_tag    = "expansion ops";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if REGRESSION_LEVEL_1

	nrOfFailedTestCases += ReportTestResult(test_two_sum(),             "expansion", "two_sum");
	nrOfFailedTestCases += ReportTestResult(test_fast_two_sum(),        "expansion", "fast_two_sum");
	nrOfFailedTestCases += ReportTestResult(test_two_prod(),            "expansion", "two_prod");
	nrOfFailedTestCases += ReportTestResult(test_grow_expansion(),      "expansion", "grow_expansion");
	nrOfFailedTestCases += ReportTestResult(test_fast_expansion_sum(),  "expansion", "fast_expansion_sum");
	nrOfFailedTestCases += ReportTestResult(test_expansion_edge_cases(),"expansion", "edge cases");
	nrOfFailedTestCases += ReportTestResult(test_linear_expansion_sum(),"expansion", "linear_expansion_sum");
	nrOfFailedTestCases += ReportTestResult(test_invariants(),          "expansion", "invariants");

#endif
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
