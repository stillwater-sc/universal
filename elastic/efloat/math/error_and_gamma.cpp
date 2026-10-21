// error_and_gamma.cpp: regression tests for efloat mathematical error and gamma functions.
//
// Categories tested:
//   - erf, erfc, tgamma, lgamma
//   - erf and erfc correctly rounded in every mode (#1763), against the alternating Maclaurin series
//     carried with the bits it cancels, an evaluation independent of the positive-term series and
//     the asymptotic series they use
//   - tgamma and lgamma correctly rounded in every mode (#1763): at the integers against the exact
//     factorial, at the half-integers against (2n)! sqrt(pi) / (4^n n!), near the zeros of lgamma
//     at 1 and 2, across the reflection formula, and far beyond the range of double
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <limits>
#include <random>
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	// Helper to check if two values are close
	template<unsigned nlimbs>
	bool IsClose(const sw::universal::efloat<nlimbs>& a, double target_val, double tolerance = 1e-12) {
		double diff = std::abs(double(a) - target_val);
		return diff <= tolerance;
	}

	int VerifyEfloatErrorAndGamma(bool reportTestCases) {
		using namespace sw::universal;
		int failures = 0;

		// ---------------------------------------------------------------------
		// 1. Error Function (erf) Validation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying Error Function (erf)...\n";
		{
			efloat<4> zero(0.0);
			if (erf(zero) != 0.0) {
				if (reportTestCases) std::cout << "    FAIL: erf(0.0) is not 0.0\n";
				++failures;
			}

			efloat<4> one(1.0);
			double expected_erf1 = std::erf(1.0);
			if (!IsClose(erf(one), expected_erf1)) {
				if (reportTestCases) std::cout << "    FAIL: erf(1.0) is inaccurate. Result: " << double(erf(one)) << "\n";
				++failures;
			}
		}

		// ---------------------------------------------------------------------
		// 2. Complementary Error Function (erfc) Validation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying Complementary Error Function (erfc)...\n";
		{
			efloat<4> zero(0.0);
			if (erfc(zero) != 1.0) {
				if (reportTestCases) std::cout << "    FAIL: erfc(0.0) is not 1.0\n";
				++failures;
			}

			efloat<4> one(1.0);
			double expected_erfc1 = std::erfc(1.0);
			if (!IsClose(erfc(one), expected_erfc1)) {
				if (reportTestCases) std::cout << "    FAIL: erfc(1.0) is inaccurate. Result: " << double(erfc(one)) << "\n";
				++failures;
			}
		}

		// ---------------------------------------------------------------------
		// 3. Gamma Function (tgamma) Validation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying Gamma Function (tgamma)...\n";
		{
			efloat<4> four(4.0);
			if (!IsClose(tgamma(four), 6.0)) {
				if (reportTestCases) std::cout << "    FAIL: tgamma(4.0) is not 6.0. Result: " << double(tgamma(four)) << "\n";
				++failures;
			}

			efloat<4> half(0.5);
			double expected_gamma_half = std::tgamma(0.5); // should be sqrt(pi) approx. 1.77245...
			if (!IsClose(tgamma(half), expected_gamma_half)) {
				if (reportTestCases) std::cout << "    FAIL: tgamma(0.5) is inaccurate\n";
				++failures;
			}
		}

		// ---------------------------------------------------------------------
		// 4. Log-gamma Function (lgamma) Validation
		// ---------------------------------------------------------------------
		if (reportTestCases) std::cout << "  Verifying Log-gamma Function (lgamma)...\n";
		{
			efloat<4> four(4.0);
			double expected_lgamma4 = std::lgamma(4.0); // should be ln(6) approx. 1.79175...
			if (!IsClose(lgamma(four), expected_lgamma4)) {
				if (reportTestCases) std::cout << "    FAIL: lgamma(4.0) is inaccurate. Result: " << double(lgamma(four)) << "\n";
				++failures;
			}
		}

		return failures;
	}

	using sw::universal::RoundingMode;
	constexpr RoundingMode all_modes[] = { RoundingMode::RoundToNearest, RoundingMode::RoundToZero,
	                                       RoundingMode::RoundTowardPositive, RoundingMode::RoundTowardNegative };

	// reference rounded to P bits in the active mode, from a value correct to many more bits
	template<unsigned nlimbs>
	sw::universal::efloat<nlimbs> rounded(const sw::universal::efloat<nlimbs>& wide, unsigned P) {
		sw::universal::efloat<nlimbs> r(wide);
		r.set_precision(P);
		return r;
	}

	template<unsigned nlimbs>
	bool same(const sw::universal::efloat<nlimbs>& a, const sw::universal::efloat<nlimbs>& b) {
		return a == b && a.get_precision() == b.get_precision() && a.sign() == b.sign();
	}

	// erf x = 2 / sqrt(pi) sum_n (-1)^n x^(2n+1) / (n! (2n + 1)) at `bits` bits past the
	// x^2 log2(e) bits its terms, up to ~e^(x^2), cancel
	template<unsigned nlimbs>
	sw::universal::efloat<nlimbs> reference_erf(const sw::universal::efloat<nlimbs>& x, unsigned bits) {
		using namespace sw::universal;
		using E = efloat<nlimbs>;
		const double x2d = double(x) * double(x);
		const unsigned W = bits + static_cast<unsigned>(x2d * 1.4426950408889634) + 32u;
		E xw(x);
		xw.set_precision(W);
		const E m = -(xw * xw);
		E power = xw;   // (-1)^n x^(2n+1) / n!
		E sum = xw;
		for (int n = 1; ; ++n) {
			power *= m;
			power /= n;
			const E term = power / (2 * n + 1);
			sum += term;
			if (n > x2d && term.scale() < -static_cast<int64_t>(W) - 4) break;
		}
		E two(2.0);
		two.set_precision(W);
		return sum * two / sqrt(efloat_detail::pi<nlimbs>(W));
	}

	// erf and erfc at P bits in every mode against the reference rounded to P bits; erfc x as
	// 1 - erf x with the reference carried past the bits that cancel
	template<unsigned nlimbs>
	int VerifyErfRounding(bool reportTestCases, unsigned P, double xmax, int count) {
		using namespace sw::universal;
		using E = efloat<nlimbs>;
		int failures = 0;
		const RoundingMode saved = efloat_rounding_mode;
		std::mt19937_64 rng(1779);
		for (int i = 0; i < count; ++i) {
			E x(xmax * (static_cast<double>(rng() % 20001) - 10000.0) / 10000.0);
			x.set_precision(P);
			E third(1.0);
			third.set_precision(P);
			x += third / 3 * E(std::ldexp(1.0, -20));   // a full-width x
			if (x.iszero()) continue;
			efloat_rounding_mode = RoundingMode::RoundToNearest;
			const double x2d = double(x) * double(x);
			// carried past erfc |x| ~ 2^-(x^2 log2 e), the gap that separates erf x from +/-1
			const E ref_erf = reference_erf(x, P + 128u + static_cast<unsigned>(x2d * 1.4426950408889634));
			const E one(1.0);
			const E ref_erfc = one - ref_erf;
			// the unrounded values keep the error bound Ziv's test relies on
			const int64_t guard = static_cast<int64_t>(P + efloat_detail::exp_log_guard_bits) - efloat_detail::erf_error_bits;
			const E y = efloat_detail::erfc_working(x, P), dy = y - ref_erfc;
			bool bounded = dy.iszero() || dy.scale() <= y.scale() - guard;
			if (!efloat_detail::erfc_negligible(x, P)) {
				const E z = efloat_detail::erf_working(x, P), dz = z - ref_erf;
				bounded = bounded && (dz.iszero() || dz.scale() <= z.scale() - guard);
			}
			if (!bounded) {
				if (reportTestCases && failures < 8) std::cout << "    FAIL: erf/erfc_working(" << double(x) << ") outside its error bound\n";
				++failures;
			}
			for (RoundingMode mode : all_modes) {
				efloat_rounding_mode = mode;
				if (!same(erf(x), rounded(ref_erf, P)) || !same(erfc(x), rounded(ref_erfc, P))) {
					if (reportTestCases && failures < 8) std::cout << "    FAIL: erf/erfc(" << double(x) << ") at " << P << " bits, mode " << int(mode) << '\n';
					++failures;
				}
			}
		}
		efloat_rounding_mode = saved;
		return failures;
	}

	// erf and erfc at their special values, at the switches where erf rounds as +/-1 and erfc of
	// a negative x as 2, at a tiny x, and where erfc leaves exp's range
	int VerifyErfSpecialValues(bool reportTestCases) {
		using namespace sw::universal;
		using E = efloat<4>;
		int failures = 0;
		auto check = [&](bool ok, const char* what) {
			if (!ok) {
				if (reportTestCases) std::cout << "    FAIL: " << what << '\n';
				++failures;
			}
		};
		E pinf; pinf.setinf(false);
		E ninf; ninf.setinf(true);
		E nzero(0.0); nzero = -nzero;
		check(erf(E(0.0)).iszero() && erf(E(0.0)).sign() == 1 && erf(nzero).iszero() && erf(nzero).sign() == -1, "erf(+/-0) = +/-0");
		check(erf(pinf) == E(1.0) && erf(ninf) == E(-1.0), "erf(+/-inf) = +/-1");
		check(erfc(E(0.0)) == E(1.0) && erfc(nzero) == E(1.0), "erfc(+/-0) = 1");
		check(erfc(pinf).iszero() && erfc(pinf).sign() == 1 && erfc(ninf) == E(2.0), "erfc(+inf) = +0, erfc(-inf) = 2");
		check(erf(E(std::nan(""))).isnan() && erfc(E(std::nan(""))).isnan(), "erf, erfc of nan");

		// erf(12) = 1 - 1.4e-64 at 128 bits: 1 to nearest, the predecessor of 1 toward 0
		const RoundingMode saved = efloat_rounding_mode;
		E x(12.0);
		x.set_precision(128);
		E below_one(1.0);
		below_one.set_precision(128);
		below_one = nextafter(below_one, E(0.0));
		E below_two(2.0);
		below_two.set_precision(128);
		below_two = nextafter(below_two, E(0.0));
		efloat_rounding_mode = RoundingMode::RoundToNearest;
		check(erf(x) == E(1.0) && erf(-x) == E(-1.0) && erfc(-x) == E(2.0), "erf(+/-12) = +/-1, erfc(-12) = 2 to nearest");
		efloat_rounding_mode = RoundingMode::RoundToZero;
		check(erf(x) == below_one && erf(-x) == -below_one && erfc(-x) == below_two, "erf(+/-12), erfc(-12) toward 0");
		efloat_rounding_mode = saved;

		// erf x = 2 x / sqrt(pi) (1 - x^2 / 3 + ...) at x = 2^-200
		E tiny(1.0);
		tiny.set_precision(128);
		tiny.setexponent(-200);
		E lead = tiny * 2 / sqrt(efloat_detail::pi<4>(256));
		lead.set_precision(128);
		check(erf(tiny) == lead && erf(-tiny) == -lead && erfc(tiny) == E(1.0), "erf(2^-200) = 2^-199 / sqrt(pi)");

		// erfc(x) ~ exp(-x^2) / (x sqrt(pi)) (1 - 1 / (2 x^2)) far past double, and 0 past exp's range
		E big(1000.5);
		big.set_precision(128);
		const E lead_big = exp(-(big * big)) / (big * sqrt(efloat_detail::pi<4>(128))) * (1 - 1 / (2 * big * big) + 3 / (4 * big * big * big * big));
		const E ratio = erfc(big) / lead_big - 1;
		check(!erfc(big).iszero() && (ratio.iszero() || ratio.scale() < -55), "erfc(1000.5) ~ 2^-1444231");
		E huge(1.0);
		huge.setexponent(32);
		clear_efloat_exceptions();
		E z = erfc(huge);
		check(z.iszero() && z.sign() == 1 && has_efloat_exception(ExceptionFlag::Underflow), "erfc(2^32) = +0, underflow");
		return failures;
	}

	// special values, poles and their exception flags
	int VerifyGammaSpecialValues(bool reportTestCases) {
		using namespace sw::universal;
		using E = efloat<4>;
		int failures = 0;
		auto check = [&](bool ok, const char* what) {
			if (!ok) {
				if (reportTestCases) std::cout << "    FAIL: " << what << '\n';
				++failures;
			}
		};
		E pinf; pinf.setinf(false);
		E ninf; ninf.setinf(true);
		E nzero(0.0); nzero = -nzero;
		clear_efloat_exceptions();
		E g = tgamma(E(0.0));
		check(g.isinf() && g.sign() == 1 && has_efloat_exception(ExceptionFlag::DivisionByZero), "tgamma(+0) = +inf, division by zero");
		g = tgamma(nzero);
		check(g.isinf() && g.sign() == -1, "tgamma(-0) = -inf");
		clear_efloat_exceptions();
		check(tgamma(E(-3.0)).isnan() && has_efloat_exception(ExceptionFlag::InvalidOperation), "tgamma(-3) = nan, invalid");
		check(tgamma(ninf).isnan(), "tgamma(-inf) = nan");
		check(tgamma(pinf).isinf() && tgamma(pinf).sign() == 1, "tgamma(+inf) = +inf");
		clear_efloat_exceptions();
		E l = lgamma(E(-3.0));
		check(l.isinf() && l.sign() == 1 && has_efloat_exception(ExceptionFlag::DivisionByZero), "lgamma(-3) = +inf, division by zero");
		check(lgamma(E(0.0)).isinf() && lgamma(nzero).isinf() && lgamma(nzero).sign() == 1, "lgamma(+/-0) = +inf");
		check(lgamma(ninf).isinf() && lgamma(ninf).sign() == 1 && lgamma(pinf).isinf(), "lgamma(+/-inf) = +inf");
		check(lgamma(E(1.0)).iszero() && lgamma(E(2.0)).iszero() && lgamma(E(2.0)).sign() == 1, "lgamma(1) = lgamma(2) = +0");
		check(tgamma(E(1.0)) == E(1.0) && tgamma(E(2.0)) == E(1.0) && tgamma(E(5.0)) == E(24.0), "tgamma at 1, 2, 5");
		// gamma x < 0 on (-1, 0), (-3, -2), ...; > 0 on (-2, -1), ...
		check(tgamma(E(-0.5)).sign() == -1 && tgamma(E(-1.5)).sign() == 1 && tgamma(E(-2.5)).sign() == -1, "tgamma signs below 0");
		return failures;
	}

	// tgamma(n) = (n - 1)! and lgamma(n) = log (n - 1)!, n = 1 .. nmax, in every mode: past twice the
	// result's precision the factorial no longer takes the exact path, and Ziv's strategy rounds it
	template<unsigned nlimbs>
	int VerifyGammaFactorials(bool reportTestCases, unsigned P, int nmax) {
		using namespace sw::universal;
		using E = efloat<nlimbs>;
		int failures = 0;
		const RoundingMode saved = efloat_rounding_mode;
		E factorial(1.0);
		factorial.set_precision(32u * 1024u);   // 1000! has 8530 bits
		for (int n = 1; n <= nmax; ++n) {
			if (n > 1) factorial *= n - 1;
			E x(static_cast<double>(n));
			x.set_precision(P);
			efloat_rounding_mode = RoundingMode::RoundToNearest;
			const E log_wide = log(rounded(factorial, P + 128u));
			for (RoundingMode mode : all_modes) {
				efloat_rounding_mode = mode;
				const E g = tgamma(x), gref = rounded(factorial, P);
				const E l = lgamma(x), lref = rounded(log_wide, P);
				if (!same(g, gref) || (n > 2 && !same(l, lref))) {
					if (reportTestCases && failures < 8) std::cout << "    FAIL: tgamma/lgamma(" << n << ") at " << P << " bits, mode " << int(mode) << '\n';
					++failures;
				}
			}
		}
		efloat_rounding_mode = saved;
		return failures;
	}

	// gamma(n + 1/2) = (2n)! sqrt(pi) / (4^n n!) and gamma(1/2 - n) = (-4)^n n! sqrt(pi) / (2n)!
	template<unsigned nlimbs>
	int VerifyGammaHalfIntegers(bool reportTestCases, unsigned P, int nmax) {
		using namespace sw::universal;
		using E = efloat<nlimbs>;
		int failures = 0;
		const RoundingMode saved = efloat_rounding_mode;
		efloat_rounding_mode = RoundingMode::RoundToNearest;
		const unsigned W = P + 128u;
		const E root_pi = sqrt(efloat_detail::pi<nlimbs>(W));
		for (int n = 0; n <= nmax; ++n) {
			E num(1.0), den(1.0);   // (2n)! and 4^n n!, exactly
			num.set_precision(8192u);
			den.set_precision(8192u);
			for (int j = 2; j <= 2 * n; ++j) num *= j;
			for (int j = 2; j <= n; ++j) den *= j;
			den.setexponent(den.scale() + 2 * n);
			efloat_rounding_mode = RoundingMode::RoundToNearest;
			E up = num * root_pi;
			up.set_precision(W);
			up /= den;
			E down = den * root_pi;
			down.set_precision(W);
			down /= num;
			if (n % 2 == 1) down = -down;
			E xu(n + 0.5), xd(0.5 - n);
			xu.set_precision(P);
			xd.set_precision(P);
			for (RoundingMode mode : all_modes) {
				efloat_rounding_mode = mode;
				if (!same(tgamma(xu), rounded(up, P)) || !same(tgamma(xd), rounded(down, P))) {
					if (reportTestCases && failures < 8) std::cout << "    FAIL: tgamma(+/-(" << n << " + 1/2)) at " << P << " bits, mode " << int(mode) << '\n';
					++failures;
				}
				E lup = up, ldown = down;
				efloat_rounding_mode = RoundingMode::RoundToNearest;
				if (ldown.sign() == -1) ldown = -ldown;
				lup = log(lup);
				ldown = log(ldown);
				efloat_rounding_mode = mode;
				// lgamma(1/2) and lgamma(3/2) are near 0.57 and -0.12, so a reference at W bits holds
				if (!same(lgamma(xu), rounded(lup, P)) || !same(lgamma(xd), rounded(ldown, P))) {
					if (reportTestCases && failures < 8) std::cout << "    FAIL: lgamma(+/-(" << n << " + 1/2)) at " << P << " bits, mode " << int(mode) << '\n';
					++failures;
				}
			}
		}
		efloat_rounding_mode = saved;
		return failures;
	}

	// lgamma near its zeros at 1 and 2, where lgamma(1 + e) = -gamma e + (pi^2 / 12) e^2 - ... and
	// lgamma(2 + e) = (1 - gamma) e + (pi^2 / 12 - 1/2) e^2 - ...: the shift's log cancels all but
	// the e-sized remainder, and the working precision is raised by what cancels
	template<unsigned nlimbs>
	int VerifyGammaNearZeros(bool reportTestCases, unsigned P) {
		using namespace sw::universal;
		using E = efloat<nlimbs>;
		constexpr double euler = 0.57721566490153286;
		int failures = 0;
		for (unsigned k = 60; k < P; k += (P - 60) / 7 + 1) {
			for (int side = -1; side <= 1; side += 2) {
				E e(1.0);
				e.set_precision(P);
				e.setexponent(-static_cast<int64_t>(k));
				if (side < 0) e = -e;
				E x1 = E(1.0) + e, x2 = E(2.0) + e;
				x1.set_precision(P);
				x2.set_precision(P);
				const E r1 = lgamma(x1) / e, r2 = lgamma(x2) / e;   // -gamma and 1 - gamma, within 2^-k
				if (std::fabs(double(r1) + euler) > 1e-15 || std::fabs(double(r2) - (1.0 - euler)) > 1e-15) {
					if (reportTestCases && failures < 8) std::cout << "    FAIL: lgamma(1 or 2 + " << side << " 2^-" << k << ") / e = " << double(r1) << ", " << double(r2) << '\n';
					++failures;
				}
				// the unrounded value keeps the error bound Ziv's test relies on, rather than leaving
				// the cancelled bits as zeros for the test to retry on
				for (const E& x : { x1, x2 }) {
					const E y = efloat_detail::lgamma_working(x, P);
					E xw(x);
					xw.set_precision(P + 256u);
					const E diff = y - lgamma(xw);
					if (!diff.iszero() && diff.scale() > y.scale() - static_cast<int64_t>(P + efloat_detail::exp_log_guard_bits)) {
						if (reportTestCases && failures < 8) std::cout << "    FAIL: lgamma_working(1 or 2 + " << side << " 2^-" << k << ") outside its error bound\n";
						++failures;
					}
				}
			}
		}
		return failures;
	}

	// tgamma's unrounded value keeps the error bound Ziv's test relies on where lgamma is large:
	// exp turns lgamma's absolute error into the result's relative error, so the working precision
	// carries the scale E of lgamma's largest part, (z - 1/2) log z at a large x and the log of the
	// shift's product at a tiny one
	template<unsigned nlimbs>
	int VerifyGammaWorkingBounds(bool reportTestCases, unsigned P) {
		using namespace sw::universal;
		using E = efloat<nlimbs>;
		int failures = 0;
		E large(1.37);
		large.set_precision(P);
		large.setexponent(51);   // lgamma ~ 2^56.2
		E tiny(1.37);
		tiny.set_precision(P);
		tiny.setexponent(-(int64_t(1) << 58));   // lgamma ~ -log x ~ 2^57.5
		for (const E& x : { large, tiny }) {
			const E y = efloat_detail::tgamma_working(x, P);
			E xw(x);
			xw.set_precision(P + 256u);
			const E diff = y - tgamma(xw);
			const int64_t bound = y.scale() - static_cast<int64_t>(P + efloat_detail::exp_log_guard_bits) + efloat_detail::exp_error_bits + 1;
			if (!diff.iszero() && diff.scale() > bound) {
				if (reportTestCases) std::cout << "    FAIL: tgamma_working at scale " << x.scale() << " outside its error bound\n";
				++failures;
			}
		}
		return failures;
	}

	// gamma(x) gamma(1 - x) = pi / sin(pi x) and lgamma(x + 1) = lgamma(x) + log |x|, at x = j / 97
	template<unsigned nlimbs>
	int VerifyGammaIdentities(bool reportTestCases, unsigned P, int count) {
		using namespace sw::universal;
		using E = efloat<nlimbs>;
		int failures = 0;
		std::mt19937_64 rng(1763);
		const E pi = efloat_detail::pi<nlimbs>(P + 64);
		for (int i = 0; i < count; ++i) {
			const int j = static_cast<int>(rng() % 4001) - 2000;
			if (j % 97 == 0) continue;
			E x(static_cast<double>(j));
			x.set_precision(P);
			x /= 97;
			E one(1.0);
			one.set_precision(P + 16);   // 1 - x and x + 1 exact
			const E product = tgamma(x) * tgamma(one - x) * sin(pi * x) / pi;
			const E step = lgamma(x + one) - lgamma(x) - log(abs(x));
			const E big = abs(lgamma(x + one)) + abs(lgamma(x)) + abs(log(abs(x)));
			if ((product - one).scale() > -static_cast<int64_t>(P) + 10 || (!step.iszero() && step.scale() > big.scale() - static_cast<int64_t>(P) + 4)) {
				if (reportTestCases && failures < 8) std::cout << "    FAIL: identities at x = " << j << " / 97, " << P << " bits\n";
				++failures;
			}
		}
		return failures;
	}

	// results far beyond double: gamma(171.7) and gamma(100001.5) overflow it, gamma(-200.5)
	// underflows it, and only the exponent's own range ends them
	int VerifyGammaBeyondDouble(bool reportTestCases) {
		using namespace sw::universal;
		using E = efloat<4>;
		int failures = 0;
		auto check = [&](bool ok, const char* what) {
			if (!ok) {
				if (reportTestCases) std::cout << "    FAIL: " << what << '\n';
				++failures;
			}
		};
		auto agrees = [](const E& g, const E& l) {   // g = exp(l) to within 2^-120
			const E ratio = log(abs(g)) - l;
			return !g.iszero() && !g.isinf() && (ratio.iszero() || ratio.scale() < -120 + std::max<int64_t>(0, l.scale()));
		};
		const double xs[] = { 171.7, 1000.5, 100001.5, -200.5, -1000.25 };
		for (double d : xs) {
			E x(d);
			x.set_precision(128);
			check(agrees(tgamma(x), lgamma(x)), "tgamma = exp(lgamma) beyond double");
		}
		check(tgamma(E(-200.5)).sign() == -1 && tgamma(E(-1001.25)).sign() == 1, "tgamma signs far below 0");
		E huge(1.0);
		huge.setexponent(60);
		clear_efloat_exceptions();
		E g = tgamma(huge);
		check(g.isinf() && g.sign() == 1 && has_efloat_exception(ExceptionFlag::Overflow), "tgamma(2^60) = +inf, overflow");
		E l = lgamma(huge);   // 2^60 (60 ln 2 - 1) + ...
		check(!l.isinf() && l.scale() == 65, "lgamma(2^60) finite");
		E tiny(-1.0);
		tiny.set_precision(128);
		tiny.setexponent(57);
		tiny += 0.5;   // -2^57 + 1/2, on (-2^57, -2^57 + 1), where gamma > 0; lgamma ~ -2^62.3
		clear_efloat_exceptions();
		g = tgamma(tiny);
		check(g.iszero() && g.sign() == 1 && has_efloat_exception(ExceptionFlag::Underflow), "tgamma(-2^57 + 1/2) = +0, underflow");
		return failures;
	}

}  // anonymous namespace

