// sweep.cpp: elreal Phase 9 (#933) high-precision sweep oracle.
//
// Randomised evaluation of elreal ZBCL arithmetic against an INDEPENDENT exact
// oracle. Universal is dependency-free, so in place of mpfr the oracle is the
// exact dyadic-rational type (verification/dyadic_exact.hpp): dyadics are closed
// under +, -, * with no rounding, so they pin down the exact value of any finite
// binary computation. zbcl_to_dyadic() widens a materialised ZBCL to that exact
// value, and we compare.
//
// Checks (swept over host FpType in {double, float}):
//   - construction: a ZBCL built from a fold of exact-in-host doubles equals the
//     exact dyadic sum (the 0-overlap invariant carries no rounding);
//   - add() is EXACT: dyadic(a + b) == dyadic(a) + dyadic(b) to the bit;
//   - sub() (negate + add) is EXACT;
//   - mul(a, b, depth) agrees with the exact dyadic product a*b to a high number
//     of decimal digits (mul truncates at `depth`, so this is a floor, not an
//     equality);
//   - cancellation-stressed accumulation (#1187) is EXACT: two workloads whose
//     terms dwarf their own total -- the naive Taylor series for exp(-40), and an
//     ill-conditioned dot product whose answer is spread over more separated
//     pieces than a fixed-limb type can hold -- accumulate to the exact dyadic
//     value of the same terms, to the bit. This is the property the benchmark's
//     section F reports; here it is a pass/fail guard.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <vector>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <utility>

#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/elreal_reference_digits.hpp>   // zbcl_to_dyadic, agreed_decimal_digits
#include <universal/verification/test_suite.hpp>

namespace {

	using namespace sw::universal;

	int dcmp(const dyadic& a, const dyadic& b) {
		dyadic d = a - b;
		if (d.numerator.iszero()) return 0;
		return d.numerator.sign() ? -1 : 1;
	}

	// a value that is exactly representable in every host FpType we sweep (mantissa
	// well within float's 24 bits), so from_native<FpType> carries it losslessly.
	double exact_host_value(std::mt19937_64& rng) {
		int k = static_cast<int>(rng() % (1u << 20)) - (1 << 19);   // |k| < 2^19
		int e = static_cast<int>(rng() % 61) - 30;                  // scale in [-30, 30]
		return std::ldexp(static_cast<double>(k), e);
	}

	// build a random ZBCL together with its exact dyadic value (a fold of exact
	// host values through the exact add() combinator).
	template<typename FpType>
	std::pair<ZBCL<FpType>, dyadic> random_zbcl(std::mt19937_64& rng, int maxTerms = 5) {
		ZBCL<FpType> z = from_native<FpType>(0.0);
		dyadic D;
		int nt = 1 + static_cast<int>(rng() % static_cast<unsigned>(maxTerms));
		for (int j = 0; j < nt; ++j) {
			double m = exact_host_value(rng);
			z = add(z, from_native<FpType>(m));
			D = D + dyadic::from_double(m);
		}
		return { z, D };
	}

	// construction + add() + sub() exactness
	template<typename FpType>
	int VerifyExactAddSub(const char* host, bool reportTestCases, int nrTests) {
		int fails = 0;
		std::mt19937_64 rng(0xE1 + std::hash<std::string>{}(host));
		for (int t = 0; t < nrTests; ++t) {
			auto [za, Da] = random_zbcl<FpType>(rng);
			auto [zb, Db] = random_zbcl<FpType>(rng);
			// construction sanity: the ZBCL exactly equals its dyadic value
			if (dcmp(zbcl_to_dyadic(za), Da) != 0) { if (reportTestCases) std::cout << "    FAIL construction " << host << '\n'; ++fails; }
			// add is exact
			if (dcmp(zbcl_to_dyadic(add(za, zb)), Da + Db) != 0) { if (reportTestCases) std::cout << "    FAIL add-exact " << host << '\n'; ++fails; }
			// sub via negate is exact
			if (dcmp(zbcl_to_dyadic(add(za, negate(zb))), Da - Db) != 0) { if (reportTestCases) std::cout << "    FAIL sub-exact " << host << '\n'; ++fails; }
		}
		return fails;
	}

	// mul() agreement with the exact dyadic product, to a high digit floor
	template<typename FpType>
	int VerifyMulAgreement(const char* host, bool reportTestCases, int nrTests, std::size_t depth, int minDigits) {
		int fails = 0;
		std::mt19937_64 rng(0x33 + std::hash<std::string>{}(host));
		for (int t = 0; t < nrTests; ++t) {
			auto [za, Da] = random_zbcl<FpType>(rng, 3);
			auto [zb, Db] = random_zbcl<FpType>(rng, 3);
			if (Da.iszero() || Db.iszero()) continue;
			ZBCL<FpType> zc = mul(za, zb, depth);
			int digits = agreed_decimal_digits(zbcl_to_dyadic(zc), Da * Db);
			if (digits < minDigits) {
				if (reportTestCases) std::cout << "    FAIL mul-agreement " << host << " digits=" << digits << " < " << minDigits << '\n';
				++fails;
			}
		}
		return fails;
	}

