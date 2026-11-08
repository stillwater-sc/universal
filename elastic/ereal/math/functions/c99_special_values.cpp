// c99_special_values.cpp: C99 Annex F special values and signed zero of the ereal math library (#2006)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Every function is checked at NaN, +-inf, +-0 and the domain edges against the values C99
// Annex F gives, by class and sign: a NaN must be a NaN, -0 must be -0. The expected values
// are written out here, not read from the host libm, so the test means the same on every
// platform. The finite results are exact, or a multiple of pi checked against the stored
// constant at the configuration's precision. Before #2006, sqrt(-1) was -1, log(+inf) and
// hypot(1e200, 1e200) NaN, pow(0, -1) NaN, atan2(0, -0) NaN, fmod(5, inf) NaN, sin(-0) +0,
// and fmod(1e200, 3) was 1.4e70.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/ereal_precision.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using namespace sw::universal;

	// what a result must be: a NaN, a signed inf or zero, an exact value, or k * pi
	struct Expect {
		enum Kind { NaN, Inf, Zero, Exact, Pi } kind;
		bool   negative;
		double value;     // Exact: the value; Pi: the multiple of pi
	};
	constexpr Expect kNaN{ Expect::NaN, false, 0.0 };
	constexpr Expect kPosInf{ Expect::Inf, false, 0.0 };
	constexpr Expect kNegInf{ Expect::Inf, true, 0.0 };
	constexpr Expect kPosZero{ Expect::Zero, false, 0.0 };
	constexpr Expect kNegZero{ Expect::Zero, true, 0.0 };
	constexpr Expect exact(double v) { return Expect{ Expect::Exact, v < 0, v }; }
	constexpr Expect pi_times(double k) { return Expect{ Expect::Pi, k < 0, k }; }

	const double inf = std::numeric_limits<double>::infinity();
	const double nan = std::numeric_limits<double>::quiet_NaN();

	std::string show(double v) {
		if (std::isnan(v)) return "nan";
		if (v == 0.0) return std::signbit(v) ? "-0" : "+0";
		return std::to_string(v);
	}

	// the stored pi of the configuration of r
	template<unsigned N, typename F>
	ereal<N, F> pi_of(const ereal<N, F>&) { return ereal_pi<N, F>(); }

	template<typename Real>
	bool matches(const Real& r, const Expect& e) {
		switch (e.kind) {
		case Expect::NaN:   return r.isnan();
		case Expect::Inf:   return r.isinf() && r.signbit() == e.negative;
		case Expect::Zero:  return r.iszero() && r.signbit() == e.negative;
		case Expect::Exact: return !r.isnan() && !r.isinf() && r == Real(e.value);
		case Expect::Pi: {
			if (r.isnan() || r.isinf() || r.iszero() || r.signbit() != e.negative) return false;
			// the stored pi times an exact multiple; a few bits of slack for the quadrant arithmetic
			const Real ref = pi_of(r) * Real(e.value);
			return agrees_to_bits(r, ref, ereal_bits_of<Real>::value - 8);
		}
		}
		return false;
	}

	std::string describe(const Expect& e) {
		switch (e.kind) {
		case Expect::NaN:   return "nan";
		case Expect::Inf:   return e.negative ? "-inf" : "+inf";
		case Expect::Zero:  return e.negative ? "-0" : "+0";
		case Expect::Exact: return std::to_string(e.value);
		case Expect::Pi:    return std::to_string(e.value) + " pi";
		}
		return "?";
	}

	template<typename Real>
	int check(const char* fn, const Real& r, const Expect& e, const std::string& args, bool reportTestCases) {
		if (matches(r, e)) return 0;
		if (reportTestCases) {
			std::cerr << "FAIL: " << fn << '(' << args << ") = " << (r.isnan() ? std::string("nan") : show(double(r)))
			          << (r.iszero() ? (r.signbit() ? " (-0)" : " (+0)") : "") << ", expected " << describe(e) << '\n';
		}
		return 1;
	}

	template<typename Real>
	struct Unary { const char* name; Real (*f)(const Real&); double x; Expect e; };
	template<typename Real>
	struct Binary { const char* name; Real (*f)(const Real&, const Real&); double x; double y; Expect e; };

	template<typename Real>
	int VerifyUnary(bool reportTestCases) {
		using R = Real;
		const Unary<R> cases[] = {
			// sqrt and cbrt (F.10.4.5, F.10.4.1)
			{ "sqrt", [](const R& a) { return sqrt(a); }, nan, kNaN },
			{ "sqrt", [](const R& a) { return sqrt(a); }, inf, kPosInf },
			{ "sqrt", [](const R& a) { return sqrt(a); }, -inf, kNaN },
			{ "sqrt", [](const R& a) { return sqrt(a); }, -0.0, kNegZero },
			{ "sqrt", [](const R& a) { return sqrt(a); }, 0.0, kPosZero },
			{ "sqrt", [](const R& a) { return sqrt(a); }, -1.0, kNaN },
			{ "sqrt", [](const R& a) { return sqrt(a); }, 4.0, exact(2.0) },
			{ "cbrt", [](const R& a) { return cbrt(a); }, -0.0, kNegZero },
			{ "cbrt", [](const R& a) { return cbrt(a); }, -inf, kNegInf },
			{ "cbrt", [](const R& a) { return cbrt(a); }, nan, kNaN },
			// exponentials (F.10.3.1, F.10.3.3)
			{ "exp", [](const R& a) { return exp(a); }, -inf, kPosZero },
			{ "exp", [](const R& a) { return exp(a); }, inf, kPosInf },
			{ "exp", [](const R& a) { return exp(a); }, -0.0, exact(1.0) },
			{ "exp", [](const R& a) { return exp(a); }, nan, kNaN },
			{ "expm1", [](const R& a) { return expm1(a); }, -0.0, kNegZero },
			{ "expm1", [](const R& a) { return expm1(a); }, -inf, exact(-1.0) },
			{ "expm1", [](const R& a) { return expm1(a); }, inf, kPosInf },
			{ "expm1", [](const R& a) { return expm1(a); }, nan, kNaN },
			// logarithms (F.10.3.7 - F.10.3.9)
			{ "log", [](const R& a) { return log(a); }, inf, kPosInf },
			{ "log", [](const R& a) { return log(a); }, 0.0, kNegInf },
			{ "log", [](const R& a) { return log(a); }, -0.0, kNegInf },
			{ "log", [](const R& a) { return log(a); }, -1.0, kNaN },
			{ "log", [](const R& a) { return log(a); }, -inf, kNaN },
			{ "log", [](const R& a) { return log(a); }, 1.0, kPosZero },
			{ "log", [](const R& a) { return log(a); }, nan, kNaN },
			{ "log2", [](const R& a) { return log2(a); }, inf, kPosInf },
			{ "log2", [](const R& a) { return log2(a); }, 0.0, kNegInf },
			{ "log10", [](const R& a) { return log10(a); }, inf, kPosInf },
			{ "log10", [](const R& a) { return log10(a); }, -2.0, kNaN },
			{ "log1p", [](const R& a) { return log1p(a); }, -0.0, kNegZero },
			{ "log1p", [](const R& a) { return log1p(a); }, -1.0, kNegInf },
			{ "log1p", [](const R& a) { return log1p(a); }, -2.0, kNaN },
			{ "log1p", [](const R& a) { return log1p(a); }, inf, kPosInf },
			{ "log1p", [](const R& a) { return log1p(a); }, -inf, kNaN },
			{ "log1p", [](const R& a) { return log1p(a); }, nan, kNaN },
			// trigonometric (F.10.1)
			{ "sin", [](const R& a) { return sin(a); }, -0.0, kNegZero },
			{ "sin", [](const R& a) { return sin(a); }, inf, kNaN },
			{ "sin", [](const R& a) { return sin(a); }, nan, kNaN },
			{ "cos", [](const R& a) { return cos(a); }, -0.0, exact(1.0) },
			{ "cos", [](const R& a) { return cos(a); }, -inf, kNaN },
			{ "tan", [](const R& a) { return tan(a); }, -0.0, kNegZero },
			{ "tan", [](const R& a) { return tan(a); }, inf, kNaN },
			{ "asin", [](const R& a) { return asin(a); }, -0.0, kNegZero },
			{ "asin", [](const R& a) { return asin(a); }, nan, kNaN },
			{ "asin", [](const R& a) { return asin(a); }, 2.0, kNaN },
			{ "asin", [](const R& a) { return asin(a); }, -1.0, pi_times(-0.5) },
			{ "acos", [](const R& a) { return acos(a); }, nan, kNaN },
			{ "acos", [](const R& a) { return acos(a); }, 1.0, kPosZero },
			{ "acos", [](const R& a) { return acos(a); }, -2.0, kNaN },
			{ "acos", [](const R& a) { return acos(a); }, -1.0, pi_times(1.0) },
			{ "atan", [](const R& a) { return atan(a); }, -0.0, kNegZero },
			{ "atan", [](const R& a) { return atan(a); }, nan, kNaN },
			{ "atan", [](const R& a) { return atan(a); }, inf, pi_times(0.5) },
			{ "atan", [](const R& a) { return atan(a); }, -inf, pi_times(-0.5) },
			// hyperbolic (F.10.2)
			{ "sinh", [](const R& a) { return sinh(a); }, -0.0, kNegZero },
			{ "sinh", [](const R& a) { return sinh(a); }, -inf, kNegInf },
			{ "cosh", [](const R& a) { return cosh(a); }, -inf, kPosInf },
			{ "cosh", [](const R& a) { return cosh(a); }, -0.0, exact(1.0) },
			{ "tanh", [](const R& a) { return tanh(a); }, -0.0, kNegZero },
			{ "tanh", [](const R& a) { return tanh(a); }, inf, exact(1.0) },
			{ "tanh", [](const R& a) { return tanh(a); }, -inf, exact(-1.0) },
			{ "asinh", [](const R& a) { return asinh(a); }, -0.0, kNegZero },
			{ "asinh", [](const R& a) { return asinh(a); }, inf, kPosInf },
			{ "asinh", [](const R& a) { return asinh(a); }, -inf, kNegInf },
			{ "asinh", [](const R& a) { return asinh(a); }, nan, kNaN },
			{ "acosh", [](const R& a) { return acosh(a); }, inf, kPosInf },
			{ "acosh", [](const R& a) { return acosh(a); }, 1.0, kPosZero },
			{ "acosh", [](const R& a) { return acosh(a); }, 0.5, kNaN },
			{ "acosh", [](const R& a) { return acosh(a); }, nan, kNaN },
			{ "acosh", [](const R& a) { return acosh(a); }, -inf, kNaN },
			{ "atanh", [](const R& a) { return atanh(a); }, 1.0, kPosInf },
			{ "atanh", [](const R& a) { return atanh(a); }, -1.0, kNegInf },
			{ "atanh", [](const R& a) { return atanh(a); }, -0.0, kNegZero },
			{ "atanh", [](const R& a) { return atanh(a); }, 2.0, kNaN },
			{ "atanh", [](const R& a) { return atanh(a); }, nan, kNaN },
			// rounding to integers (F.10.6): a zero result has the sign of the argument
			{ "floor", [](const R& a) { return floor(a); }, -0.0, kNegZero },
			{ "floor", [](const R& a) { return floor(a); }, -inf, kNegInf },
			{ "floor", [](const R& a) { return floor(a); }, -0.5, exact(-1.0) },
			{ "ceil", [](const R& a) { return ceil(a); }, -0.0, kNegZero },
			{ "ceil", [](const R& a) { return ceil(a); }, -0.5, kNegZero },
			{ "ceil", [](const R& a) { return ceil(a); }, nan, kNaN },
			{ "trunc", [](const R& a) { return trunc(a); }, -0.0, kNegZero },
			{ "trunc", [](const R& a) { return trunc(a); }, -0.5, kNegZero },
			{ "trunc", [](const R& a) { return trunc(a); }, inf, kPosInf },
			{ "round", [](const R& a) { return round(a); }, -0.0, kNegZero },
			{ "round", [](const R& a) { return round(a); }, -0.3, kNegZero },
			{ "round", [](const R& a) { return round(a); }, -0.5, exact(-1.0) },
			{ "rint", [](const R& a) { return rint(a); }, -0.3, kNegZero },
			{ "rint", [](const R& a) { return rint(a); }, -0.5, kNegZero },
			{ "rint", [](const R& a) { return rint(a); }, -1.5, exact(-2.0) },
			{ "nearbyint", [](const R& a) { return nearbyint(a); }, -0.5, kNegZero },
		};
		int nrOfFailedTestCases = 0;
		for (const auto& c : cases) nrOfFailedTestCases += check(c.name, c.f(R(c.x)), c.e, show(c.x), reportTestCases);
		return nrOfFailedTestCases;
	}

	template<typename Real>
	int VerifyBinary(bool reportTestCases) {
		using R = Real;
		const double big = std::ldexp(1.0, 60);   // an even integer far beyond int, in every limb type
		const Binary<R> cases[] = {
			// pow (F.10.4.4)
			{ "pow", [](const R& a, const R& b) { return pow(a, b); }, nan, 0.0, exact(1.0) },
			{ "pow", [](const R& a, const R& b) { return pow(a, b); }, nan, -0.0, exact(1.0) },
			{ "pow", [](const R& a, const R& b) { return pow(a, b); }, 1.0, nan, exact(1.0) },
			{ "pow", [](const R& a, const R& b) { return pow(a, b); }, nan, 1.0, kNaN },
			{ "pow", [](const R& a, const R& b) { return pow(a, b); }, 2.0, nan, kNaN },
			{ "pow", [](const R& a, const R& b) { return pow(a, b); }, -1.0, inf, exact(1.0) },
			{ "pow", [](const R& a, const R& b) { return pow(a, b); }, -1.0, -inf, exact(1.0) },
			{ "pow", [](const R& a, const R& b) { return pow(a, b); }, 0.5, inf, kPosZero },
			{ "pow", [](const R& a, const R& b) { return pow(a, b); }, -0.5, -inf, kPosInf },
			{ "pow", [](const R& a, const R& b) { return pow(a, b); }, 2.0, inf, kPosInf },
			{ "pow", [](const R& a, const R& b) { return pow(a, b); }, -2.0, -inf, kPosZero },
			{ "pow", [](const R& a, const R& b) { return pow(a, b); }, 0.0, -1.0, kPosInf },
			{ "pow", [](const R& a, const R& b) { return pow(a, b); }, -0.0, -1.0, kNegInf },
			{ "pow", [](const R& a, const R& b) { return pow(a, b); }, -0.0, -2.0, kPosInf },
			{ "pow", [](const R& a, const R& b) { return pow(a, b); }, -0.0, -0.5, kPosInf },
			{ "pow", [](const R& a, const R& b) { return pow(a, b); }, 0.0, -inf, kPosInf },
			{ "pow", [](const R& a, const R& b) { return pow(a, b); }, -0.0, 3.0, kNegZero },
			{ "pow", [](const R& a, const R& b) { return pow(a, b); }, -0.0, 2.0, kPosZero },
			{ "pow", [](const R& a, const R& b) { return pow(a, b); }, -0.0, 0.5, kPosZero },
			{ "pow", [](const R& a, const R& b) { return pow(a, b); }, -inf, -3.0, kNegZero },
			{ "pow", [](const R& a, const R& b) { return pow(a, b); }, -inf, -2.0, kPosZero },
			{ "pow", [](const R& a, const R& b) { return pow(a, b); }, -inf, 3.0, kNegInf },
			{ "pow", [](const R& a, const R& b) { return pow(a, b); }, -inf, 0.5, kPosInf },
			{ "pow", [](const R& a, const R& b) { return pow(a, b); }, inf, -1.0, kPosZero },
			{ "pow", [](const R& a, const R& b) { return pow(a, b); }, inf, 0.5, kPosInf },
			{ "pow", [](const R& a, const R& b) { return pow(a, b); }, -2.0, 0.5, kNaN },
			{ "pow", [](const R& a, const R& b) { return pow(a, b); }, -2.0, 3.0, exact(-8.0) },
			{ "pow", [](const R& a, const R& b) { return pow(a, b); }, -2.0, big, kPosInf },
			{ "pow", [](const R& a, const R& b) { return pow(a, b); }, -0.5, big, kPosZero },
			// atan2 (F.10.1.4)
			{ "atan2", [](const R& a, const R& b) { return atan2(a, b); }, nan, 1.0, kNaN },
			{ "atan2", [](const R& a, const R& b) { return atan2(a, b); }, 1.0, nan, kNaN },
			{ "atan2", [](const R& a, const R& b) { return atan2(a, b); }, 0.0, 0.0, kPosZero },
			{ "atan2", [](const R& a, const R& b) { return atan2(a, b); }, -0.0, 0.0, kNegZero },
			{ "atan2", [](const R& a, const R& b) { return atan2(a, b); }, 0.0, -0.0, pi_times(1.0) },
			{ "atan2", [](const R& a, const R& b) { return atan2(a, b); }, -0.0, -0.0, pi_times(-1.0) },
			{ "atan2", [](const R& a, const R& b) { return atan2(a, b); }, 0.0, -1.0, pi_times(1.0) },
			{ "atan2", [](const R& a, const R& b) { return atan2(a, b); }, -0.0, -1.0, pi_times(-1.0) },
			{ "atan2", [](const R& a, const R& b) { return atan2(a, b); }, -0.0, 1.0, kNegZero },
			{ "atan2", [](const R& a, const R& b) { return atan2(a, b); }, 1.0, 0.0, pi_times(0.5) },
			{ "atan2", [](const R& a, const R& b) { return atan2(a, b); }, -1.0, -0.0, pi_times(-0.5) },
			{ "atan2", [](const R& a, const R& b) { return atan2(a, b); }, inf, inf, pi_times(0.25) },
			{ "atan2", [](const R& a, const R& b) { return atan2(a, b); }, -inf, -inf, pi_times(-0.75) },
			{ "atan2", [](const R& a, const R& b) { return atan2(a, b); }, inf, -2.0, pi_times(0.5) },
			{ "atan2", [](const R& a, const R& b) { return atan2(a, b); }, 1.0, inf, kPosZero },
			{ "atan2", [](const R& a, const R& b) { return atan2(a, b); }, -1.0, inf, kNegZero },
			{ "atan2", [](const R& a, const R& b) { return atan2(a, b); }, 1.0, -inf, pi_times(1.0) },
			{ "atan2", [](const R& a, const R& b) { return atan2(a, b); }, -1.0, -inf, pi_times(-1.0) },
			{ "atan2", [](const R& a, const R& b) { return atan2(a, b); }, -1.0, -1.0, pi_times(-0.75) },
			// hypot (F.10.4.3): an infinite operand wins over a NaN
			{ "hypot", [](const R& a, const R& b) { return hypot(a, b); }, inf, nan, kPosInf },
			{ "hypot", [](const R& a, const R& b) { return hypot(a, b); }, nan, -inf, kPosInf },
			{ "hypot", [](const R& a, const R& b) { return hypot(a, b); }, nan, 1.0, kNaN },
			{ "hypot", [](const R& a, const R& b) { return hypot(a, b); }, -0.0, -0.0, kPosZero },
			{ "hypot", [](const R& a, const R& b) { return hypot(a, b); }, -3.0, 4.0, exact(5.0) },
			// fmod and remainder (F.10.7)
			{ "fmod", [](const R& a, const R& b) { return fmod(a, b); }, 5.0, inf, exact(5.0) },
			{ "fmod", [](const R& a, const R& b) { return fmod(a, b); }, -5.0, -inf, exact(-5.0) },
			{ "fmod", [](const R& a, const R& b) { return fmod(a, b); }, inf, 2.0, kNaN },
			{ "fmod", [](const R& a, const R& b) { return fmod(a, b); }, nan, 2.0, kNaN },
			{ "fmod", [](const R& a, const R& b) { return fmod(a, b); }, 2.0, nan, kNaN },
			{ "fmod", [](const R& a, const R& b) { return fmod(a, b); }, -0.0, 3.0, kNegZero },
			{ "fmod", [](const R& a, const R& b) { return fmod(a, b); }, -6.0, 3.0, kNegZero },
			{ "fmod", [](const R& a, const R& b) { return fmod(a, b); }, 6.0, -3.0, kPosZero },
			{ "fmod", [](const R& a, const R& b) { return fmod(a, b); }, -7.0, 3.0, exact(-1.0) },
			{ "fmod", [](const R& a, const R& b) { return fmod(a, b); }, big + 1.0, 3.0, exact(std::fmod(big + 1.0, 3.0)) },
			{ "remainder", [](const R& a, const R& b) { return remainder(a, b); }, 5.0, inf, exact(5.0) },
			{ "remainder", [](const R& a, const R& b) { return remainder(a, b); }, -0.0, 3.0, kNegZero },
			{ "remainder", [](const R& a, const R& b) { return remainder(a, b); }, -6.0, 3.0, kNegZero },
			{ "remainder", [](const R& a, const R& b) { return remainder(a, b); }, inf, 1.0, kNaN },
			{ "remainder", [](const R& a, const R& b) { return remainder(a, b); }, 5.0, 2.0, exact(1.0) },
			{ "remainder", [](const R& a, const R& b) { return remainder(a, b); }, 7.0, 2.0, exact(-1.0) },
			{ "remainder", [](const R& a, const R& b) { return remainder(a, b); }, -5.0, 2.0, exact(-1.0) },
			{ "remainder", [](const R& a, const R& b) { return remainder(a, b); }, 7.0, -3.0, exact(1.0) },
			// copysign reads the sign bit, -0 included
			{ "copysign", [](const R& a, const R& b) { return copysign(a, b); }, 1.0, -0.0, exact(-1.0) },
			{ "copysign", [](const R& a, const R& b) { return copysign(a, b); }, -0.0, 1.0, kPosZero },
			{ "copysign", [](const R& a, const R& b) { return copysign(a, b); }, 0.0, -0.0, kNegZero },
			{ "copysign", [](const R& a, const R& b) { return copysign(a, b); }, inf, -0.0, kNegInf },
			{ "copysign", [](const R& a, const R& b) { return copysign(a, b); }, -2.0, 0.0, exact(2.0) },
			// nextafter: NaN in, NaN out
			{ "nextafter", [](const R& a, const R& b) { return nextafter(a, b); }, nan, 1.0, kNaN },
			{ "nextafter", [](const R& a, const R& b) { return nextafter(a, b); }, 1.0, nan, kNaN },
		};
		int nrOfFailedTestCases = 0;
		for (const auto& c : cases) nrOfFailedTestCases += check(c.name, c.f(R(c.x), R(c.y)), c.e, show(c.x) + ", " + show(c.y), reportTestCases);

		// the zero divisor: NaN, unless the build asks for the exception
#if !EREAL_THROW_ARITHMETIC_EXCEPTION
		nrOfFailedTestCases += check("fmod", fmod(R(5.0), R(0.0)), kNaN, "5, 0", reportTestCases);
		nrOfFailedTestCases += check("remainder", remainder(R(5.0), R(-0.0)), kNaN, "5, -0", reportTestCases);
#endif
		// the 3-argument hypot
		nrOfFailedTestCases += check("hypot", hypot(R(1.0), R(nan), R(-inf)), kPosInf, "1, nan, -inf", reportTestCases);
		nrOfFailedTestCases += check("hypot", hypot(R(2.0), R(-3.0), R(6.0)), exact(7.0), "2, -3, 6", reportTestCases);
		return nrOfFailedTestCases;
	}

	// modf: the fraction carries the sign of x when it is zero (F.10.3.12)
	template<typename Real>
	int VerifyModf(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		struct Case { double x; Expect frac; Expect ipart; };
		const Case cases[] = {
			{ -3.0, kNegZero, exact(-3.0) },
			{ -0.0, kNegZero, kNegZero },
			{ -0.5, exact(-0.5), kNegZero },
			{ -inf, kNegZero, kNegInf },
			{ inf, kPosZero, kPosInf },
			{ nan, kNaN, kNaN },
		};
		for (const auto& c : cases) {
			Real ip;
			const Real fr = modf(Real(c.x), &ip);
			nrOfFailedTestCases += check("modf fraction", fr, c.frac, show(c.x), reportTestCases);
			nrOfFailedTestCases += check("modf integer part", ip, c.ipart, show(c.x), reportTestCases);
		}
		return nrOfFailedTestCases;
	}

	// fmod and remainder are exact, whatever the size of the quotient. The host's fmod and
	// remainder are exact too (IEEE 754), so for limb-type arguments they are the reference.
	template<typename Real, typename Host>
	int VerifyExactRemainders(bool reportTestCases, int nrCases) {
		int nrOfFailedTestCases = 0;
		std::mt19937_64 rng(0x2006);
		std::uniform_real_distribution<double> mant(1.0, 2.0);
		const int span = std::numeric_limits<Host>::max_exponent - 2;
		std::uniform_int_distribution<int> expo(-span, span);
		// each draw in a fixed order: argument evaluation order is unspecified in C++, and gcc
		// and clang would otherwise generate different cases from the same seed
		auto draw = [&](int expoDivisor, bool withSign) {
			const double m = mant(rng);
			const int e = expo(rng) / expoDivisor;
			const double s = (withSign && (rng() & 1)) ? -1.0 : 1.0;
			return static_cast<Host>(std::ldexp(m, e) * s);
		};
		for (int i = 0; i < nrCases; ++i) {
			const Host x = draw(1, true);
			Host y = draw(1, true);
			if (i % 3 == 0) y = draw(16, false);   // quotients of every size
			const Host f = std::fmod(x, y), r = std::remainder(x, y);
			const Real ef = fmod(Real(x), Real(y)), er = remainder(Real(x), Real(y));
			if (!(ef == Real(f)) || ef.signbit() != std::signbit(f)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: fmod(" << x << ", " << y << ") = " << double(ef) << ", expected " << double(f) << '\n';
			}
			if (!(er == Real(r)) || er.signbit() != std::signbit(r)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: remainder(" << x << ", " << y << ") = " << double(er) << ", expected " << double(r) << '\n';
			}
		}
		// ties go to the even quotient
		for (int k = -6; k <= 6; ++k) {
			const Host x = static_cast<Host>(k) + Host(0.5);
			if (!(remainder(Real(x), Real(1.0)) == Real(std::remainder(x, Host(1))))) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: remainder(" << x << ", 1) tie\n";
			}
		}
		// a remainder that needs the low limbs: (1 + 2^-150) mod 2^-100 = 2^-150
		const Real x = Real(1.0) + Real(std::ldexp(1.0, -150));
		if (std::numeric_limits<Host>::digits >= 53 && !(fmod(x, Real(std::ldexp(1.0, -100))) == Real(std::ldexp(1.0, -150)))) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: fmod(1 + 2^-150, 2^-100) != 2^-150\n";
		}
		return nrOfFailedTestCases;
	}

	// hypot no longer squares past the limb type's range: hypot(1e200, 1e200) = sqrt(2) 1e200.
	// The small operand is 1e-100, not 1e-200: twelve limbs below 1e-200 would reach under
	// DBL_MIN, where no ereal, the reference included, carries the bits.
	template<typename Real>
	int VerifyHypotRange(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		const Real a(1e200), tiny(1e-100);
		const Real h = hypot(a, a), ht = hypot(tiny, tiny);
		const Real s2 = sqrt(Real(2.0));
		const int bits = ereal_bits_of<Real>::value - 8;
		if (!agrees_to_bits(h, a * s2, bits)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: hypot(1e200, 1e200) = " << double(h) << '\n';
		}
		if (!agrees_to_bits(ht, tiny * s2, bits)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: hypot(1e-100, 1e-100) = " << double(ht) << '\n';
		}
		if (!(hypot(Real(1e300), Real(1e300), Real(1e300)).isinf() == false)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: hypot(1e300, 1e300, 1e300) overflowed\n";
		}
		return nrOfFailedTestCases;
	}

}  // namespace

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
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "ereal C99 special values";
	std::string test_tag    = "c99 special values";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	using Real = ereal<4>;
	std::cout << "atan2(0, -0) = " << double(atan2(Real(0.0), Real(-0.0))) << '\n';
	std::cout << "fmod(1e200, 3) = " << double(fmod(Real(1e200), Real(3.0))) << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyUnary<ereal<4>>(reportTestCases), "ereal<4>", "unary");
	nrOfFailedTestCases += ReportTestResult(VerifyBinary<ereal<4>>(reportTestCases), "ereal<4>", "binary");
	nrOfFailedTestCases += ReportTestResult(VerifyModf<ereal<4>>(reportTestCases), "ereal<4>", "modf");
	nrOfFailedTestCases += ReportTestResult(VerifyExactRemainders<ereal<4>, double>(reportTestCases, 2000), "ereal<4>", "exact fmod/remainder");
	nrOfFailedTestCases += ReportTestResult(VerifyHypotRange<ereal<4>>(reportTestCases), "ereal<4>", "hypot range");

	nrOfFailedTestCases += ReportTestResult(VerifyUnary<ereal<4, float>>(reportTestCases), "ereal<4, float>", "unary");
	nrOfFailedTestCases += ReportTestResult(VerifyBinary<ereal<4, float>>(reportTestCases), "ereal<4, float>", "binary");
	nrOfFailedTestCases += ReportTestResult(VerifyModf<ereal<4, float>>(reportTestCases), "ereal<4, float>", "modf");
	nrOfFailedTestCases += ReportTestResult(VerifyExactRemainders<ereal<4, float>, float>(reportTestCases, 2000), "ereal<4, float>", "exact fmod/remainder");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyUnary<ereal<12>>(reportTestCases), "ereal<12>", "unary");
	nrOfFailedTestCases += ReportTestResult(VerifyBinary<ereal<12>>(reportTestCases), "ereal<12>", "binary");
	nrOfFailedTestCases += ReportTestResult(VerifyExactRemainders<ereal<8>, double>(reportTestCases, 20000), "ereal<8>", "exact fmod/remainder");
	nrOfFailedTestCases += ReportTestResult(VerifyHypotRange<ereal<12>>(reportTestCases), "ereal<12>", "hypot range");
#if LONG_DOUBLE_SUPPORT
	nrOfFailedTestCases += ReportTestResult(VerifyUnary<ereal<3, long double>>(reportTestCases), "ereal<3, long double>", "unary");
	nrOfFailedTestCases += ReportTestResult(VerifyBinary<ereal<3, long double>>(reportTestCases), "ereal<3, long double>", "binary");
#endif
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
	std::cerr << "Uncaught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Uncaught unexpected universal internal exception: " << err.what() << std::endl;
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
