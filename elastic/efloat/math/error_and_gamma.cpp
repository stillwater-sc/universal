// error_and_gamma.cpp: regression tests for efloat mathematical error and gamma functions.
//
// Categories tested:
//   - erf, erfc, tgamma, lgamma
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
	nrOfFailedTestCases += ReportTestResult(VerifyGammaSpecialValues(reportTestCases), "efloat", "gamma special values");
	nrOfFailedTestCases += ReportTestResult(VerifyGammaFactorials<4>(reportTestCases, 128, 120), "efloat<4>", "gamma at the integers");
	nrOfFailedTestCases += ReportTestResult(VerifyGammaHalfIntegers<4>(reportTestCases, 128, 40), "efloat<4>", "gamma at the half-integers");
	nrOfFailedTestCases += ReportTestResult(VerifyGammaNearZeros<4>(reportTestCases, 128), "efloat<4>", "lgamma near 1 and 2");
	nrOfFailedTestCases += ReportTestResult(VerifyGammaIdentities<4>(reportTestCases, 128, 60), "efloat<4>", "gamma identities");
	nrOfFailedTestCases += ReportTestResult(VerifyGammaWorkingBounds<4>(reportTestCases, 128), "efloat<4>", "tgamma error bound");
	nrOfFailedTestCases += ReportTestResult(VerifyGammaBeyondDouble(reportTestCases), "efloat<4>", "gamma beyond double");
#	endif

#	if REGRESSION_LEVEL_2
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
