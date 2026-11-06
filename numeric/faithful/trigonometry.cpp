// trigonometry.cpp: test suite runner for faithful's sin, cos, tan, asin, acos, atan and atan2
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The trigonometric functions are compensated (#1956). Each is measured against a reference with
// more than the pair's precision: dd for faithful<float>, and qd for faithful<double>,
// whose sin and cos use the exact Payne-Hanek reduction since #1723. The error is in units of
// the pair's epsilon(), relative to the exact result. The arguments include the whole finite range
// and values next to multiples of pi/2, where an inexact reduction loses the result.
#include <universal/utility/directives.hpp>
#include <universal/number/faithful/faithful.hpp>
#include <universal/number/dd/dd.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/verification/test_suite.hpp>
#include <cerrno>
#include <cmath>
#include <limits>
#include <random>
#include <string>

namespace {

using sw::universal::faithful;
using sw::universal::qd;

// The faithful<float> reference is dd, not long double: on MSVC and macOS ARM64 long double is a
// 53-bit double, which cannot hold every canonical faithful<float> exactly (#1955).
struct FloatRef {
	using Real = float;
	using Exact = sw::universal::dd;
	static constexpr int epsExp = -47;
	static Exact value(const faithful<float>& v) { return Exact(double(v.high())) + Exact(double(v.low())); }
	static Exact sin(const Exact& v) { return sw::universal::sin(v); }
	static Exact cos(const Exact& v) { return sw::universal::cos(v); }
	static Exact tan(const Exact& v) { return sw::universal::tan(v); }
	static Exact asin(const Exact& v) { return sw::universal::asin(v); }
	static Exact acos(const Exact& v) { return sw::universal::acos(v); }
	static Exact atan(const Exact& v) { return sw::universal::atan(v); }
	static Exact atan2(const Exact& y, const Exact& x) { return sw::universal::atan2(y, x); }
	static double rel(const Exact& got, const Exact& want) { return double(abs((got - want) / want)); }
	// the pair nearest k pi/2
	static faithful<float> nearPio2(long long k) {
		const Exact v = sw::universal::dd_pi_2 * Exact(static_cast<double>(k));
		const float h = static_cast<float>(double(v));
		return faithful<float>(h, static_cast<float>(double(v - Exact(double(h)))));
	}
};
struct DoubleRef {
	using Real = double;
	using Exact = qd;
	static constexpr int epsExp = -105;
	static Exact value(const faithful<double>& v) { return qd(v.high()) + qd(v.low()); }
	static Exact sin(const Exact& v) { return sw::universal::sin(v); }
	static Exact cos(const Exact& v) { return sw::universal::cos(v); }
	static Exact tan(const Exact& v) { return sw::universal::tan(v); }
	static Exact asin(const Exact& v) { return sw::universal::asin(v); }
	static Exact acos(const Exact& v) { return sw::universal::acos(v); }
	static Exact atan(const Exact& v) { return sw::universal::atan(v); }
	static Exact atan2(const Exact& y, const Exact& x) { return sw::universal::atan2(y, x); }
	static double rel(const Exact& got, const Exact& want) { return double(abs((got - want) / want)); }
	static faithful<double> nearPio2(long long k) {
		const qd v = sw::universal::qd_pi_2 * qd(static_cast<double>(k));
		const double h = double(v);
		return faithful<double>(h, double(v - qd(h)));
	}
};

template<typename Real>
faithful<Real> randomIn(std::mt19937_64& rng, Real lo, Real hi) {
	std::uniform_real_distribution<Real> head(lo, hi);
	std::uniform_real_distribution<Real> frac(Real(-0.5), Real(0.5));
	const Real v = head(rng);
	const Real ulp = std::nextafter(std::fabs(v), std::numeric_limits<Real>::infinity()) - std::fabs(v);
	return faithful<Real>(v, frac(rng) * ulp);
}
template<typename Real>
faithful<Real> randomLog(std::mt19937_64& rng, int e0, int e1) {
	std::uniform_real_distribution<Real> mant(Real(1), Real(2));
	std::uniform_int_distribution<int> expo(e0, e1 - 1);
	std::uniform_real_distribution<Real> frac(Real(-0.5), Real(0.5));
	std::bernoulli_distribution neg(0.5);
	Real v = std::ldexp(mant(rng), expo(rng));
	const Real ulp = std::nextafter(v, std::numeric_limits<Real>::infinity()) - v;
	faithful<Real> x(v, frac(rng) * ulp);
	return neg(rng) ? -x : x;
}

template<typename Ref, typename Fn, typename RefFn, typename Gen>
double worstUnits(Fn f, RefFn ref, Gen gen, int n, std::mt19937_64& rng, faithful<typename Ref::Real>& worstAt) {
	double worst = 0.0;
	for (int i = 0; i < n; ++i) {
		const faithful<typename Ref::Real> x = gen(rng);
		const auto want = ref(Ref::value(x));
		const double u = Ref::rel(Ref::value(f(x)), want) / std::ldexp(1.0, Ref::epsExp);
		if (!(u <= worst)) { worst = u; worstAt = x; }
	}
	return worst;
}

template<typename Ref>
int VerifyAccuracy(double bound) {
	using Real = typename Ref::Real;
	using F = faithful<Real>;
	using namespace sw::universal;
	const int eMax = std::numeric_limits<Real>::max_exponent - 1;
	std::mt19937_64 rng(1956);
	int fails = 0;
	F at;
	auto report = [&](const char* name, double worst) {
		std::cout << "  " << type_tag(F()) << ' ' << name << " worst " << worst << " units";
		if (worst <= bound) { std::cout << '\n'; return; }
		++fails;
		std::cout << "  FAIL at " << at << '\n';
	};
	constexpr int N = 20000;
	const Real pi = Real(3.14159265358979323846L);
	auto small = [&](std::mt19937_64& g) { return randomIn<Real>(g, -pi, pi); };
	auto mid   = [&](std::mt19937_64& g) { return randomIn<Real>(g, Real(-1.0e4), Real(1.0e4)); };
	auto wide  = [&](std::mt19937_64& g) { return randomLog<Real>(g, -20, eMax); };
	auto nearK = [&](std::mt19937_64& g) { std::uniform_int_distribution<long long> k(1, 1000000); return Ref::nearPio2(k(g)); };
	report("sin [-pi, pi]",   worstUnits<Ref>([](const F& x) { return sin(x); }, [](const auto& v) { return Ref::sin(v); }, small, N, rng, at));
	report("cos [-pi, pi]",   worstUnits<Ref>([](const F& x) { return cos(x); }, [](const auto& v) { return Ref::cos(v); }, small, N, rng, at));
	report("tan [-pi, pi]",   worstUnits<Ref>([](const F& x) { return tan(x); }, [](const auto& v) { return Ref::tan(v); }, small, N, rng, at));
	report("sin [-1e4, 1e4]", worstUnits<Ref>([](const F& x) { return sin(x); }, [](const auto& v) { return Ref::sin(v); }, mid, N, rng, at));
	report("cos [-1e4, 1e4]", worstUnits<Ref>([](const F& x) { return cos(x); }, [](const auto& v) { return Ref::cos(v); }, mid, N, rng, at));
	report("sin to max()",    worstUnits<Ref>([](const F& x) { return sin(x); }, [](const auto& v) { return Ref::sin(v); }, wide, N, rng, at));
	report("cos to max()",    worstUnits<Ref>([](const F& x) { return cos(x); }, [](const auto& v) { return Ref::cos(v); }, wide, N, rng, at));
	report("sin near k pi/2", worstUnits<Ref>([](const F& x) { return sin(x); }, [](const auto& v) { return Ref::sin(v); }, nearK, N, rng, at));
	report("cos near k pi/2", worstUnits<Ref>([](const F& x) { return cos(x); }, [](const auto& v) { return Ref::cos(v); }, nearK, N, rng, at));
	report("atan wide",       worstUnits<Ref>([](const F& x) { return atan(x); }, [](const auto& v) { return Ref::atan(v); }, [&](std::mt19937_64& g) { return randomLog<Real>(g, -40, 40); }, N, rng, at));
	report("asin [-1, 1]",    worstUnits<Ref>([](const F& x) { return asin(x); }, [](const auto& v) { return Ref::asin(v); }, [](std::mt19937_64& g) { return randomIn<Real>(g, Real(-1), Real(1)); }, N, rng, at));
	report("acos [-1, 1]",    worstUnits<Ref>([](const F& x) { return acos(x); }, [](const auto& v) { return Ref::acos(v); }, [](std::mt19937_64& g) { return randomIn<Real>(g, Real(-1), Real(1)); }, N, rng, at));
	report("asin near +-1",   worstUnits<Ref>([](const F& x) { return asin(x); }, [](const auto& v) { return Ref::asin(v); }, [](std::mt19937_64& g) { const F d = abs(randomLog<Real>(g, -40, -2)); return (g() & 1) ? F(Real(1)) - d : d - F(Real(1)); }, N, rng, at));
	report("acos near +-1",   worstUnits<Ref>([](const F& x) { return acos(x); }, [](const auto& v) { return Ref::acos(v); }, [](std::mt19937_64& g) { const F d = abs(randomLog<Real>(g, -40, -2)); return (g() & 1) ? F(Real(1)) - d : d - F(Real(1)); }, N, rng, at));
	// atan2 over all four quadrants and a wide ratio of the arguments
	{
		double worst = 0.0;
		for (int i = 0; i < N; ++i) {
			const F y = randomLog<Real>(rng, -30, 30), x = randomLog<Real>(rng, -30, 30);
			const double u = Ref::rel(Ref::value(atan2(y, x)), Ref::atan2(Ref::value(y), Ref::value(x))) / std::ldexp(1.0, Ref::epsExp);
			if (!(u <= worst)) { worst = u; at = y; }
		}
		report("atan2 quadrants", worst);
	}
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
	const F pinf(RL::infinity()), nan(RL::quiet_NaN()), one(Real(1)), zero(Real(0)), nzero(Real(-0.0));
	check(signbit(sin(nzero)) && iszero(sin(nzero)) && cos(zero) == one && signbit(tan(nzero)), "sin(-0) = -0, cos(0) = 1, tan(-0) = -0");
	errno = 0;
	check(isnan(sin(pinf)) && errno == EDOM, "sin(inf) = NaN with EDOM");
	errno = 0;
	check(isnan(cos(-pinf)) && errno == EDOM, "cos(-inf) = NaN with EDOM");
	check(isnan(sin(nan)) && isnan(atan(nan)) && isnan(atan2(nan, one)), "NaN in, NaN out");
	errno = 0;
	check(isnan(asin(F(Real(1.5)))) && errno == EDOM, "asin(1.5) = NaN with EDOM");
	errno = 0;
	check(isnan(acos(F(Real(-2)))) && errno == EDOM, "acos(-2) = NaN with EDOM");
	check(asin(zero) == zero && signbit(asin(nzero)) && signbit(atan(nzero)), "asin and atan of +-0");
	check(acos(one) == zero, "acos(1) = 0");
	const F pi_2 = asin(one);
	check(std::fabs(Real(pi_2) - Real(1.57079632679489661923L)) <= std::numeric_limits<Real>::epsilon(), "asin(1) = pi/2");
	check(atan2(zero, F(Real(-1))) == F(Real(2)) * pi_2 && atan2(nzero, F(Real(-1))) == F(Real(-2)) * pi_2, "atan2(+-0, -1) = +-pi");
	check(signbit(atan2(nzero, one)) && iszero(atan2(nzero, one)), "atan2(-0, 1) = -0");
	check(atan2(one, zero) == pi_2 && atan2(-one, zero) == -pi_2, "atan2(+-1, 0) = +-pi/2");
	check(atan2(one, pinf) == zero && atan2(pinf, pinf) == F(Real(0.5)) * pi_2, "atan2 with infinities");
	// the largest arguments still reduce
	check(isfinite(sin(F(RL::max()))) && isfinite(cos(std::numeric_limits<F>::max())), "sin(max) and cos(max) are finite");
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

	std::string test_suite  = "faithful sin, cos, tan, asin, acos, atan, atan2";
	std::string test_tag    = "faithful";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	faithful<double> x(1.0);
	std::cout << sin(x) << ' ' << atan(x) << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	// the bound, in units of epsilon(). Measured worst over 20000 arguments per range, gcc and
	// clang: 3.1 for faithful<float> (tan) and 3.5 for faithful<double> (tan). Through the
	// FloatingPointType sin is 10^10 to 10^51 units off, and a reduction that subtracts q pi/2
	// with a pair pi/2 is 10^4 units off on [-pi, pi] and far worse beyond, so the test fails both.
	constexpr double bound = 5.0;
	nrOfFailedTestCases += ReportTestResult(VerifyAccuracy<FloatRef>(bound), "faithful<float>", "accuracy");
	nrOfFailedTestCases += ReportTestResult(VerifyAccuracy<DoubleRef>(bound), "faithful<double>", "accuracy");
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