#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 0
#	define REGRESSION_LEVEL_3 0
#	define REGRESSION_LEVEL_4 0
#endif

int main() try {
	using namespace sw::universal;

	std::string test_suite          = "efloat mathematical error and gamma library";
	std::string test_tag            = "error_and_gamma";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyEfloatErrorAndGamma(reportTestCases), "efloat", "error_and_gamma manual");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyEfloatErrorAndGamma(reportTestCases), "efloat", "error_and_gamma foundational");
	nrOfFailedTestCases += ReportTestResult(VerifyErfSpecialValues(reportTestCases), "efloat", "erf special values");
	nrOfFailedTestCases += ReportTestResult(VerifyErfRounding<4>(reportTestCases, 128, 4.0, 120), "efloat<4>", "erf, erfc for |x| < 4");
	nrOfFailedTestCases += ReportTestResult(VerifyErfRounding<4>(reportTestCases, 128, 16.0, 40), "efloat<4>", "erf, erfc for |x| < 16");
	nrOfFailedTestCases += ReportTestResult(VerifyGammaSpecialValues(reportTestCases), "efloat", "gamma special values");
	nrOfFailedTestCases += ReportTestResult(VerifyGammaFactorials<4>(reportTestCases, 128, 120), "efloat<4>", "gamma at the integers");
	nrOfFailedTestCases += ReportTestResult(VerifyGammaHalfIntegers<4>(reportTestCases, 128, 40), "efloat<4>", "gamma at the half-integers");
	nrOfFailedTestCases += ReportTestResult(VerifyGammaNearZeros<4>(reportTestCases, 128), "efloat<4>", "lgamma near 1 and 2");
	nrOfFailedTestCases += ReportTestResult(VerifyGammaIdentities<4>(reportTestCases, 128, 60), "efloat<4>", "gamma identities");
	nrOfFailedTestCases += ReportTestResult(VerifyGammaWorkingBounds<4>(reportTestCases, 128), "efloat<4>", "tgamma error bound");
	nrOfFailedTestCases += ReportTestResult(VerifyGammaBeyondDouble(reportTestCases), "efloat<4>", "gamma beyond double");
#	endif

#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyErfRounding<16>(reportTestCases, 512, 32.0, 40), "efloat<16>", "erf, erfc for |x| < 32");
	nrOfFailedTestCases += ReportTestResult(VerifyGammaFactorials<16>(reportTestCases, 512, 400), "efloat<16>", "gamma at the integers");
	nrOfFailedTestCases += ReportTestResult(VerifyGammaHalfIntegers<16>(reportTestCases, 512, 100), "efloat<16>", "gamma at the half-integers");
	nrOfFailedTestCases += ReportTestResult(VerifyGammaNearZeros<16>(reportTestCases, 512), "efloat<16>", "lgamma near 1 and 2");
	nrOfFailedTestCases += ReportTestResult(VerifyGammaIdentities<16>(reportTestCases, 512, 40), "efloat<16>", "gamma identities");
#	endif

#	if REGRESSION_LEVEL_3
#	endif

#	if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyGammaFactorials<64>(reportTestCases, 2048, 1000), "efloat<64>", "gamma at the integers");
	nrOfFailedTestCases += ReportTestResult(VerifyGammaNearZeros<64>(reportTestCases, 2048), "efloat<64>", "lgamma near 1 and 2");
#	endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif
}
catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