	// ---- cancellation-stressed accumulation (#1187) --------------------------

	// naive Taylor exp(-40): terms from the recurrence t_k = t_{k-1} * (-40/k),
	// each pre-rounded in double. Their magnitudes peak near 1.5e16 while the sum
	// is of order 1, so the accumulation is where everything can be lost.
	std::vector<double> naive_exp_terms(double x, int maxTerms = 200) {
		std::vector<double> terms;
		double t = 1.0;
		terms.push_back(t);
		for (int k = 1; k <= maxTerms; ++k) {
			t *= x / static_cast<double>(k);
			terms.push_back(t);
			if (k > 60 && std::fabs(t) < 1e-40) break;
		}
		return terms;
	}

	// A generated term is only usable if it survives binary64 intact: neither factor
	// flushed to zero, the product finite and non-zero, and the 26 x 26 -> 52-bit
	// integer product exact. Checked with integers rather than an fma residual,
	// because a platform with a sloppy software fma would false-positive.
	bool representable_term(double x, double y) {
		if (x == 0.0 || y == 0.0) return false;
		double p = x * y;
		if (!std::isfinite(p) || p == 0.0) return false;
		int ex = 0, ey = 0;
		double mx = std::frexp(x, &ex), my = std::frexp(y, &ey);
		std::uint64_t ix = static_cast<std::uint64_t>(std::ldexp(std::fabs(mx), 26));
		std::uint64_t iy = static_cast<std::uint64_t>(std::ldexp(std::fabs(my), 26));
		return (ix * iy) < (1ull << 53);
	}

	// an ill-conditioned dot product whose exact answer is spread over `chunks`
	// 52-bit pieces separated by 100 bits, plus large exactly-cancelling pairs.
	// Both factors carry 26 bits, so every product is exact in double and the only
	// thing under test is the accumulation.
	//
	// The answer is centred on 2^0. Anchoring the top chunk at 2^0 instead would put
	// the bottom one at 2^(-100*(chunks-1)), and at 12 chunks that is 2^-1100 --
	// below the smallest subnormal, so the chunk would quietly round to zero and the
	// test would be scoring a narrower answer than it claims. Returns the number of
	// terms that did not survive binary64, which the caller treats as a failure.
	int gen_illconditioned_dot(int chunks, std::vector<double>& X, std::vector<double>& Y, dyadic& D) {
		std::mt19937_64 rng(0xBEEF + static_cast<unsigned>(chunks));
		X.clear(); Y.clear(); D = dyadic();
		int unrepresentable = 0;
		const int hi = 50 * (chunks - 1);
		for (int j = 0; j < chunks; ++j) {
			double xv = std::ldexp(static_cast<double>((rng() & 0x3FFFFFFull) | (1ull << 25)), hi - 100 * j - 25);
			double yv = static_cast<double>((rng() & 0x3FFFFFFull) | (1ull << 25));
			if (!representable_term(xv, yv)) ++unrepresentable;
			X.push_back(xv); Y.push_back(yv);
			D = D + dyadic::from_double(xv) * dyadic::from_double(yv);
		}
		for (int j = 0; j < 12; ++j) {
			double b  = std::ldexp(static_cast<double>((rng() % 4000) + 1), 850);
			double yb = static_cast<double>((rng() % 4000) + 1);
			if (!representable_term(b, yb)) ++unrepresentable;
			X.push_back(b);  Y.push_back(yb);
			X.push_back(-b); Y.push_back(yb);
		}
		for (std::size_t i = X.size(); i > 1; --i) {   // adjacent +P/-P would not stress anything
			std::size_t k = static_cast<std::size_t>(rng() % i);
			std::swap(X[i - 1], X[k]); std::swap(Y[i - 1], Y[k]);
		}
		return unrepresentable;
	}

