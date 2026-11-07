// special_values.cpp: interval at the edges of its domain -- subnormal square roots, negative and
// unbounded arguments, the empty interval, odd powers, and stream input
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The QA sweep in #1962 found that interval lost containment or returned NaN at these edges (#1971):
//   - sqrt of a subnormal: the residual s*s - x rounded to zero below denorm_min, so an inexact
//     root was taken as exact and 1955 of the 1999 values k*denorm_min excluded their root;
//   - sqrt of a fully negative interval was [0, NaN];
//   - sqrt of interval<cfloat<16,5>> with subnormals lost containment for 1939 of its 31743
//     positive encodings, for the same reason at 11 significand bits;
//   - mid() overflowed for two large bounds, and mid() and double() of [-inf, +inf] were NaN;
//   - the empty interval (NaN bounds) compared <= and >= to everything, hull(empty, x) was
//     [nan, nan] in one order, and intersect(x, empty) was x;
//   - an odd power was up to 2x too wide across zero;
//   - operator>> overwrote its target on a failed parse and accepted a wrong delimiter.
#include <universal/utility/directives.hpp>
#include <cerrno>
#include <cmath>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <string>

#include <universal/number/interval/interval.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using sw::universal::interval;

// the sign of e*e - v, exactly, for a native Real: e*2^k and v*2^(2k) are normal for the k used,
// the product is split exactly by fma, and e is within an ulp of sqrt(v), so p - vs is exact
// (Sterbenz) and adding the product's roundoff keeps the sign
template<typename Real>
int SquareMinus(Real e, Real v) {
	const int k = (std::numeric_limits<Real>::digits == 53) ? 600 : 60;
	const Real es = std::ldexp(e, k), vs = std::ldexp(v, 2 * k);
	const Real p = es * es;
	const Real err = std::fma(es, es, -p);
	const Real d = (p - vs) + err;
	return (d > Real(0)) - (d < Real(0));
}

// does r contain sqrt(v) exactly, and is it at most one ulp wide?
template<typename Real>
bool ContainsRoot(const interval<Real>& r, Real v, bool& tight) {
	const bool loOk = r.lo() <= Real(0) || SquareMinus(r.lo(), v) <= 0;
	const bool hiOk = SquareMinus(r.hi(), v) >= 0;
	tight = (r.hi() == r.lo()) || (std::nextafter(r.lo(), std::numeric_limits<Real>::infinity()) == r.hi());
	return loOk && hiOk;
}

template<typename Real>
int VerifySubnormalSqrt(bool reportTestCases) {
	using I = interval<Real>;
	using RL = std::numeric_limits<Real>;
	int fails = 0, reported = 0;
	auto check = [&](Real v) {
		bool tight = false;
		const I r = sqrt(I(v, v));
		if (!ContainsRoot(r, v, tight) || !tight) {
			++fails;
			if (reportTestCases && reported++ < 5) std::cerr << "FAIL sqrt([" << v << "]) = " << r << (tight ? "" : " (wider than an ulp)") << '\n';
		}
	};
	// every small multiple of denorm_min, the case the sweep measured
	for (int k = 1; k < 2000; ++k) check(Real(k) * RL::denorm_min());
	// across the band where the fma residual stops being representable, up to the threshold
	// min() * 2^(2 digits) and past it
	std::mt19937_64 rng(1971);
	const int e0 = RL::min_exponent - RL::digits - 1, e1 = RL::min_exponent + 2 * RL::digits + 4;
	std::uniform_int_distribution<int> ue(e0, e1);
	std::uniform_real_distribution<Real> um(Real(1), Real(2));
	for (int i = 0; i < 20000; ++i) {
		const Real v = std::ldexp(um(rng), ue(rng));
		if (v > Real(0)) check(v);
	}
	// an interval with two subnormal ends
	{
		const Real a = Real(3) * RL::denorm_min(), b = Real(1000) * RL::denorm_min();
		const I r = sqrt(I(a, b));
		bool t1 = false, t2 = false;
		const I ra = sqrt(I(a, a)), rb = sqrt(I(b, b));
		if (!ContainsRoot(ra, a, t1) || !ContainsRoot(rb, b, t2) || r.lo() != ra.lo() || r.hi() != rb.hi()) ++fails;
	}
	return fails;
}

// cfloat with subnormals takes the double-promotion path of sqrt_enclose. Its fma residual was
// not exact at 11 significand bits either: 1939 of the 31743 positive encodings lost containment.
int VerifyCfloatSqrt(bool reportTestCases) {
	using C = sw::universal::cfloat<16, 5, std::uint16_t, true, false, false>;
	using I = interval<C>;
	int fails = 0;
	C v;
	for (unsigned raw = 1; raw < 0x7C00u; ++raw) {   // every positive finite encoding
		v.setbits(raw);
		const I r = sqrt(I(v, v));
		const double vd = double(v), lo = double(r.lo()), hi = double(r.hi());
		// exact in double: the squares of 11-bit significands fit
		const bool ok = (lo <= 0.0 || lo * lo <= vd) && hi * hi >= vd;
		if (!ok) {
			++fails;
			if (reportTestCases && fails < 5) std::cerr << "FAIL cfloat sqrt(" << vd << ") = [" << lo << ", " << hi << "]\n";
		}
	}
	return fails;
}

