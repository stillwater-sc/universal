// error_and_gamma.cpp: regression tests for the error and gamma functions of
// ereal adaptive-precision.
//
// Standardized as part of #950 (math/functions refactor).
//
// erf, erfc, tgamma and lgamma were double-precision shims, 53 bits on every configuration,
// and their precision checks were left out rather than loosened (#2008). They are computed in
// ereal arithmetic now (#2017), and the checks are back, exact (ereal_precision.hpp): every
// value is compared with a stored decimal reference (reference_constants.hpp, mpmath) or an
// identity as a dyadic rational, never through a double, and must agree to the
// configuration's bits less a margin.
//
// Measured agreement, against the targets of 212 (ereal<4>), 424 (ereal<8>), 96 (ereal<4, float>):
//   erf, erfc  ereal<4> 255-441, ereal<8> 468-883, float limbs 102-109 bits
//   tgamma     ereal<4> 255-265, ereal<8> 471-481, float limbs 102 bits
//   lgamma     ereal<4> 262-275, ereal<8> 478-488, float limbs 99-106 bits
// A value close to the limb type's floor keeps only the bits above it (erfc(20) is near
// 2^-577), so a check asks for no more than the reference's magnitude leaves room for.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <algorithm>
#include <cmath>
#include <limits>
#include <random>
#include <string>
#include <string_view>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/ereal_precision.hpp>
#include <math/constants/reference_constants.hpp>

namespace {
	using namespace sw::universal;

	constexpr int kFunctionMargin = 16;   // a function against a stored reference
	constexpr int kIdentityMargin = 24;   // an identity through two or three functions

	// exact: the dyadic values are equal, whatever the limb structure
	template<typename Real>
	bool exactly(const Real& a, const Real& b) { return agreed_bits_absolute(a, b) == kExactAgreement; }

	int expect_bits(int got, int wanted, const std::string& what, bool reportTestCases) {
#ifdef EREAL_PRECISION_TRACE
		std::cout << "    " << what << ": " << got << " bits (wanted " << wanted << ")\n";
#endif
		if (got >= wanted) return 0;
		if (reportTestCases) std::cout << "    FAIL " << what << ": " << got << " bits, wanted at least " << wanted << '\n';
		return 1;
	}

	template<typename Real> struct limb_of;
	template<unsigned m, typename F> struct limb_of<ereal<m, F>> { using type = F; };

	// The bits a check can ask for at a value of magnitude 2^e: the configuration's, less the
	// margin, but no more than the limbs between 2^e and the limb type's floor hold. -1 where
	// the value lies outside the limb type's range, and the check is skipped.
	template<typename Real>
	int wanted_at(double magnitude, int margin) {
		using F = typename limb_of<Real>::type;
		const int e = std::ilogb(magnitude);
		if (e >= std::numeric_limits<F>::max_exponent - 1) return -1;
		const int room = e - (std::numeric_limits<F>::min_exponent - 1) + std::numeric_limits<F>::digits;
		const int wanted = std::min(ereal_bits_of<Real>::value, room) - margin;
		return (wanted < 32) ? -1 : wanted;
	}

	// bits to which v agrees with a signed decimal reference; -1 for a value that cannot agree
	template<typename Real>
	int bits_vs(const Real& v, std::string_view ref) {
		if (v.isnan() || v.isinf() || v.iszero()) return -1;
		Real w = v;
		if (!ref.empty() && ref.front() == '-') { ref.remove_prefix(1); w = -v; }
		if (w.isneg()) return -1;
		const int cap = std::min(320, static_cast<int>(ereal_bits_of<Real>::value * 0.30103) + 4);
		return agreed_bits(w, ref, cap);
	}

	// f(x) against its stored reference, where the reference lies in the limb type's range
	template<typename Real>
	int expect_ref(const Real& v, std::string_view ref, const std::string& what, bool reportTestCases) {
		const int wanted = wanted_at<Real>(std::fabs(std::stod(std::string(ref))), kFunctionMargin);
		if (wanted < 0) return 0;
		return expect_bits(bits_vs(v, ref), wanted, what, reportTestCases);
	}

