// limits_and_attributes.cpp: test suite runner for faithful's numeric_limits, extremes and attributes
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Until #1949 std::numeric_limits<faithful<Real>> was a stub: min(), max() and lowest() were zero,
// epsilon() and denorm_min() one, and digits 3333333. The minpos/maxpos/minneg/maxneg helpers
// returned their argument unchanged, and attributes.hpp did not compile. The limits now follow the
// convention of dd (#1882), with Real in place of double: min() is the full-precision floor,
// epsilon() is eps(Real)^2 / 2, and max() is Real's maximum plus the largest error that does not
// round the pair up to infinity.
#include <universal/utility/directives.hpp>
#include <universal/number/faithful/faithful.hpp>
#include <universal/verification/test_suite.hpp>
#include <cmath>
#include <limits>
#include <string>

namespace {

// the limits are constant expressions, as for the native types
using FD = sw::universal::faithful<double>;
static_assert(std::numeric_limits<FD>::epsilon().high() == 0x1p-105, "faithful<double> epsilon is 2^-105");
static_assert(std::numeric_limits<FD>::min().high() == 0x1p-969, "faithful<double> min is 2^-969, as dd's");
static_assert(std::numeric_limits<FD>::max().high() == std::numeric_limits<double>::max(), "faithful<double> max leads with DBL_MAX");
static_assert(std::numeric_limits<FD>::max().low() == 0x1.fffffffffffffp+969, "faithful<double> max error is DBL_MAX * 2^-54, as dd's");
static_assert(std::numeric_limits<FD>::digits == 106, "faithful<double> carries 106 bits");

// the exact sum of a faithful<float>, which fits a double
double exact(const sw::universal::faithful<float>& v) { return double(v.high()) + double(v.low()); }

template<typename Real>
int VerifyLimits(bool reportTestCases) {
	using namespace sw::universal;
	using F = faithful<Real>;
	using L = std::numeric_limits<F>;
	using RL = std::numeric_limits<Real>;
	int fails = 0;
	auto check = [&](bool ok, const char* what) {
		if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL " << type_tag(F()) << ' ' << what << '\n'; }
	};
	check(L::is_specialized && L::is_signed && !L::is_integer && !L::is_exact && L::radix == 2, "basic traits");
	check(L::digits == 2 * RL::digits, "digits is twice Real's");
	check(L::digits10 == static_cast<int>(L::digits * 0.30103), "digits10");
	check(L::max_digits10 == static_cast<int>(std::ceil(1 + L::digits * 0.3010299956639812)), "max_digits10 = ceil(1 + digits * log10 2)");

	// min() == 2^(min_exponent - 1), the full-precision floor: Real's floor raised by Real's digits
	check(L::min_exponent == RL::min_exponent + RL::digits, "min_exponent");
	check(L::min().high() == std::ldexp(Real(1), L::min_exponent - 1) && L::min().low() == 0, "min() == 2^(min_exponent - 1)");
	check(L::min().high() == std::ldexp(RL::min(), RL::digits), "min() is Real's min * 2^digits");
	check(L::max_exponent == RL::max_exponent, "max_exponent");

	// epsilon() = eps(Real)^2 / 2, and 1 + epsilon() is a pair distinct from 1
	check(L::epsilon().high() == RL::epsilon() * RL::epsilon() / 2 && L::epsilon().low() == 0, "epsilon() == eps(Real)^2 / 2");
	{
		const F one(Real(1));
		const F next = one + L::epsilon();
		check(next != one && next > one && next.high() == Real(1) && next.low() == L::epsilon().high(), "1 + epsilon() > 1");
	}

	// max(): the value is Real's maximum, the error just below half its ulp, and the pair finite
	{
		const F m = L::max();
		const Real top = RL::max();
		const Real halfUlp = (top - std::nextafter(top, Real(0))) / 2;
		check(m.high() == top, "max() leads with Real's max");
		check(m.low() > 0 && m.low() < halfUlp, "max() error is below half an ulp of Real's max");
		check(m.low() == std::ldexp(top, -(RL::digits + 1)), "max() error is max * 2^-(digits + 1)");
		check(!m.isinf() && std::isfinite(Real(m)), "max() is finite and converts to a finite Real");
		check(L::lowest() == -m && L::lowest().sign(), "lowest() == -max()");
		check(m > F(top) && F(top) < m, "max() is above Real's max");
	}

	// denorm_min() is Real's subnormal minimum, the smallest positive pair
	check(L::denorm_min().high() == RL::denorm_min() && L::denorm_min().low() == 0, "denorm_min() is Real's");
	check(L::denorm_min() > F(Real(0)) && L::denorm_min() < L::min(), "0 < denorm_min() < min()");

	// the specials
	check(L::infinity().isinf() && !L::infinity().sign() && L::infinity().low() == 0, "infinity()");
	check(L::quiet_NaN().isnan() && L::signaling_NaN().isnan(), "the NaNs");
	check(L::has_infinity == RL::has_infinity && L::has_quiet_NaN == RL::has_quiet_NaN, "has_infinity, has_quiet_NaN");
	check(L::round_error() == F(Real(0.5)), "round_error() is one half");
	check(L::round_style == std::round_to_nearest && L::is_bounded && !L::is_iec559, "round_style, is_bounded, is_iec559");
	return fails;
}

// the free minpos/maxpos/minneg/maxneg set their argument and agree with numeric_limits
template<typename Real>
int VerifyExtremes(bool reportTestCases) {
	using namespace sw::universal;
	using F = faithful<Real>;
	using L = std::numeric_limits<F>;
	int fails = 0;
	auto check = [&](bool ok, const char* what) {
		if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL " << type_tag(F()) << ' ' << what << '\n'; }
	};
	F a, b, c, d;
	check(&minpos(a) == &a && a == L::denorm_min(), "minpos is denorm_min()");
	check(&maxpos(b) == &b && b == L::max(), "maxpos is max()");
	check(&minneg(c) == &c && c == -L::denorm_min() && c.sign(), "minneg is -denorm_min()");
	check(&maxneg(d) == &d && d == L::lowest(), "maxneg is lowest()");
	check(d < c && c < F(Real(0)) && F(Real(0)) < a && a < b, "maxneg < minneg < 0 < minpos < maxpos");
	return fails;
}

// sign(), scale() and faithful_range() from attributes.hpp
int VerifyAttributes(bool reportTestCases) {
	using namespace sw::universal;
	using F = faithful<float>;
	int fails = 0;
	auto check = [&](bool ok, const std::string& what) {
		if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL " << what << '\n'; }
	};
	check(sign(F(-2.0f)) && !sign(F(2.0f)), "sign()");
	check(sign(F(1.0f, -2.0f)), "sign() of the represented value, (1, -2) is -1");
	check(scale(F(8.0f)) == 3 && scale(F(0.375f)) == -2, "scale()");
	check(scale(F(1.0f, 3.0f)) == 2, "scale() of the represented value, (1, 3) is 4");

	// minneg is -minpos, and unary minus negates both parts, so its error prints as -0
	const std::string r = faithful_range<float>();
	F lmaxpos;
	maxpos(lmaxpos);
	check(r.find("faithful<float> : [ ") != std::string::npos, "faithful_range() names the type: " + r);
	check(r.find("( -3.40282e+38, -1.01412e+31) ... ( -1.4013e-45, -0) 0 ( 1.4013e-45, 0) ... ( 3.40282e+38, 1.01412e+31) ]") != std::string::npos,
	      "faithful_range() lists maxneg ... minneg 0 minpos ... maxpos: " + r);
	check(exact(lmaxpos) == double(std::numeric_limits<float>::max()) + std::ldexp(double(std::numeric_limits<float>::max()), -25), "maxpos of faithful<float>, exactly");
	check(faithful_range<double>().find("faithful<double>") != std::string::npos, "faithful_range<double>()");
	return fails;
}

}  // namespace

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

	std::string test_suite  = "faithful numeric_limits, extremes and attributes";
	std::string test_tag    = "faithful";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	using L = std::numeric_limits<faithful<double>>;
	std::cout << L::min() << ' ' << L::max() << ' ' << L::epsilon() << ' ' << L::denorm_min() << '\n';
	std::cout << faithful_range<double>() << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyLimits<float>(reportTestCases), "faithful<float>", "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyLimits<double>(reportTestCases), "faithful<double>", "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyExtremes<float>(reportTestCases), "faithful<float>", "minpos maxpos minneg maxneg");
	nrOfFailedTestCases += ReportTestResult(VerifyExtremes<double>(reportTestCases), "faithful<double>", "minpos maxpos minneg maxneg");
	nrOfFailedTestCases += ReportTestResult(VerifyAttributes(reportTestCases), test_tag, "sign scale faithful_range");
#endif

#if REGRESSION_LEVEL_2
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
