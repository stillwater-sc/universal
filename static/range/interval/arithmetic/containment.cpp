// containment.cpp: the Fundamental Theorem of Interval Arithmetic must hold
//
// interval<Scalar> must round OUTWARD so that for all x in X and y in Y,
// x o y  in  fl(X o Y), evaluated over the exact reals (#1234). Before the fix,
// interval arithmetic used round-to-nearest and the computed enclosure could be
// narrower than the true range -- e.g. interval(0.1)*interval(0.1) had width 0 and
// did not contain the exact product. This suite exercises INEXACT operands (which
// the equality-on-small-integers tests structurally cannot), plus a randomized
// containment fuzz and a tightness bound so a future fix cannot regress into
// uselessly wide intervals.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <iostream>
#include <random>
#include <string>

#include <universal/number/interval/interval.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	// higher-precision (long double) evaluation of a scalar operation, used as the
	// containment reference. Operands are the exact double endpoints.
	enum class Op { add, sub, mul };
	inline long double apply(Op op, long double x, long double y) {
		switch (op) {
		case Op::add: return x + y;
		case Op::sub: return x - y;
		default:      return x * y;
		}
	}
	inline interval<double> apply(Op op, const interval<double>& X, const interval<double>& Y) {
		switch (op) {
		case Op::add: return X + Y;
		case Op::sub: return X - Y;
		default:      return X * Y;
		}
	}

	// ---- 1. deterministic containment on inexact operands ----------------------
	int VerifyInexactContainment(bool reportTestCases) {
		using I = interval<double>;
		int fails = 0;
		auto check = [&](const char* tag, const I& R, long double truth) {
			if (!((long double)R.lo() <= truth && truth <= (long double)R.hi())) {
				++fails;
				if (reportTestCases) std::cout << "    FAIL " << tag << ": " << (double)truth
					<< " not in [" << R.lo() << ", " << R.hi() << "]\n";
			}
		};
		// the issue's reproduction: 0.1 * 0.1 is inexact, must be enclosed with nonzero width
		{ double a = 0.1; I p = I(a) * I(a); check("0.1*0.1", p, (long double)a * (long double)a);
		  if (!(p.width() > 0.0)) { ++fails; if (reportTestCases) std::cout << "    FAIL 0.1*0.1 has zero width\n"; } }
		// a third, and its reciprocal
		{ double a = 1.0/3.0; I r = I(1.0) / I(3.0); check("1/3", r, 1.0L/3.0L); }
		// sqrt(2) via sqr round-trip: sqrt([2,2]) must enclose the true sqrt(2)
		{ I s = sqrt(I(2.0)); check("sqrt(2)", s, std::sqrt(2.0L)); }
		// composition: an 8-term accumulation of 0.1 must carry nonzero uncertainty
		{ I acc(0.0); for (int i = 0; i < 8; ++i) acc = acc + I(0.1);
		  long double truth = 8.0L * 0.1L;   // exact-real target for the sum of the double 0.1
		  check("sum 8x0.1", acc, truth);
		  if (!(acc.width() > 0.0)) { ++fails; if (reportTestCases) std::cout << "    FAIL 8x0.1 sum has zero width\n"; } }
		// cross-type constructor: interval<float>(double 0.1) must enclose the double 0.1
		{ interval<float> f(0.1);
		  if (!((double)f.lo() <= 0.1 && 0.1 <= (double)f.hi())) {
			++fails; if (reportTestCases) std::cout << "    FAIL interval<float>(0.1) does not contain double 0.1\n"; } }
		return fails;
	}

	// ---- 2. randomized containment fuzz ----------------------------------------
	// random X, Y; sample points x in X, y in Y; assert x o y in fl(X o Y).
	int VerifyContainmentFuzz(bool reportTestCases, int nrTests, uint64_t seed) {
		using I = interval<double>;
		int fails = 0;
		std::mt19937_64 rng(seed);
		std::uniform_real_distribution<double> U(-10.0, 10.0);
		std::uniform_real_distribution<double> S(0.0, 1.0);
		const Op ops[] = { Op::add, Op::sub, Op::mul };
		for (int t = 0; t < nrTests; ++t) {
			double a = U(rng), b = U(rng); if (a > b) std::swap(a, b);
			double c = U(rng), d = U(rng); if (c > d) std::swap(c, d);
			I X(a, b), Y(c, d);
			for (Op op : ops) {
				I R = apply(op, X, Y);
				// sample points: endpoints and a few interior points of each interval
				for (double sx : {0.0, 1.0, S(rng), S(rng)}) {
					for (double sy : {0.0, 1.0, S(rng), S(rng)}) {
						long double x = (long double)a + (long double)sx * ((long double)b - a);
						long double y = (long double)c + (long double)sy * ((long double)d - c);
						long double r = apply(op, x, y);
						if (!((long double)R.lo() <= r && r <= (long double)R.hi())) {
							++fails;
							if (reportTestCases && fails < 10) std::cout << "    FAIL fuzz: point " << (double)r
								<< " not in [" << R.lo() << ", " << R.hi() << "]\n";
						}
					}
				}
			}
		}
		return fails;
	}

	// ---- 3. tightness: the enclosure must not be uselessly wide ----------------
	// for a single operation, the optimal enclosure width is the true-range width; we
	// allow a small slack (a few ulps) so Stage-1 outward rounding passes but a future
	// regression to grossly wide intervals is caught.
	int VerifyTightness(bool reportTestCases, int nrTests, uint64_t seed) {
		using I = interval<double>;
		int fails = 0;
		std::mt19937_64 rng(seed);
		std::uniform_real_distribution<double> U(-10.0, 10.0);
		const Op ops[] = { Op::add, Op::sub, Op::mul };
		for (int t = 0; t < nrTests; ++t) {
			double a = U(rng), b = U(rng); if (a > b) std::swap(a, b);
			double c = U(rng), d = U(rng); if (c > d) std::swap(c, d);
			I X(a, b), Y(c, d);
			for (Op op : ops) {
				I R = apply(op, X, Y);
				// true range over the reals: min/max of the operation over the corners
				long double corners[4] = {
					apply(op, (long double)a, (long double)c), apply(op, (long double)a, (long double)d),
					apply(op, (long double)b, (long double)c), apply(op, (long double)b, (long double)d) };
				long double tlo = corners[0], thi = corners[0];
				for (int k = 1; k < 4; ++k) { tlo = std::min(tlo, corners[k]); thi = std::max(thi, corners[k]); }
				long double trueWidth = thi - tlo;
				long double got = (long double)R.width();
				long double ulp = std::ldexp(1.0L, std::ilogb(std::max(std::abs(tlo), std::abs(thi))) - 52);
				if (got > trueWidth + 8.0L * ulp) {   // generous slack; catches gross over-widening
					++fails;
					if (reportTestCases && fails < 10) std::cout << "    FAIL tightness: width " << (double)got
						<< " >> true " << (double)trueWidth << "\n";
				}
			}
		}
		return fails;
	}

}} // namespace sw::universal

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
	std::string test_suite = "interval containment (Fundamental Theorem of Interval Arithmetic) (#1234)";
	int nrOfFailedTestCases = 0;
	bool reportTestCases = true;
	ReportTestSuiteHeader(test_suite, reportTestCases);

	int base = 20000;
#if REGRESSION_LEVEL_2
	base = 100000;
#endif

	nrOfFailedTestCases += ReportTestResult(VerifyInexactContainment(reportTestCases),
		"containment on inexact operands (0.1*0.1, 1/3, sqrt(2), sum, cross-type)", "containment");
	nrOfFailedTestCases += ReportTestResult(VerifyContainmentFuzz(reportTestCases, base, 0x1234ABCD),
		"randomized containment fuzz (+ - *)", "containment");
	nrOfFailedTestCases += ReportTestResult(VerifyTightness(reportTestCases, base, 0xCAFED00D),
		"tightness bound (enclosure not uselessly wide)", "containment");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
	std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