	// bits to which a agrees with b, relative to b; -1 for NaN or inf
	template<typename Real>
	int bits_vs(const Real& a, const Real& b) {
		if (a.isnan() || a.isinf() || b.isnan() || b.isinf()) return -1;
		if (b.iszero()) return a.iszero() ? kExactAgreement : -1;
		return agreed_bits(a, b);
	}

	struct ErfCase { double x; std::string_view erf_ref, erfc_ref; };
	struct GammaCase { double x; std::string_view tgamma_ref, lgamma_ref; };

	const ErfCase erf_cases[] = {
		{ 0.3,  s_erf_0p3,  s_erfc_0p3 },  { 1.0,  s_erf_1,  s_erfc_1 },  { 2.5, s_erf_2p5, s_erfc_2p5 },
		{ 5.0,  s_erf_5,    s_erfc_5 },    { 10.0, s_erf_10, s_erfc_10 }, { 20.0, s_erf_20, s_erfc_20 },
		{ -1.7, s_erf_m1p7, s_erfc_m1p7 },
	};
	const GammaCase gamma_cases[] = {
		{ 0.5,    s_tgamma_0p5,    s_lgamma_0p5 },    { 2.7,   s_tgamma_2p7,   s_lgamma_2p7 },
		{ 10.5,   s_tgamma_10p5,   s_lgamma_10p5 },   { 100.25, s_tgamma_100p25, s_lgamma_100p25 },
		{ -2.5,   s_tgamma_m2p5,   s_lgamma_m2p5 },   { -0.3,  s_tgamma_m0p3,  s_lgamma_m0p3 },
		{ 1e-20,  s_tgamma_1em20,  s_lgamma_1em20 },
	};

	// erf at the edge of the limb range: erf's series sums to about exp(x^2), which would
	// overflow where x^2 log2(e) passes max_exponent before erfc's asymptotic series reaches
	// the precision (float limbs near 9.5, ereal<19> near 27). erf is 1 there to every bit
	// the configuration holds.
	template<typename Real>
	int VerifyErfRangeEdge(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		using F = typename limb_of<Real>::type;
		const double lim = std::sqrt(static_cast<double>(std::numeric_limits<F>::max_exponent) * 0.6931471805599453);
		for (double v : { lim - 0.3, lim - 0.1, lim, lim + 0.2, lim + 0.6 }) {
			const Real e = erf(Real(v));
			nrOfFailedTestCases += expect_bits(bits_vs(e, Real(1.0)), ereal_bits_of<Real>::value - kFunctionMargin, "erf at the range edge " + std::to_string(v), reportTestCases);
			const Real c = erfc(Real(v));
			if (c.isnan() || c.isinf() || c.isneg()) {
				if (reportTestCases) std::cout << "    FAIL erfc at the range edge " << v << " is not a finite nonnegative value\n";
				++nrOfFailedTestCases;
			}
		}
		return nrOfFailedTestCases;
	}

