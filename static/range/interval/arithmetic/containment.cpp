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
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	// higher-precision (long double) evaluation of a scalar operation, used as the
	// containment reference. Operands are the exact double endpoints.
	enum class Op { add, sub, mul, div };
	inline long double apply(Op op, long double x, long double y) {
		switch (op) {
		case Op::add: return x + y;
		case Op::sub: return x - y;
		case Op::mul: return x * y;
		default:      return x / y;
		}
	}
	template<typename Scalar>
	inline interval<Scalar> apply(Op op, const interval<Scalar>& X, const interval<Scalar>& Y) {
		switch (op) {
		case Op::add: return X + Y;
		case Op::sub: return X - Y;
		case Op::mul: return X * Y;
		default:      return X / Y;
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
		{ I r = I(1.0) / I(3.0); check("1/3", r, 1.0L/3.0L); }
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
		// OVERFLOW: 1e308 + 1e308 = 2e308 exceeds double max. The EFT residual is non-finite,
		// so tightening must fall back to outward rounding -- the enclosure is [maxfinite, +inf]
		// and must still contain the finite real 2e308 (#1248, regression for the EFT overflow
		// containment bug: without the isfinite guard the sum was [+inf, +inf]).
		{ double big = 1e308; I s = I(big) + I(big);
		  check("1e308+1e308 overflow", s, 2.0e308L);
		  if (!(std::isinf((double)s.hi()) && std::isfinite((double)s.lo()))) {
			++fails; if (reportTestCases) std::cout << "    FAIL overflow enclosure not [finite,+inf]: ["
				<< s.lo() << ", " << s.hi() << "]\n"; } }
		{ double big = 1e308; I p = I(big) * I(big);   // 1e616 overflow on the product
		  check("1e308*1e308 overflow", p, 1.0e616L);
		  if (!(std::isinf((double)p.hi()) && std::isfinite((double)p.lo()))) {
			++fails; if (reportTestCases) std::cout << "    FAIL overflow product not [finite,+inf]: ["
				<< p.lo() << ", " << p.hi() << "]\n"; } }
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
		const Op ops[] = { Op::add, Op::sub, Op::mul, Op::div };
		for (int t = 0; t < nrTests; ++t) {
			double a = U(rng), b = U(rng); if (a > b) std::swap(a, b);
			double c = U(rng), d = U(rng); if (c > d) std::swap(c, d);
			I X(a, b), Y(c, d);
			for (Op op : ops) {
				if (op == Op::div && c <= 0.0 && d >= 0.0) continue;   // denominator straddles 0 -> unbounded
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

	// ---- 2b. containment fuzz for the Stage-1 FALLBACK types --------------------
	// The EFT tight path (#1247) is enabled for native floats ONLY. Universal fixed-size
	// types keep Stage-1 unconditional outward rounding because their EFT roundoff term
	// is not guaranteed representable -- cfloat WITHOUT subnormals (the default) flushes
	// it to zero, which would silently break containment if EFT were (mis)enabled. This
	// fuzz PINS that the fallback keeps the Fundamental Theorem for those exact types, so
	// a future change that routes them through EFT without proving representability fails.
	template<typename Scalar>
	int VerifyContainmentFuzzT(const char* tag, bool reportTestCases, int nrTests, uint64_t seed) {
		using I = interval<Scalar>;
		int fails = 0;
		std::mt19937_64 rng(seed);
		std::uniform_real_distribution<double> U(-8.0, 8.0);
		std::uniform_real_distribution<double> Sd(0.0, 1.0);
		const Op ops[] = { Op::add, Op::sub, Op::mul, Op::div };
		for (int t = 0; t < nrTests; ++t) {
			Scalar a(U(rng)), b(U(rng)); if (double(a) > double(b)) std::swap(a, b);
			Scalar c(U(rng)), d(U(rng)); if (double(c) > double(d)) std::swap(c, d);
			I X(a, b), Y(c, d);
			long double alo = (long double)double(X.lo()), ahi = (long double)double(X.hi());
			long double clo = (long double)double(Y.lo()), chi = (long double)double(Y.hi());
			for (Op op : ops) {
				if (op == Op::div && double(Y.lo()) <= 0.0 && double(Y.hi()) >= 0.0) continue;   // straddles 0
				I R = apply(op, X, Y);
				long double Rlo = (long double)double(R.lo()), Rhi = (long double)double(R.hi());
				for (double sx : {0.0, 1.0, Sd(rng)}) {
					for (double sy : {0.0, 1.0, Sd(rng)}) {
						long double x = alo + (long double)sx * (ahi - alo);
						long double y = clo + (long double)sy * (chi - clo);
						long double r = apply(op, x, y);
						if (!(Rlo <= r && r <= Rhi)) {
							++fails;
							if (reportTestCases && fails < 10) std::cout << "    FAIL " << tag << " containment: point "
								<< (double)r << " not in [" << R.lo() << ", " << R.hi() << "]\n";
						}
					}
				}
			}
		}
		return fails;
	}

	// ---- 3. tightness: the enclosure must be 1-ulp optimal (Stage 2, #1247) -----
	// the OPTIMAL directed-rounding enclosure of the true real range [tlo, thi] is
	// [down(tlo), up(thi)] -- the tightest representable interval that still contains
	// the range. EFT (TwoSum/TwoProduct) achieves this to within 1 ulp; Stage-1
	// unconditional outward rounding widened EXACT corners too, so it could not. We
	// assert each computed endpoint is within 1 ulp of the optimal endpoint.
	int VerifyTightness(bool reportTestCases, int nrTests, uint64_t seed) {
		using I = interval<double>;
		int fails = 0;
		std::mt19937_64 rng(seed);
		std::uniform_real_distribution<double> U(-10.0, 10.0);
		// division is excluded: it is reciprocal-then-multiply (two roundings), so it is
		// not single-operation 1-ulp optimal -- its soundness is covered by the fuzz above.
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
				// optimal enclosure: largest double <= tlo, smallest double >= thi
				double olo = (double)tlo; if ((long double)olo > tlo) olo = std::nextafter(olo, -INFINITY);
				double ohi = (double)thi; if ((long double)ohi < thi) ohi = std::nextafter(ohi, +INFINITY);
				// Compare ENDPOINTS to the optimum (comparing widths would fold in the
				// rounding of R.width()'s own double subtraction). EFT must land each
				// endpoint on the optimum, tolerating at most 1 ulp of extra outward slack.
				auto ulpOf = [](double x) {
					double m = std::abs(x); if (!(m > 0.0)) m = std::ldexp(1.0, -1000);
					return std::ldexp(1.0, std::ilogb(m) - 52);
				};
				bool tightLo = (double)R.lo() >= olo - ulpOf(olo);   // not more than 1 ulp below optimum
				bool tightHi = (double)R.hi() <= ohi + ulpOf(ohi);   // not more than 1 ulp above optimum
				if (!tightLo || !tightHi) {
					++fails;
					if (reportTestCases && fails < 10) std::cout << "    FAIL tightness: [" << R.lo() << ", " << R.hi()
						<< "] vs optimal [" << olo << ", " << ohi << "]\n";
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
		"randomized containment fuzz (+ - * /)", "containment");
	// Stage-1 fallback types: EFT is NOT enabled for them; containment must still hold.
	nrOfFailedTestCases += ReportTestResult(
		VerifyContainmentFuzzT<cfloat<16, 5, std::uint16_t>>("cfloat<16,5>", reportTestCases, base / 4, 0xBADC0FFE),
		"containment fuzz, Stage-1 fallback cfloat<16,5> (no subnormals)", "containment");
	nrOfFailedTestCases += ReportTestResult(
		VerifyContainmentFuzzT<posit<32, 2>>("posit<32,2>", reportTestCases, base / 4, 0xF00DBEEF),
		"containment fuzz, Stage-1 fallback posit<32,2>", "containment");
	nrOfFailedTestCases += ReportTestResult(VerifyTightness(reportTestCases, base, 0xCAFED00D),
		"tightness bound (1-ulp optimal, Stage 2)", "containment");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
	std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