	// double host only: the workload terms are doubles, so from_native<float> would
	// round them on the way in and the exact-sum claim would be about different
	// numbers than the dyadic reference was built from. The narrow hosts are
	// covered by the add/sub exactness sweep above, which generates terms that are
	// exact in every host it sweeps.
	template<typename FpType>
	int VerifyCancellationExact(const char* host, bool reportTestCases) {
		int fails = 0;

		// the Taylor sum accumulates to the exact dyadic sum of its terms
		std::vector<double> terms = naive_exp_terms(-40.0);
		dyadic exactSum;
		ZBCL<FpType> z = from_native<FpType>(0.0);
		for (double v : terms) {
			exactSum = exactSum + dyadic::from_double(v);
			z = add(z, from_native<FpType>(v));
		}
		if (dcmp(zbcl_to_dyadic(z), exactSum) != 0) {
			if (reportTestCases) std::cout << "    FAIL taylor-exact " << host << '\n';
			++fails;
		}

		// the dot product accumulates to the exact dyadic dot
		for (int chunks : { 2, 6, 12 }) {
			std::vector<double> X, Y;
			dyadic D;
			int unrepresentable = gen_illconditioned_dot(chunks, X, Y, D);
			if (unrepresentable != 0) {
				// the workload did not survive binary64, so whatever the accumulators
				// agree on afterwards is not the question this test is asking
				if (reportTestCases) std::cout << "    FAIL dot-generator " << host << " chunks=" << chunks
				                               << " has " << unrepresentable << " unrepresentable terms\n";
				++fails;
			}
			ZBCL<FpType> dz = from_native<FpType>(0.0);
			for (std::size_t i = 0; i < X.size(); ++i)
				dz = add(dz, mul(from_native<FpType>(X[i]), from_native<FpType>(Y[i]), 8));
			if (dcmp(zbcl_to_dyadic(dz), D) != 0) {
				if (reportTestCases) std::cout << "    FAIL dot-exact " << host << " chunks=" << chunks << '\n';
				++fails;
			}
		}
		return fails;
	}

	// ---- exact geometric predicates (#1186) ----------------------------------
	// orient2d's sign against the exact dyadic determinant, over collinear points
	// perturbed by single ulps. The degenerate (exactly collinear) cases are the
	// point of this test: elreal's sign() answers +1 for zero and a cancelling sum
	// stays lazily unnormalised, so reading the raw leading block gives a phantom
	// sign on precisely those inputs. The comparison operators route through
	// elreal_cmp, which skips zero blocks. This guard fails if anyone swaps the
	// one for the other.
	int VerifyOrient2dExactSign(bool reportTestCases, int n) {
		int fails = 0, degenerateSeen = 0;
		const double bx = 12.0, by = 12.0, cx = 24.0, cy = 24.0;
		const double u = std::ldexp(1.0, -53);
		auto D = [](double v) { return dyadic::from_double(v); };
		for (int i = 0; i < n; ++i) {
			for (int j = 0; j < n; ++j) {
				double ax = 0.5 + i * u, ay = 0.5 + j * u;
				dyadic E = (D(ax) - D(cx)) * (D(by) - D(cy)) - (D(ay) - D(cy)) * (D(bx) - D(cx));
				int expected = E.numerator.iszero() ? 0 : (E.numerator.sign() ? -1 : 1);
				if (expected == 0) ++degenerateSeen;

				elreal<double> Ax(ax), Ay(ay), Bx(bx), By(by), Cx(cx), Cy(cy), Z(0.0);
				elreal<double> det = (Ax - Cx) * (By - Cy) - (Ay - Cy) * (Bx - Cx);
				int got = det < Z ? -1 : (det > Z ? 1 : 0);
				if (got != expected) {
					if (reportTestCases && fails < 5)
						std::cout << "    FAIL orient2d i=" << i << " j=" << j
						          << " expected " << expected << " got " << got << '\n';
					++fails;
				}
			}
		}
		if (degenerateSeen == 0) {   // the grid must actually contain degeneracies
			if (reportTestCases) std::cout << "    FAIL orient2d grid has no degenerate cases\n";
			++fails;
		}
		return fails;
	}

}  // anonymous namespace

#define MANUAL_TESTING 0
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
	std::string test_suite = "elreal Phase 9 (#933) high-precision sweep oracle (vs exact dyadic)";
	int nrOfFailedTestCases = 0;
	bool reportTestCases = true;
	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// TODO: place hand-run diagnostics here (this branch ignores failures)

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

	int base = 500;
#if REGRESSION_LEVEL_2
	base = 2000;
#endif
	// mul truncates at `depth`; each host block carries k bits, so a modest digit
	// floor is comfortably reached without demanding full convergence.
	nrOfFailedTestCases += ReportTestResult(VerifyExactAddSub<double>("double", reportTestCases, base), "elreal<double> add/sub exact vs dyadic", "add/sub");
	nrOfFailedTestCases += ReportTestResult(VerifyExactAddSub<float>("float", reportTestCases, base), "elreal<float> add/sub exact vs dyadic", "add/sub");
	nrOfFailedTestCases += ReportTestResult(VerifyMulAgreement<double>("double", reportTestCases, base, 24, 40), "elreal<double> mul agreement vs dyadic", "mul");
	nrOfFailedTestCases += ReportTestResult(VerifyMulAgreement<float>("float", reportTestCases, base, 24, 30), "elreal<float> mul agreement vs dyadic", "mul");
	nrOfFailedTestCases += ReportTestResult(VerifyCancellationExact<double>("double", reportTestCases), "elreal<double> cancellation-stressed accumulation is exact", "cancellation");
	{
		// operator* is depth-bounded; the default precision truncates the determinant
		elreal_precision_guard predicateDepth(32);
		nrOfFailedTestCases += ReportTestResult(VerifyOrient2dExactSign(reportTestCases, 24), "elreal<double> orient2d sign is exact", "predicates");
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
	std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