	// erf/erfc: fixed point, parity, the special values, the stored references and erf + erfc == 1
	template<typename Real>
	int VerifyErrorFunction(bool reportTestCases) {
		int nrOfFailedTestCases = 0;

		// erf(+/-0) == +/-0, erfc(0) == 1
		if (!erf(Real(0.0)).iszero() || erf(Real(0.0)).signbit() || !erf(-Real(0.0)).signbit()) {
			if (reportTestCases) std::cout << "    FAIL erf(+/-0) != +/-0\n";
			++nrOfFailedTestCases;
		}
		if (!exactly(erfc(Real(0.0)), Real(1.0))) {
			if (reportTestCases) std::cout << "    FAIL erfc(0) != 1\n";
			++nrOfFailedTestCases;
		}
		// parity: erf(-x) == -erf(x), exactly
		for (double v : { 0.3, 1.0, 2.5, 7.0 }) {
			Real x(v);
			if (!exactly(erf(-x), -erf(x))) {
				if (reportTestCases) std::cout << "    FAIL erf parity at " << v << "\n";
				++nrOfFailedTestCases;
			}
		}

		// the stored references
		for (const auto& c : erf_cases) {
			const Real x(c.x);
			const std::string at = "(" + std::to_string(c.x) + ")";
			nrOfFailedTestCases += expect_ref(erf(x), c.erf_ref, "erf" + at, reportTestCases);
			nrOfFailedTestCases += expect_ref(erfc(x), c.erfc_ref, "erfc" + at, reportTestCases);
		}

		nrOfFailedTestCases += VerifyErfRangeEdge<Real>(reportTestCases);

		// erf(x) + erfc(x) == 1, absolute
		for (double v : { 0.3, 1.0, 2.0, 2.5, -1.7, 3.0 }) {
			const Real x(v);
			const int got = agreed_bits_absolute(erf(x) + erfc(x), Real(1.0));
			nrOfFailedTestCases += expect_bits(got, ereal_bits_of<Real>::value - kIdentityMargin, "erf + erfc == 1 at " + std::to_string(v), reportTestCases);
		}

		// special values: erf(+/-Inf) = +/-1, erfc(+/-Inf) = 0/2, NaN propagates
		const double pinf = std::numeric_limits<double>::infinity();
		const double qnan = std::numeric_limits<double>::quiet_NaN();
		if (!exactly(erf(Real(pinf)), Real(1.0)) || !exactly(erf(Real(-pinf)), Real(-1.0))) {
			if (reportTestCases) std::cout << "    FAIL erf(+/-Inf) != +/-1\n";
			++nrOfFailedTestCases;
		}
		if (!erfc(Real(pinf)).iszero() || !exactly(erfc(Real(-pinf)), Real(2.0))) {
			if (reportTestCases) std::cout << "    FAIL erfc(+/-Inf) != 0/2\n";
			++nrOfFailedTestCases;
		}
		if (!erf(Real(qnan)).isnan() || !erfc(Real(qnan)).isnan()) {
			if (reportTestCases) std::cout << "    FAIL erf/erfc(NaN) not NaN\n";
			++nrOfFailedTestCases;
		}
		return nrOfFailedTestCases;
	}

	// tgamma: factorials, the special values, the stored references, and the identities
	template<typename Real>
	int VerifyTgamma(bool reportTestCases) {
		int nrOfFailedTestCases = 0;

		// tgamma(n) == (n-1)!, exactly
		double fact = 1.0;  // (n-1)! ; starts at 0!=1 for n=1
		for (int n = 1; n <= 20; ++n) {
			if (n > 1) fact *= (n - 1);
			if (!exactly(tgamma(Real(double(n))), Real(fact))) {
				if (reportTestCases) std::cout << "    FAIL tgamma(" << n << ") != " << fact << "\n";
				++nrOfFailedTestCases;
			}
		}

		// the stored references
		for (const auto& c : gamma_cases) {
			nrOfFailedTestCases += expect_ref(tgamma(Real(c.x)), c.tgamma_ref, "tgamma(" + std::to_string(c.x) + ")", reportTestCases);
		}

		const int wanted = ereal_bits_of<Real>::value - kIdentityMargin;
		// tgamma(1/2)^2 == pi, and 2 / tgamma(1/2) == 2 / sqrt(pi)
		const Real g = tgamma(Real(0.5));
		nrOfFailedTestCases += expect_bits(bits_vs(g * g, s_pi), wanted, "tgamma(1/2)^2 == pi", reportTestCases);
		nrOfFailedTestCases += expect_bits(bits_vs(Real(2.0) / g, s_two_over_sqrt_pi), wanted, "2 / tgamma(1/2) == 2/sqrt(pi)", reportTestCases);
		// the recurrence tgamma(x + 1) == x tgamma(x)
		for (double v : { 1.5, 2.7, 4.2, -0.6, -3.3 }) {
			const Real x(v);
			nrOfFailedTestCases += expect_bits(bits_vs(tgamma(x + Real(1.0)), x * tgamma(x)), wanted, "tgamma(x + 1) == x tgamma(x) at " + std::to_string(v), reportTestCases);
		}
		// the reflection tgamma(x) tgamma(1 - x) == pi / sin(pi x)
		for (double v : { 0.3, -1.25 }) {
			const Real x(v);
			const Real pi = ereal_pi<Real::maxNrLimbs, typename limb_of<Real>::type>();
			nrOfFailedTestCases += expect_bits(bits_vs(tgamma(x) * tgamma(Real(1.0) - x), pi / sin(pi * x)), wanted, "reflection at " + std::to_string(v), reportTestCases);
		}

		// special values (C99 F.10.5.4): tgamma(+/-0) = +/-Inf, tgamma(negative integer) = NaN,
		// tgamma(-Inf) = NaN, tgamma(+Inf) = +Inf, tgamma(NaN) = NaN
		const double pinf = std::numeric_limits<double>::infinity();
		const Real pz = tgamma(Real(0.0)), nz = tgamma(-Real(0.0));
		if (!pz.isinf() || pz.signbit() || !nz.isinf() || !nz.signbit()) {
			if (reportTestCases) std::cout << "    FAIL tgamma(+/-0) != +/-Inf\n";
			++nrOfFailedTestCases;
		}
		if (!tgamma(Real(-1.0)).isnan() || !tgamma(Real(-7.0)).isnan() || !tgamma(Real(-pinf)).isnan()) {
			if (reportTestCases) std::cout << "    FAIL tgamma(negative integer or -Inf) != NaN\n";
			++nrOfFailedTestCases;
		}
		const Real rinf = tgamma(Real(pinf));
		if (!rinf.isinf() || rinf.signbit()) {
			if (reportTestCases) std::cout << "    FAIL tgamma(+Inf) != +Inf\n";
			++nrOfFailedTestCases;
		}
		if (!tgamma(Real(std::numeric_limits<double>::quiet_NaN())).isnan()) {
			if (reportTestCases) std::cout << "    FAIL tgamma(NaN) != NaN\n";
			++nrOfFailedTestCases;
		}
		return nrOfFailedTestCases;
	}