template<typename Real>
int VerifyNegativeSqrt(bool reportTestCases) {
	using I = interval<Real>;
	int fails = 0;
	errno = 0;
	const I neg = sqrt(I(Real(-4), Real(-1)));
	// the empty interval, both bounds NaN, with EDOM; it was [0, NaN]
	if (!(std::isnan(neg.lo()) && std::isnan(neg.hi())) || errno != EDOM) ++fails;
	const I straddle = sqrt(I(Real(-1), Real(4)));
	if (straddle.lo() != Real(0) || straddle.hi() != Real(2)) ++fails;   // the nonnegative part
	I empty;
	empty.setnan();
	if (!sqrt(empty).isnan()) ++fails;
	if (fails && reportTestCases) std::cerr << "FAIL negative sqrt: " << neg << ' ' << straddle << '\n';
	return fails;
}

template<typename Real>
int VerifyMidpoint(bool reportTestCases) {
	using I = interval<Real>;
	using RL = std::numeric_limits<Real>;
	int fails = 0;
	auto check = [&](bool ok, const char* what) { if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL " << what << '\n'; } };
	const Real inf = RL::infinity(), big = RL::max() / Real(2);
	const I large(big, RL::max());
	check(std::isfinite(large.mid()) && large.contains(large.mid()), "mid of two large bounds is finite and inside");
	check(I(-RL::max(), RL::max()).mid() == Real(0), "mid([-max, max]) = 0");
	I entire; entire.setlo(-inf); entire.sethi(inf);
	check(entire.mid() == Real(0) && double(entire) == 0.0, "mid and double of [-inf, inf] are 0");
	I left; left.setlo(-inf); left.sethi(Real(5));
	I right; right.setlo(Real(5)); right.sethi(inf);
	check(left.mid() == RL::lowest() && right.mid() == RL::max(), "a half-bounded interval's mid is the finite extreme");
	I empty; empty.setnan();
	check(std::isnan(empty.mid()), "the empty interval's mid is NaN");
	check(I(Real(1), Real(3)).mid() == Real(2), "an ordinary mid is unchanged");
	// random wide intervals near the top of the range: mid is finite and inside
	std::mt19937_64 rng(19710);
	std::uniform_real_distribution<Real> u(Real(0.25), Real(1));
	for (int i = 0; i < 1000; ++i) {
		Real a = u(rng) * RL::max(), b = u(rng) * RL::max();
		if (b < a) std::swap(a, b);
		const I x(a, b);
		if (!std::isfinite(x.mid()) || !x.contains(x.mid())) { ++fails; break; }
	}
	return fails;
}

template<typename Real>
int VerifyEmptyInterval(bool reportTestCases) {
	using I = interval<Real>;
	int fails = 0;
	auto check = [&](bool ok, const char* what) { if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL " << what << '\n'; } };
	I empty; empty.setnan();
	const I x(Real(1), Real(2)), y(Real(5), Real(6));
	check(!(empty < x) && !(empty > x) && !(empty <= x) && !(empty >= x) && !(empty == x), "the empty interval is unordered with x");
	check(!(x < empty) && !(x > empty) && !(x <= empty) && !(x >= empty) && (x != empty), "x is unordered with the empty interval");
	check(!(empty <= empty) && !(empty >= empty), "the empty interval is unordered with itself");
	check(x <= x && x >= x && x <= y && !(y <= x), "ordinary orderings are unchanged");
	check(hull(empty, x) == x && hull(x, empty) == x, "hull with the empty interval is the other operand");
	check(hull(empty, empty).isnan(), "the hull of two empty intervals is empty");
	check(hull(x, y) == I(Real(1), Real(6)), "an ordinary hull is unchanged");
	check(intersect(x, empty).isnan() && intersect(empty, x).isnan(), "intersecting with the empty interval is empty");
	check(intersect(x, y).isnan(), "disjoint intervals intersect to empty");
	check(intersect(I(Real(1), Real(4)), I(Real(3), Real(6))) == I(Real(3), Real(4)), "an ordinary intersection is unchanged");
	return fails;
}

template<typename Real>
int VerifyOddPower(bool reportTestCases) {
	using I = interval<Real>;
	int fails = 0;
	// integer endpoints: every power is exact, so the result must be exactly [lo^n, hi^n]
	for (int lo = -6; lo <= 6; ++lo) {
		for (int hi = lo; hi <= 6; ++hi) {
			for (int n = 3; n <= 7; n += 2) {
				const I r = pow(I(Real(lo), Real(hi)), n);
				const Real wantLo = std::pow(Real(lo), Real(n)), wantHi = std::pow(Real(hi), Real(n));
				if (r.lo() != wantLo || r.hi() != wantHi) {
					++fails;
					if (reportTestCases && fails < 5) std::cerr << "FAIL pow([" << lo << ", " << hi << "], " << n << ") = " << r << ", exact [" << wantLo << ", " << wantHi << "]\n";
				}
			}
		}
	}
	// inexact endpoints: the result encloses the exact range and stays within a few ulps of it
	const I x(Real(-0.3), Real(0.7));
	const I r = pow(x, 3);
	const long double lo3 = static_cast<long double>(x.lo()) * x.lo() * x.lo(), hi3 = static_cast<long double>(x.hi()) * x.hi() * x.hi();
	if (!(r.lo() <= lo3 && hi3 <= r.hi())) ++fails;
	if (r.hi() - r.lo() > Real(1.0001) * Real(hi3 - lo3)) ++fails;   // not the dependency-widened x * x^2
	// even powers and negative odd powers keep working
	if (pow(I(Real(-2), Real(3)), 2) != I(Real(0), Real(9))) ++fails;
	const I inv = pow(I(Real(2), Real(4)), -3);
	if (!(inv.lo() <= Real(1) / Real(64) && Real(1) / Real(8) <= inv.hi())) ++fails;
	return fails;
}

template<typename Real>
int VerifyStreamInput(bool reportTestCases) {
	using I = interval<Real>;
	int fails = 0;
	struct Case { const char* text; bool ok; Real lo, hi; };
	const Case cases[] = {
		{ "[1, 2]",          true,  Real(1),    Real(2) },
		{ "[1,2]",           true,  Real(1),    Real(2) },
		{ "  [ -0.5 , 3 ]",  true,  Real(-0.5), Real(3) },
		{ "[2, 2]",          true,  Real(2),    Real(2) },
		{ "bogus",           false, Real(0),    Real(0) },
		{ "[1, 2)",          false, Real(0),    Real(0) },
		{ "(1, 2]",          false, Real(0),    Real(0) },
		{ "[1; 2]",          false, Real(0),    Real(0) },
		{ "[3, 2]",          false, Real(0),    Real(0) },   // lo > hi is not an interval
		{ "[1, x]",          false, Real(0),    Real(0) },
		{ "[1, 2",           false, Real(0),    Real(0) },
		{ "",                false, Real(0),    Real(0) },
	};
	for (const Case& c : cases) {
		I v(Real(7), Real(9));
		std::istringstream in(c.text);
		in >> v;
		const bool ok = !in.fail();
		const bool pass = (ok == c.ok) && (c.ok ? (v.lo() == c.lo && v.hi() == c.hi) : (v.lo() == Real(7) && v.hi() == Real(9)));
		if (!pass) {
			++fails;
			if (reportTestCases) std::cerr << "FAIL '" << c.text << "' -> " << (ok ? "ok " : "fail ") << v << '\n';
		}
	}
	// what operator<< writes reads back
	std::mt19937_64 rng(1971);
	std::uniform_real_distribution<Real> u(Real(-1000), Real(1000));
	for (int i = 0; i < 1000; ++i) {
		Real a = u(rng), b = u(rng);
		if (b < a) std::swap(a, b);
		I x; x.set(a, b);
		std::stringstream s;
		s.precision(std::numeric_limits<Real>::max_digits10);
		s << x;
		I y;
		s >> y;
		if (s.fail() || !(x == y)) { ++fails; break; }
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

	std::string test_suite  = "interval special values";
	std::string test_tag    = "special values";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	interval<double> x(4.0e-320, 4.0e-320);
	std::cout << sqrt(x) << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifySubnormalSqrt<float>(reportTestCases), "interval<float>", "subnormal sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifySubnormalSqrt<double>(reportTestCases), "interval<double>", "subnormal sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatSqrt(reportTestCases), "interval<cfloat<16,5,sub>>", "sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifyNegativeSqrt<float>(reportTestCases), "interval<float>", "negative sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifyNegativeSqrt<double>(reportTestCases), "interval<double>", "negative sqrt");
	nrOfFailedTestCases += ReportTestResult(VerifyMidpoint<float>(reportTestCases), "interval<float>", "mid");
	nrOfFailedTestCases += ReportTestResult(VerifyMidpoint<double>(reportTestCases), "interval<double>", "mid");
	nrOfFailedTestCases += ReportTestResult(VerifyEmptyInterval<float>(reportTestCases), "interval<float>", "empty interval");
	nrOfFailedTestCases += ReportTestResult(VerifyEmptyInterval<double>(reportTestCases), "interval<double>", "empty interval");
	nrOfFailedTestCases += ReportTestResult(VerifyOddPower<float>(reportTestCases), "interval<float>", "odd power");
	nrOfFailedTestCases += ReportTestResult(VerifyOddPower<double>(reportTestCases), "interval<double>", "odd power");
	nrOfFailedTestCases += ReportTestResult(VerifyStreamInput<float>(reportTestCases), "interval<float>", "operator>>");
	nrOfFailedTestCases += ReportTestResult(VerifyStreamInput<double>(reportTestCases), "interval<double>", "operator>>");
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
	std::cerr << "Caught unexpected runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
