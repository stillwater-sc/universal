// hyperbolic_and_power.cpp: test suite runner for faithful's hyperbolic functions, pow, cbrt and hypot
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// sinh, cosh, tanh, asinh, acosh, atanh, pow, cbrt and hypot are compensated (#1957). Each is
// measured against a reference with more than the pair's precision: dd for
// faithful<float>, qd for faithful<double>. qd has no cbrt, so cbrt is measured by its residual,
// |r^3 - x| / (3 |x|) in qd, which is the relative error of r to first order. The error is in
// units of the pair's epsilon().
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
	static Exact sinh(const Exact& v) { return sw::universal::sinh(v); }
	static Exact cosh(const Exact& v) { return sw::universal::cosh(v); }
	static Exact tanh(const Exact& v) { return sw::universal::tanh(v); }
	static Exact asinh(const Exact& v) { return sw::universal::asinh(v); }
	static Exact acosh(const Exact& v) { return sw::universal::acosh(v); }
	static Exact atanh(const Exact& v) { return sw::universal::atanh(v); }
	static Exact pow(const Exact& x, const Exact& y) { return sw::universal::pow(x, y); }
	static Exact hypot(const Exact& x, const Exact& y) { return sw::universal::hypot(x, y); }
	static double rel(const Exact& got, const Exact& want) { return double(abs((got - want) / want)); }
	static double cbrtRel(const faithful<float>& r, const faithful<float>& x) {
		const Exact rv = value(r), xv = value(x);
		return double(abs((rv * rv * rv - xv) / (Exact(3.0) * xv)));
	}
};
struct DoubleRef {
	using Real = double;
	using Exact = qd;
	static constexpr int epsExp = -105;
	static Exact value(const faithful<double>& v) { return qd(v.high()) + qd(v.low()); }
	static Exact sinh(const Exact& v) { return sw::universal::sinh(v); }
	static Exact cosh(const Exact& v) { return sw::universal::cosh(v); }
	static Exact tanh(const Exact& v) { return sw::universal::tanh(v); }
	static Exact asinh(const Exact& v) { return sw::universal::asinh(v); }
	static Exact acosh(const Exact& v) { return sw::universal::acosh(v); }
	static Exact atanh(const Exact& v) { return sw::universal::atanh(v); }
	static Exact pow(const Exact& x, const Exact& y) { return sw::universal::pow(x, y); }
	static Exact hypot(const Exact& x, const Exact& y) { return sw::universal::hypot(x, y); }
	static double rel(const Exact& got, const Exact& want) { return double(abs((got - want) / want)); }
	static double cbrtRel(const faithful<double>& r, const faithful<double>& x) {
		const qd rv = value(r), xv = value(x);
		return double(abs((rv * rv * rv - xv) / (qd(3.0) * xv)));
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
faithful<Real> randomLog(std::mt19937_64& rng, int e0, int e1, bool signs) {
	std::uniform_real_distribution<Real> mant(Real(1), Real(2));
	std::uniform_int_distribution<int> expo(e0, e1 - 1);
	std::uniform_real_distribution<Real> frac(Real(-0.5), Real(0.5));
	Real v = std::ldexp(mant(rng), expo(rng));
	const Real ulp = std::nextafter(v, std::numeric_limits<Real>::infinity()) - v;
	faithful<Real> x(v, frac(rng) * ulp);
	return (signs && (rng() & 1)) ? -x : x;
}

template<typename Ref>
int VerifyAccuracy(double bound, double powBound) {
	using Real = typename Ref::Real;
	using F = faithful<Real>;
	using namespace sw::universal;
	const Real expMax = std::log(std::numeric_limits<Real>::max()) * Real(0.98);
	const int eMax = std::numeric_limits<Real>::max_exponent - 2;
	const int eMin = std::numeric_limits<Real>::min_exponent + std::numeric_limits<Real>::digits + 2;
	std::mt19937_64 rng(1957);
	int fails = 0;
	F at;
	auto report = [&](const char* name, double worst, double b) {
		std::cout << "  " << type_tag(F()) << ' ' << name << " worst " << worst << " units";
		if (worst <= b) { std::cout << '\n'; return; }
		++fails;
		std::cout << "  FAIL at " << at << '\n';
	};
	constexpr int N = 20000;
	auto unary = [&](const char* name, auto f, auto ref, auto gen, double b) {
		double worst = 0.0;
		for (int i = 0; i < N; ++i) {
			const F x = gen(rng);
			const double u = Ref::rel(Ref::value(f(x)), ref(Ref::value(x))) / std::ldexp(1.0, Ref::epsExp);
			if (std::isnan(u) ? !std::isnan(worst) : (!std::isnan(worst) && u > worst)) { worst = u; at = x; }   // a NaN, once seen, sticks
		}
		report(name, worst, b);
	};
	auto small = [](std::mt19937_64& g) { return randomLog<Real>(g, -40, -1, true); };
	unary("sinh small",       [](const F& x) { return sinh(x); }, [](const auto& v) { return Ref::sinh(v); }, small, bound);
	unary("sinh full range",  [](const F& x) { return sinh(x); }, [](const auto& v) { return Ref::sinh(v); }, [&](std::mt19937_64& g) { return randomIn<Real>(g, -expMax, expMax); }, bound);
	unary("cosh full range",  [](const F& x) { return cosh(x); }, [](const auto& v) { return Ref::cosh(v); }, [&](std::mt19937_64& g) { return randomIn<Real>(g, -expMax, expMax); }, bound);
	unary("tanh small",       [](const F& x) { return tanh(x); }, [](const auto& v) { return Ref::tanh(v); }, small, bound);
	unary("tanh [-40, 40]",   [](const F& x) { return tanh(x); }, [](const auto& v) { return Ref::tanh(v); }, [](std::mt19937_64& g) { return randomIn<Real>(g, Real(-40), Real(40)); }, bound);
	unary("asinh small",      [](const F& x) { return asinh(x); }, [](const auto& v) { return Ref::asinh(v); }, small, bound);
	unary("asinh wide",       [](const F& x) { return asinh(x); }, [](const auto& v) { return Ref::asinh(v); }, [&](std::mt19937_64& g) { return randomLog<Real>(g, -10, eMax, true); }, bound);
	unary("acosh near 1",     [](const F& x) { return acosh(x); }, [](const auto& v) { return Ref::acosh(v); }, [](std::mt19937_64& g) { return F(Real(1)) + randomLog<Real>(g, -40, -1, false); }, bound);
	unary("acosh wide",       [](const F& x) { return acosh(x); }, [](const auto& v) { return Ref::acosh(v); }, [&](std::mt19937_64& g) { return F(Real(1)) + randomLog<Real>(g, 0, eMax - 1, false); }, bound);
	unary("atanh small",      [](const F& x) { return atanh(x); }, [](const auto& v) { return Ref::atanh(v); }, small, bound);
	unary("atanh near +-1",   [](const F& x) { return atanh(x); }, [](const auto& v) { return Ref::atanh(v); }, [](std::mt19937_64& g) { const F d = randomLog<Real>(g, -40, -2, false); return (g() & 1) ? F(Real(1)) - d : d - F(Real(1)); }, bound);
	// cbrt by its residual
	{
		double worst = 0.0;
		for (int i = 0; i < N; ++i) {
			const F x = randomLog<Real>(rng, eMin, eMax, true);
			const double u = Ref::cbrtRel(cbrt(x), x) / std::ldexp(1.0, Ref::epsExp);
			if (std::isnan(u) ? !std::isnan(worst) : (!std::isnan(worst) && u > worst)) { worst = u; at = x; }   // a NaN, once seen, sticks
		}
		report("cbrt full range", worst, bound);
	}
	// hypot over a wide ratio of the arguments, including near the ends of the range
	{
		double worst = 0.0;
		for (int i = 0; i < N; ++i) {
			const int e0 = (i % 2) ? eMin : -40, e1 = (i % 2) ? eMax : 40;
			const F x = randomLog<Real>(rng, e0, e1, true), y = randomLog<Real>(rng, e0, e1, true);
			const double u = Ref::rel(Ref::value(hypot(x, y)), Ref::hypot(Ref::value(x), Ref::value(y))) / std::ldexp(1.0, Ref::epsExp);
			if (std::isnan(u) ? !std::isnan(worst) : (!std::isnan(worst) && u > worst)) { worst = u; at = x; }   // a NaN, once seen, sticks
		}
		report("hypot", worst, bound);
	}
	// pow's error grows as pow.hpp documents, so each sample is held to its own bound,
	// bound + powBound * g, with g the measure of the path the sample takes: |y| for an integer y
	// with |y| <= 64, which takes binary powering, and |y log x| for the exp/log path. The report
	// gives the worst error and the worst slope, (error - bound) / g, the growth per unit of g.
	auto powCheck = [&](const char* name, auto genX, auto genY) {
		double worst = 0.0, slope = 0.0, excess = -1.0;
		for (int i = 0; i < N; ++i) {
			const F x = genX(rng), y = genY(rng);
			const double u = Ref::rel(Ref::value(pow(x, y)), Ref::pow(Ref::value(x), Ref::value(y))) / std::ldexp(1.0, Ref::epsExp);
			const double yv = double(y.high()), xv = double(x.high());
			const bool binary = (std::floor(yv) == yv) && y.low() == Real(0) && std::fabs(yv) <= 64.0;
			const double g = binary ? std::fabs(yv) : std::fabs(yv * std::log(xv));
			const double allowed = bound + powBound * g;
			if (std::isnan(u)) { worst = u; at = x; excess = u; continue; }
			if (!std::isnan(worst) && u > worst) worst = u;
			if (g > 1.0 && (u - bound) / g > slope) slope = (u - bound) / g;
			if (!std::isnan(excess) && u - allowed > excess) { excess = u - allowed; at = x; }
		}
		std::cout << "  " << type_tag(F()) << ' ' << name << " worst " << worst << " units, slope " << slope;
		if (!std::isnan(excess) && excess <= 0.0) { std::cout << '\n'; return; }
		++fails;
		std::cout << "  FAIL at " << at << '\n';
	};
	auto uniform = [](Real lo, Real hi) { return [=](std::mt19937_64& g) { return randomIn<Real>(g, lo, hi); }; };
	auto integer = [](int lo, int hi) { return [=](std::mt19937_64& g) { std::uniform_int_distribution<int> n(lo, hi); return F(Real(n(g))); }; };
	powCheck("pow x in [0.5, 2], |y| <= 1",    uniform(Real(0.5), Real(2)),    uniform(Real(-1), Real(1)));
	powCheck("pow x in [0.5, 2], |y| <= 50",   uniform(Real(0.5), Real(2)),    uniform(Real(-50), Real(50)));
	powCheck("pow x in [1e-6, 1e6], |y| <= 2", uniform(Real(1e-6), Real(1e6)), uniform(Real(-2), Real(2)));
	// integer exponents beyond +-64 take the exp/log path; |y log x| up to about 49
	powCheck("pow integer y in [-1000, 1000]", uniform(Real(0.95), Real(1.05)), integer(-1000, 1000));
	// integer exponents up to +-64: binary powering, whose error grows with |y|
	powCheck("pow integer y in [-64, 64]",     uniform(Real(0.5), Real(2)),    integer(-64, 64));
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
	const F pinf(RL::infinity()), nan(RL::quiet_NaN()), one(Real(1)), zero(Real(0)), nzero(Real(-0.0)), two(Real(2));
	check(signbit(sinh(nzero)) && cosh(zero) == one && signbit(tanh(nzero)) && signbit(asinh(nzero)) && signbit(atanh(nzero)), "the zeros");
	check(isinf(sinh(-pinf)) && signbit(sinh(-pinf)) && isinf(cosh(-pinf)) && !signbit(cosh(-pinf)), "sinh and cosh of -inf");
	check(tanh(pinf) == one && tanh(-pinf) == -one && tanh(F(Real(1000))) == one, "tanh saturates");
	errno = 0;
	check(isnan(acosh(F(Real(0.5)))) && errno == EDOM, "acosh(0.5) = NaN with EDOM");
	check(acosh(one) == zero, "acosh(1) = 0");
	errno = 0;
	check(isinf(atanh(one)) && errno == ERANGE && signbit(atanh(-one)), "atanh(+-1) = +-inf with ERANGE");
	errno = 0;
	check(isnan(atanh(two)) && errno == EDOM, "atanh(2) = NaN with EDOM");
	errno = 0;
	check(isinf(cosh(F(RL::max()))) && errno == ERANGE, "cosh overflow is inf with ERANGE");
	// sinh and cosh at std::log(max) rounded to Real, which for float lies above the true ln(max)
	{
		const F x(std::log(RL::max()));
		errno = 0;
		const F s = sinh(x), c = cosh(x);
		check(isfinite(s) && isfinite(c) && errno == 0, "sinh and cosh at log(max) are finite, without ERANGE");
	}
	// sinh just below its overflow point: e^x overflows, sinh(x) does not
	{
		const F x(std::log(RL::max()) + Real(0.5));
		check(isfinite(sinh(x)) && isfinite(cosh(x)), "sinh and cosh past exp's overflow point are finite");
	}
	// cbrt
	{
		// a perfect cube comes back to within the pair's precision; std::cbrt itself can be an ulp off
		const Real tol = Real(4) * Real(std::numeric_limits<F>::epsilon());
		auto near = [&](const F& got, Real want) { return std::fabs(Real(got - F(want))) <= tol * std::fabs(want); };
		check(near(cbrt(F(Real(27))), Real(3)) && near(cbrt(F(Real(-8))), Real(-2)) && near(cbrt(F(Real(0.125))), Real(0.5)), "cbrt of perfect cubes");
	}
	check(signbit(cbrt(nzero)) && isinf(cbrt(-pinf)) && isnan(cbrt(nan)), "cbrt of -0, -inf, NaN");
	// hypot
	check(hypot(F(Real(3)), F(Real(4))) == F(Real(5)), "hypot(3, 4) = 5");
	check(isinf(hypot(pinf, nan)) && isinf(hypot(nan, -pinf)) && isnan(hypot(nan, one)), "hypot with inf and NaN");
	check(isfinite(hypot(F(RL::max() / 2), F(RL::max() / 4))), "hypot near max() does not overflow");
	check(isinf(hypot(F(RL::max()), F(RL::max() / 2))), "hypot beyond max() is inf");
	check(hypot(F(RL::denorm_min()), zero) == F(RL::denorm_min()), "hypot of a subnormal");
	// pow: C99 F.10.4.4
	check(pow(nan, zero) == one && pow(one, nan) == one && isnan(pow(two, nan)), "pow with NaN");
	check(pow(-one, pinf) == one && pow(F(Real(0.5)), pinf) == zero && isinf(pow(two, pinf)) && pow(two, -pinf) == zero, "pow with an infinite exponent");
	check(isinf(pow(pinf, two)) && pow(pinf, -two) == zero && signbit(pow(-pinf, F(Real(-3)))) && isinf(pow(-pinf, F(Real(3)))), "pow with an infinite base");
	errno = 0;
	check(isinf(pow(zero, -one)) && errno == ERANGE && signbit(pow(nzero, F(Real(-3)))), "pow(+-0, negative) = +-inf with ERANGE");
	check(pow(zero, two) == zero && signbit(pow(nzero, F(Real(3)))) && !signbit(pow(nzero, two)), "pow(+-0, positive)");
	errno = 0;
	check(isnan(pow(-two, F(Real(0.5)))) && errno == EDOM, "pow(-2, 0.5) = NaN with EDOM");
	check(pow(-two, F(Real(3))) == F(Real(-8)) && pow(-two, F(Real(4))) == F(Real(16)), "pow of a negative base, integer y");
	check(pow(F(Real(3)), F(Real(4))) == F(Real(81)) && pow(two, F(Real(-10))) == F(std::ldexp(Real(1), -10)), "pow with a small integer y is exact");
	check(pow(two, F(Real(100))) == F(std::ldexp(Real(1), 100)), "pow(2, 100) is exact");
	errno = 0;
	check(isinf(pow(F(Real(10)), F(Real(1.0e6)))) && errno == ERANGE, "pow overflow is inf with ERANGE");
	check(iszero(pow(F(Real(10)), F(Real(-1.0e6)))), "pow underflow is 0");
	errno = 0;
	check(isinf(pow(F(Real(1.0e10)), F(Real(64)))) && errno == ERANGE, "pow overflow with an integer y is inf with ERANGE");
	{
		// exp(y log(b)) alone is past exp's range, and 2^n brings the result back: 0.7^270 for
		// float, 0.7^2000 for double, both nonzero
		const F y(Real(std::numeric_limits<Real>::digits == 24 ? 270 : 2000));
		const F r = pow(F(Real(0.7)), y);
		check(isfinite(r) && !iszero(r), "pow(0.7, y) with exp(w) past exp's range is finite and nonzero");
	}
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

	std::string test_suite  = "faithful sinh, cosh, tanh, asinh, acosh, atanh, pow, cbrt, hypot";
	std::string test_tag    = "faithful";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	faithful<double> x(1.0);
	std::cout << sinh(x) << ' ' << pow(x, x) << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	// the bounds, in units of epsilon(). Measured worst over 20000 arguments per range, gcc and
	// clang: 3.6 for faithful<float> and 3.9 for faithful<double>, except pow, whose error grows
	// with |y log x| (or |y| for binary powering) as pow.hpp documents. Each pow sample is held to
	// bound + powBound * that. The measured slope is at most 2.7 on the exp/log path and 1.0 for
	// binary powering, gcc and clang; powBound = 3 leaves room for other platforms' samples. Through the FloatingPointType sinh and cbrt are 10^7 to
	// 10^18 units off, so the test fails them.
	constexpr double bound = 5.0, powBound = 3.0;
	nrOfFailedTestCases += ReportTestResult(VerifyAccuracy<FloatRef>(bound, powBound), "faithful<float>", "accuracy");
	nrOfFailedTestCases += ReportTestResult(VerifyAccuracy<DoubleRef>(bound, powBound), "faithful<double>", "accuracy");
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