	// lgamma: fixed points, the special values, the stored references, and lgamma == log tgamma
	template<typename Real>
	int VerifyLgamma(bool reportTestCases) {
		int nrOfFailedTestCases = 0;

		// lgamma(1) == 0 and lgamma(2) == 0
		if (!lgamma(Real(1.0)).iszero() || !lgamma(Real(2.0)).iszero()) {
			if (reportTestCases) std::cout << "    FAIL lgamma(1) or lgamma(2) != 0\n";
			++nrOfFailedTestCases;
		}
		// lgamma(3) = log(2)
		nrOfFailedTestCases += expect_ref(lgamma(Real(3.0)), s_ln2, "lgamma(3) == log(2)", reportTestCases);

		// the stored references
		for (const auto& c : gamma_cases) {
			nrOfFailedTestCases += expect_ref(lgamma(Real(c.x)), c.lgamma_ref, "lgamma(" + std::to_string(c.x) + ")", reportTestCases);
		}

		// lgamma(x) == log |tgamma(x)|
		const int wanted = ereal_bits_of<Real>::value - kIdentityMargin;
		for (double v : { 0.5, 1.5, 3.0, 5.0, 10.0, -2.5 }) {
			const Real x(v);
			nrOfFailedTestCases += expect_bits(bits_vs(lgamma(x), log(abs(tgamma(x)))), wanted, "lgamma == log |tgamma| at " + std::to_string(v), reportTestCases);
		}

		// special values (C99 F.10.5.3): lgamma(+/-Inf) = +Inf, lgamma(0) = +Inf,
		// lgamma(negative integer) = +Inf
		const double pinf = std::numeric_limits<double>::infinity();
		for (double v : { pinf, -pinf, 0.0, -1.0, -4.0 }) {
			const Real r = lgamma(Real(v));
			if (!r.isinf() || r.signbit()) {
				if (reportTestCases) std::cout << "    FAIL lgamma(" << v << ") != +Inf\n";
				++nrOfFailedTestCases;
			}
		}
		return nrOfFailedTestCases;
	}

