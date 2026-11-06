// exponential.cpp: test suite runner for faithful's exp, exp2, exp10, expm1, log, log2, log10 and log1p
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The exponentials and logarithms are compensated (#1955): evaluated in faithful arithmetic, not
// through the FloatingPointType. Each is measured against a reference that holds more than the
// pair's precision: dd for faithful<float>, qd for faithful<double>. The error is in
// units of the pair's epsilon(), eps(Real)^2 / 2, relative to the exact result.
#include <universal/utility/directives.hpp>
#include <universal/number/faithful/faithful.hpp>
#include <universal/number/dd/dd.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/verification/test_suite.hpp>
#include <cerrno>
#include <cmath>
#include <functional>
#include <limits>
#include <random>
#include <string>

namespace {

using sw::universal::faithful;
using sw::universal::qd;

// the reference arithmetic of each FloatingPointType
// The faithful<float> reference is dd, not long double: on MSVC and on macOS ARM64 long double is
// a 53-bit double, which cannot hold a pair such as (1, 2^-40) exactly, so the reference would
// take the logarithm of a different number. dd holds every canonical faithful<float> exactly and
// its functions are accurate to about 2^-100, far below a unit of 2^-47.
struct FloatRef {
	using Real = float;
	using Exact = sw::universal::dd;
	static constexpr int epsExp = -47;
	static Exact value(const faithful<float>& v) { return Exact(double(v.high())) + Exact(double(v.low())); }
	static Exact exp(const Exact& v)   { return sw::universal::exp(v); }
	static Exact exp2(const Exact& v)  { return sw::universal::exp2(v); }
	static Exact exp10(const Exact& v) { return sw::universal::exp(v * sw::universal::dd_ln10); }
	static Exact expm1(const Exact& v) { return sw::universal::expm1(v); }
	static Exact log(const Exact& v)   { return sw::universal::log(v); }
	static Exact log2(const Exact& v)  { return sw::universal::log2(v); }
	static Exact log10(const Exact& v) { return sw::universal::log10(v); }
	static Exact log1p(const Exact& v) { return sw::universal::log1p(v); }
	static double rel(const Exact& got, const Exact& want) { return double(abs((got - want) / want)); }
};
struct DoubleRef {
	using Real = double;
	using Exact = qd;
	static constexpr int epsExp = -105;
	static Exact value(const faithful<double>& v) { return qd(v.high()) + qd(v.low()); }
	static Exact exp(const Exact& v)   { return sw::universal::exp(v); }
	static Exact exp2(const Exact& v)  { return sw::universal::exp2(v); }
	static Exact exp10(const Exact& v) { return sw::universal::exp(v * sw::universal::qd_ln10); }
	static Exact expm1(const Exact& v) { return sw::universal::expm1(v); }
	static Exact log(const Exact& v)   { return sw::universal::log(v); }
	static Exact log2(const Exact& v)  { return sw::universal::log2(v); }
	static Exact log10(const Exact& v) { return sw::universal::log10(v); }
	static Exact log1p(const Exact& v) { return sw::universal::log1p(v); }
	static double rel(const Exact& got, const Exact& want) { return double(abs((got - want) / want)); }
};

// a canonical pair with a head in [lo, hi) and a random error part
template<typename Real>
faithful<Real> randomIn(std::mt19937_64& rng, Real lo, Real hi) {
	std::uniform_real_distribution<Real> head(lo, hi);
	std::uniform_real_distribution<Real> frac(Real(-0.5), Real(0.5));
	const Real v = head(rng);
	const Real ulp = std::nextafter(std::fabs(v), std::numeric_limits<Real>::infinity()) - std::fabs(v);
	return faithful<Real>(v, frac(rng) * ulp);
}
// a canonical positive pair with a log-uniform head over [2^e0, 2^e1)
template<typename Real>
faithful<Real> randomLog(std::mt19937_64& rng, int e0, int e1) {
	std::uniform_real_distribution<Real> mant(Real(1), Real(2));
	std::uniform_int_distribution<int> expo(e0, e1 - 1);
	std::uniform_real_distribution<Real> frac(Real(-0.5), Real(0.5));
	const Real v = std::ldexp(mant(rng), expo(rng));
	const Real ulp = std::nextafter(v, std::numeric_limits<Real>::infinity()) - v;
	return faithful<Real>(v, frac(rng) * ulp);
}

// the worst error of f against ref over the arguments gen produces, in units of epsilon()
template<typename Ref, typename Fn, typename RefFn, typename Gen>
double worstUnits(Fn f, RefFn ref, Gen gen, int n, std::mt19937_64& rng, faithful<typename Ref::Real>& worstAt) {
	double worst = 0.0;
	for (int i = 0; i < n; ++i) {
		const faithful<typename Ref::Real> x = gen(rng);
		const auto want = ref(Ref::value(x));
		const double u = Ref::rel(Ref::value(f(x)), want) / std::ldexp(1.0, Ref::epsExp);
		if (std::isnan(u) ? !std::isnan(worst) : (!std::isnan(worst) && u > worst)) { worst = u; worstAt = x; }   // a NaN, once seen, sticks
	}
	return worst;
}

template<typename Ref>
int VerifyAccuracy(bool reportTestCases, double bound) {
	using Real = typename Ref::Real;
	using F = faithful<Real>;
	using namespace sw::universal;
	constexpr int digits = std::numeric_limits<Real>::digits;
	// exp's results within the full-precision range: below min(), 2^-102 for float and 2^-969
	// for double, the error part goes subnormal and the pair holds fewer digits
	const Real expMax = std::log(std::numeric_limits<Real>::max()) * Real(0.98);
	const Real expMin = Real(std::log(Real(std::numeric_limits<F>::min()))) * Real(0.98);
	const int eMin = std::numeric_limits<Real>::min_exponent + digits;
	const int eMax = std::numeric_limits<Real>::max_exponent - 1;
	std::mt19937_64 rng(1955);
	int fails = 0;
	F at;
	auto report = [&](const char* name, double worst) {
		std::cout << "  " << type_tag(F()) << ' ' << name << " worst " << worst << " units";
		if (worst <= bound) { std::cout << '\n'; return; }
		++fails;
		std::cout << "  FAIL at " << at << '\n';
		(void)reportTestCases;
	};
	constexpr int N = 20000;
	report("exp  [-1, 1]",     worstUnits<Ref>([](const F& x) { return exp(x); },   [](const auto& v) { return Ref::exp(v); },   [](std::mt19937_64& g) { return randomIn<Real>(g, Real(-1), Real(1)); }, N, rng, at));
	report("exp  full range",  worstUnits<Ref>([](const F& x) { return exp(x); },   [](const auto& v) { return Ref::exp(v); },   [&](std::mt19937_64& g) { return randomIn<Real>(g, expMin, expMax); }, N, rng, at));
	report("exp2 full range",  worstUnits<Ref>([](const F& x) { return exp2(x); },  [](const auto& v) { return Ref::exp2(v); },  [&](std::mt19937_64& g) { return randomIn<Real>(g, Real(eMin), Real(eMax)); }, N, rng, at));
	report("exp10 full range", worstUnits<Ref>([](const F& x) { return exp10(x); }, [](const auto& v) { return Ref::exp10(v); }, [&](std::mt19937_64& g) { return randomIn<Real>(g, Real(eMin) * Real(0.301), Real(eMax) * Real(0.301)); }, N, rng, at));
	report("exp10 [-30, 30]",  worstUnits<Ref>([](const F& x) { return exp10(x); }, [](const auto& v) { return Ref::exp10(v); }, [](std::mt19937_64& g) { return randomIn<Real>(g, Real(-30), Real(30)); }, N, rng, at));
	report("expm1 small",      worstUnits<Ref>([](const F& x) { return expm1(x); }, [](const auto& v) { return Ref::expm1(v); }, [](std::mt19937_64& g) { F x = randomLog<Real>(g, -60, -1); return (g() & 1) ? -x : x; }, N, rng, at));
	report("expm1 [-5, 5]",    worstUnits<Ref>([](const F& x) { return expm1(x); }, [](const auto& v) { return Ref::expm1(v); }, [](std::mt19937_64& g) { return randomIn<Real>(g, Real(-5), Real(5)); }, N, rng, at));
	report("log  full range",  worstUnits<Ref>([](const F& x) { return log(x); },   [](const auto& v) { return Ref::log(v); },   [&](std::mt19937_64& g) { return randomLog<Real>(g, eMin, eMax); }, N, rng, at));
	report("log  near 1",      worstUnits<Ref>([](const F& x) { return log(x); },   [](const auto& v) { return Ref::log(v); },   [](std::mt19937_64& g) { const F d = randomLog<Real>(g, -40, -2); return (g() & 1) ? F(Real(1)) + d : F(Real(1)) - d; }, N, rng, at));
	report("log2 full range",  worstUnits<Ref>([](const F& x) { return log2(x); },  [](const auto& v) { return Ref::log2(v); },  [&](std::mt19937_64& g) { return randomLog<Real>(g, eMin, eMax); }, N, rng, at));
	report("log10 full range", worstUnits<Ref>([](const F& x) { return log10(x); }, [](const auto& v) { return Ref::log10(v); }, [&](std::mt19937_64& g) { return randomLog<Real>(g, eMin, eMax); }, N, rng, at));
	report("log1p small",      worstUnits<Ref>([](const F& x) { return log1p(x); }, [](const auto& v) { return Ref::log1p(v); }, [](std::mt19937_64& g) { F x = randomLog<Real>(g, -60, -2); return (g() & 1) ? -x : x; }, N, rng, at));
	report("log1p [-0.9, 10]", worstUnits<Ref>([](const F& x) { return log1p(x); }, [](const auto& v) { return Ref::log1p(v); }, [](std::mt19937_64& g) { return randomIn<Real>(g, Real(-0.9), Real(10)); }, N, rng, at));
	return fails;
}

template<typename Real>
int VerifySpecialValues(bool reportTestCases) {
	using F = faithful<Real>;
	using namespace sw::universal;
	using RL = std::numeric_limits<Real>;
	int fails = 0;
	auto check = [&](bool ok, const char* what) {
		if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL " << type_tag(F()) << ' ' << what << '\n'; }
	};
	const F pinf(RL::infinity()), ninf(-RL::infinity()), nan(RL::quiet_NaN()), one(Real(1)), zero(Real(0));
	check(exp(zero) == one && exp2(zero) == one && exp10(zero) == one, "exp(0) = 1");
	check(isinf(exp(pinf)) && iszero(exp(ninf)) && isnan(exp(nan)), "exp(+-inf), exp(NaN)");
	errno = 0;
	check(isinf(exp(F(RL::max()))) && errno == ERANGE, "exp overflow is inf with ERANGE");
	errno = 0;
	check(iszero(exp(F(-RL::max()))) && errno == ERANGE, "exp underflow is 0 with ERANGE");
	check(exp2(F(Real(10))) == F(Real(1024)) && exp2(F(Real(-3))) == F(Real(0.125)), "exp2 of an integer is exact");
	check(signbit(expm1(F(Real(-0.0)))) && expm1(ninf) == F(Real(-1)), "expm1(-0) = -0, expm1(-inf) = -1");
	errno = 0;
	check(expm1(F(-RL::max())) == F(Real(-1)) && errno == 0, "expm1(-max) = -1 without a range error");
	check(log(one) == zero && log2(one) == zero && log10(one) == zero, "log(1) = 0");
	check(log2(F(Real(1024))) == F(Real(10)) && log2(F(Real(0.125))) == F(Real(-3)), "log2 of a power of two is exact");
	check(log2(F(std::ldexp(Real(1), RL::min_exponent - RL::digits))) == F(Real(RL::min_exponent - RL::digits)), "log2(denorm_min) is exact");
	errno = 0;
	check(isinf(log(zero)) && signbit(log(zero)) && errno == ERANGE, "log(0) = -inf with ERANGE");
	errno = 0;
	check(isnan(log(F(Real(-1)))) && errno == EDOM, "log(-1) = NaN with EDOM");
	check(isinf(log(pinf)) && isnan(log(nan)), "log(inf), log(NaN)");
	check(signbit(log1p(F(Real(-0.0)))), "log1p(-0) = -0");
	errno = 0;
	check(isinf(log1p(F(Real(-1)))) && errno == ERANGE, "log1p(-1) = -inf with ERANGE");
	errno = 0;
	check(isnan(log1p(F(Real(-2)))) && errno == EDOM, "log1p(-2) = NaN with EDOM");
	// round trips
	const F x(Real(2.75), Real(0));
	check(std::fabs(Real(log(exp(x)) - x)) <= std::ldexp(Real(1), -2 * RL::digits + 4), "log(exp(x)) = x");
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

	std::string test_suite  = "faithful exp, exp2, exp10, expm1, log, log2, log10, log1p";
	std::string test_tag    = "faithful";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	faithful<double> x(1.0);
	std::cout << exp(x) << ' ' << log(x) << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	// the bound, in units of epsilon(). Measured worst over 20000 arguments per range, gcc and
	// clang: 2.7 for faithful<float> and 3.7 for faithful<double>. Through the FloatingPointType,
	// exp and log are off by 10^8 to 10^27 units, so the test fails it.
	constexpr double bound = 5.0;
	nrOfFailedTestCases += ReportTestResult(VerifyAccuracy<FloatRef>(reportTestCases, bound), "faithful<float>", "accuracy");
	nrOfFailedTestCases += ReportTestResult(VerifyAccuracy<DoubleRef>(reportTestCases, bound), "faithful<double>", "accuracy");
	nrOfFailedTestCases += ReportTestResult(VerifySpecialValues<float>(reportTestCases), "faithful<float>", "special values");
	nrOfFailedTestCases += ReportTestResult(VerifySpecialValues<double>(reportTestCases), "faithful<double>", "special values");
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