	// Property fuzzer: erf parity and monotonicity, erf + erfc == 1, and the gamma recurrence
	// over random inputs
	template<typename Real>
	int VerifyErrorGammaFuzz(bool reportTestCases, unsigned nrIterations) {
		int nrOfFailedTestCases = 0;
		const int wanted = ereal_bits_of<Real>::value - kIdentityMargin;
		std::mt19937_64 rng(0xC1A55'1FFEULL);
		std::uniform_real_distribution<double> edist(-3.0, 3.0);
		std::uniform_real_distribution<double> gdist(0.1, 30.0);
		for (unsigned i = 0; i < nrIterations; ++i) {
			const double e = edist(rng);
			const double gv = gdist(rng);
			const Real x(e);
			if (!exactly(erf(-x), -erf(x))) {
				if (reportTestCases) std::cout << "    FAIL erf parity at " << e << "\n";
				++nrOfFailedTestCases;
			}
			if (!(erf(x) < erf(x + Real(0.5)))) {   // erf strictly increasing
				if (reportTestCases) std::cout << "    FAIL erf monotonic at " << e << "\n";
				++nrOfFailedTestCases;
			}
			nrOfFailedTestCases += expect_bits(agreed_bits_absolute(erf(x) + erfc(x), Real(1.0)), wanted, "erf + erfc == 1 at " + std::to_string(e), reportTestCases);
			const Real g(gv);
			nrOfFailedTestCases += expect_bits(bits_vs(tgamma(g + Real(1.0)), g * tgamma(g)), wanted, "tgamma(x + 1) == x tgamma(x) at " + std::to_string(gv), reportTestCases);
		}
		return nrOfFailedTestCases;
	}

}  // anonymous namespace

// Regression testing guards
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

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "ereal mathlib error and gamma function validation";
	std::string test_tag    = "erf/erfc/tgamma/lgamma";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyErrorFunction<ereal<>>(reportTestCases), "erf/erfc(ereal)", test_tag);
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#	if REGRESSION_LEVEL_1
	test_tag = "erf/erfc";
	nrOfFailedTestCases += ReportTestResult(VerifyErrorFunction<ereal<4>>(reportTestCases), "erf/erfc(ereal<4>)", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyErrorFunction<ereal<>>(reportTestCases), "erf/erfc(ereal<8>)", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyErrorFunction<ereal<4, float>>(reportTestCases), "erf/erfc(ereal<4, float>)", test_tag);

	test_tag = "tgamma";
	nrOfFailedTestCases += ReportTestResult(VerifyTgamma<ereal<4>>(reportTestCases), "tgamma(ereal<4>)", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyTgamma<ereal<>>(reportTestCases), "tgamma(ereal<8>)", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyTgamma<ereal<4, float>>(reportTestCases), "tgamma(ereal<4, float>)", test_tag);

	test_tag = "lgamma";
	nrOfFailedTestCases += ReportTestResult(VerifyLgamma<ereal<4>>(reportTestCases), "lgamma(ereal<4>)", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLgamma<ereal<>>(reportTestCases), "lgamma(ereal<8>)", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLgamma<ereal<4, float>>(reportTestCases), "lgamma(ereal<4, float>)", test_tag);
#	endif

#	if REGRESSION_LEVEL_2
	test_tag = "error/gamma fuzz";
	nrOfFailedTestCases += ReportTestResult(VerifyErrorGammaFuzz<ereal<>>(reportTestCases, 50), "error/gamma property fuzz", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyErrorFunction<ereal<16>>(reportTestCases), "erf/erfc(ereal<16>)", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyTgamma<ereal<16>>(reportTestCases), "tgamma(ereal<16>)", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLgamma<ereal<16>>(reportTestCases), "lgamma(ereal<16>)", test_tag);
#	endif

#	if REGRESSION_LEVEL_3
	// ereal<19> has no wider configuration to run erfc's 1 - erf in, and gives up the bits
	// that cancellation costs for 2 < x < 8: its erf/erfc checks are only the range edge
	test_tag = "erf range edge";
	nrOfFailedTestCases += ReportTestResult(VerifyErfRangeEdge<ereal<19>>(reportTestCases), "erf range edge(ereal<19>)", test_tag);
	test_tag = "gamma";
	nrOfFailedTestCases += ReportTestResult(VerifyTgamma<ereal<19>>(reportTestCases), "tgamma(ereal<19>)", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLgamma<ereal<19>>(reportTestCases), "lgamma(ereal<19>)", test_tag);
#	endif

#	if REGRESSION_LEVEL_4
#	endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::exception& err) {
	std::cerr << "Caught exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
