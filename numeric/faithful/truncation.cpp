// truncation.cpp: test suite runner for faithful's floor, ceil, trunc, round and fmod
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The integer-valued functions and fmod are exact (#1954). Each is checked against an exact
// reference: a faithful<float> sums exactly into a double, where std::floor and std::fmod are
// exact, and a faithful<double> sums exactly into a qd, whose floor and fmod are exact (#1667).
// The arguments include values above 2^digits(Real), where the value part is an integer and the
// fraction sits in the error part, so evaluating through the FloatingPointType fails.
#include <universal/utility/directives.hpp>
#include <universal/number/faithful/faithful.hpp>
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

// the exact reference arithmetic of each FloatingPointType
struct FloatRef {
	using Real = float;
	using Exact = double;
	static Exact value(const faithful<float>& v) { return double(v.high()) + double(v.low()); }
	static Exact floor(Exact v) { return std::floor(v); }
	static Exact ceil(Exact v) { return std::ceil(v); }
	static Exact trunc(Exact v) { return std::trunc(v); }
	static Exact round(Exact v) { return std::round(v); }
	static Exact fmod(Exact a, Exact b) { return std::fmod(a, b); }
	static double toDouble(Exact v) { return v; }
};
struct DoubleRef {
	using Real = double;
	using Exact = qd;
	static Exact value(const faithful<double>& v) { return qd(v.high()) + qd(v.low()); }
	static Exact floor(const Exact& v) { return sw::universal::floor(v); }
	static Exact ceil(const Exact& v) { return sw::universal::ceil(v); }
	static Exact trunc(const Exact& v) { return sw::universal::trunc(v); }
	static Exact round(const Exact& v) { return sw::universal::round(v); }
	static Exact fmod(const Exact& a, const Exact& b) { return sw::universal::fmod(a, b); }
	static double toDouble(const Exact& v) { return double(v); }
};

// a pair (v, e) whose value part is an integer above 2^digits(Real) and whose error part has a
// fraction: the case where rounding to Real first gives the wrong integer
template<typename Real>
faithful<Real> bigWithFraction(std::mt19937_64& rng) {
	constexpr int digits = std::numeric_limits<Real>::digits;
	std::uniform_int_distribution<int> extra(1, digits / 2);
	std::uniform_real_distribution<Real> mant(Real(1), Real(2));
	std::uniform_real_distribution<Real> frac(Real(-0.49), Real(0.49));
	std::bernoulli_distribution neg(0.5);
	Real v = std::ldexp(std::floor(std::ldexp(mant(rng), digits - 1)), extra(rng));   // an integer, 2^(digits + extra) or so
	// an error part below half an ulp of v, with a fraction: an integer offset plus a fraction
	const Real halfUlp = std::ldexp(Real(1), std::ilogb(v) - digits);
	std::uniform_int_distribution<int> off(-static_cast<int>(std::fmin(Real(1000), halfUlp - 1)), static_cast<int>(std::fmin(Real(1000), halfUlp - 1)));
	Real e = Real(off(rng)) + frac(rng);
	if (neg(rng)) { v = -v; e = -e; }
	return faithful<Real>(v, e);
}

// a canonical pair of ordinary magnitude, with the error part at most half an ulp of the value
template<typename Real>
faithful<Real> ordinary(std::mt19937_64& rng, int minExp, int maxExp) {
	std::uniform_real_distribution<Real> mant(Real(1), Real(2));
	std::uniform_int_distribution<int>   expo(minExp, maxExp);
	std::uniform_real_distribution<Real> frac(Real(-0.5), Real(0.5));
	std::bernoulli_distribution          neg(0.5);
	Real v = std::ldexp(mant(rng), expo(rng));
	if (neg(rng)) v = -v;
	const Real ulp = std::nextafter(std::fabs(v), std::numeric_limits<Real>::infinity()) - std::fabs(v);
	return faithful<Real>(v, frac(rng) * ulp);
}

template<typename Ref>
int VerifyIntegerValued(bool reportTestCases) {
	using Real = typename Ref::Real;
	using F = faithful<Real>;
	using namespace sw::universal;
	int fails = 0, reported = 0;
	auto checkOne = [&](const F& x) {
		const auto ex = Ref::value(x);
		const F got[4] = { floor(x), ceil(x), trunc(x), round(x) };
		const decltype(ex) want[4] = { Ref::floor(ex), Ref::ceil(ex), Ref::trunc(ex), Ref::round(ex) };
		static const char* names[4] = { "floor", "ceil", "trunc", "round" };
		for (int i = 0; i < 4; ++i) {
			if (Ref::value(got[i]) == want[i]) continue;
			++fails;
			if (reportTestCases && reported++ < 8)
				std::cerr << "FAIL " << type_tag(F()) << ' ' << names[i] << x << " = " << got[i]
				          << ", expected " << Ref::toDouble(want[i]) << '\n';
		}
	};
	// halfway cases and small values
	for (Real v : { Real(0.5), Real(1.5), Real(2.5), Real(-0.5), Real(-1.5), Real(-2.5), Real(0.3), Real(-0.3), Real(0.7), Real(-0.7), Real(1), Real(-1), Real(42) })
		checkOne(F(v));
	// an integer value part with a small error: the error decides
	const Real tiny = std::numeric_limits<Real>::epsilon() / 8;
	for (Real v : { Real(1), Real(-1), Real(7), Real(-7) })
		for (Real e : { tiny, -tiny })
			checkOne(F(v, e));
	// 2^digits + 0.5: the halfway case lives in the error part
	{
		const Real big = std::ldexp(Real(1), std::numeric_limits<Real>::digits + 4);
		checkOne(F(big, Real(0.5)));
		checkOne(F(big, Real(-0.5)));
		checkOne(F(-big, Real(0.5)));
		checkOne(F(big, Real(0.25)));
		checkOne(F(big, Real(-0.25)));
	}
	std::mt19937_64 rng(1954);
	int naiveMisses = 0;   // samples where floor through Real gives the wrong integer
	for (int i = 0; i < 40000; ++i) {
		const F big = bigWithFraction<Real>(rng);
		checkOne(big);
		if (Ref::value(F(std::floor(Real(big)))) != Ref::floor(Ref::value(big))) ++naiveMisses;
		checkOne(ordinary<Real>(rng, -4, std::numeric_limits<Real>::digits + 8));
	}
	// the generator must reach the case the exact algorithm exists for, or the loop proves nothing
	if (naiveMisses == 0) {
		++fails;
		if (reportTestCases) std::cerr << "FAIL " << type_tag(F()) << " no sample where floor through Real is wrong\n";
	}

	// the special values and the signed zeros
	auto check = [&](bool ok, const char* what) {
		if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL " << type_tag(F()) << ' ' << what << '\n'; }
	};
	const F pinf(std::numeric_limits<Real>::infinity()), nan(std::numeric_limits<Real>::quiet_NaN());
	check(isinf(floor(pinf)) && isinf(ceil(-pinf)) && isinf(trunc(pinf)) && isinf(round(pinf)), "infinities");
	check(isnan(floor(nan)) && isnan(ceil(nan)) && isnan(trunc(nan)) && isnan(round(nan)), "NaN");
	check(signbit(floor(F(Real(-0.0)))) && !signbit(floor(F(Real(0)))), "floor(+-0) = +-0");
	check(signbit(ceil(F(Real(-0.5)))) && iszero(ceil(F(Real(-0.5)))), "ceil(-0.5) = -0");
	check(signbit(trunc(F(Real(-0.5)))) && iszero(trunc(F(Real(-0.5)))), "trunc(-0.5) = -0");
	check(signbit(round(F(Real(-0.3)))) && iszero(round(F(Real(-0.3)))), "round(-0.3) = -0");
	check(!signbit(floor(F(Real(1), Real(-1)))), "floor of the cancelling pair (1, -1) = +0");
	return fails;
}

template<typename Ref>
int VerifyFmod(bool reportTestCases, double relTol) {
	using Real = typename Ref::Real;
	using F = faithful<Real>;
	using namespace sw::universal;
	int fails = 0, reported = 0;
	auto checkOne = [&](const F& x, const F& y) {
		const auto want = Ref::fmod(Ref::value(x), Ref::value(y));
		const F got = fmod(x, y);
		const double w = Ref::toDouble(want);
		const double diff = std::fabs(Ref::toDouble(Ref::value(got) - want));
		const bool signOk = !iszero(got) || signbit(got) == signbit(x);   // a zero remainder keeps x's sign
		if (diff <= relTol * std::fabs(w) && signOk) return;
		++fails;
		if (reportTestCases && reported++ < 8)
			std::cerr << "FAIL " << type_tag(F()) << " fmod(" << x << ", " << y << ") = " << got << ", expected " << w << '\n';
	};
	// exact multiples, x < y, signs
	checkOne(F(Real(6)), F(Real(3)));
	checkOne(F(Real(-6)), F(Real(3)));
	checkOne(F(Real(7)), F(Real(3)));
	checkOne(F(Real(-7)), F(Real(3)));
	checkOne(F(Real(7)), F(Real(-3)));
	checkOne(F(Real(1)), F(Real(3)));
	checkOne(F(Real(5.5)), F(Real(1.25)));
	// a divisor whose error part matters
	{
		const Real e = std::ldexp(Real(1), -std::numeric_limits<Real>::digits - 4);
		checkOne(F(std::ldexp(Real(1), 40)), F(Real(1), e));
		checkOne(F(std::ldexp(Real(3), std::numeric_limits<Real>::digits + 10), Real(1)), F(Real(1.5), -e));
	}
	// a quotient beyond Real's range
	checkOne(F(std::numeric_limits<Real>::max() / 4), F(std::ldexp(Real(1), std::numeric_limits<Real>::min_exponent + 20)));
	checkOne(F(std::ldexp(Real(1.75), std::numeric_limits<Real>::max_exponent - 4)), F(Real(3.1), Real(0)));
	std::mt19937_64 rng(19540);
	const int maxE = std::numeric_limits<Real>::max_exponent / 4;
	for (int i = 0; i < 20000; ++i) {
		const F x = ordinary<Real>(rng, -maxE, maxE);
		const F y = ordinary<Real>(rng, -maxE / 2, maxE / 2);
		checkOne(x, y);
	}

	auto check = [&](bool ok, const char* what) {
		if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL " << type_tag(F()) << ' ' << what << '\n'; }
	};
	const F pinf(std::numeric_limits<Real>::infinity()), nan(std::numeric_limits<Real>::quiet_NaN());
	errno = 0;
	check(isnan(fmod(F(Real(1)), F(Real(0)))) && errno == EDOM, "fmod(x, 0) = NaN with EDOM");
	errno = 0;
	check(isnan(fmod(pinf, F(Real(2)))) && errno == EDOM, "fmod(inf, y) = NaN with EDOM");
	check(fmod(F(Real(2.5)), pinf) == F(Real(2.5)), "fmod(x, inf) = x");
	check(isnan(fmod(nan, F(Real(1)))) && isnan(fmod(F(Real(1)), nan)), "a NaN operand gives NaN");
	check(signbit(fmod(F(Real(-6)), F(Real(3)))) && iszero(fmod(F(Real(-6)), F(Real(3)))), "fmod(-6, 3) = -0");
	check(signbit(fmod(F(Real(-0.0)), F(Real(3)))), "fmod(-0, y) = -0");
	return fails;
}

}  // namespace

// The exact expansion behind fmod, exp, exp10 and pow keeps its components in an inline buffer of
// 32, and moves to the heap past that (#1969). The library's functions stay below it (12 components
// for float, 11 for double), so this drives one past it directly: 2^(-1074 + 60k) for k = 0..34
// are 35 double components that no twoSum can merge, 60 bits apart. It checks the size, the sign,
// that a copy of a spilled expansion is the same sum, and that adding the negations in turn
// cancels it to zero exactly, through and back below the inline capacity.
int VerifyExpansionSpill(bool reportTestCases) {
	using sw::universal::faithful_detail::exact_expansion;
	int fails = 0;
	auto check = [&](bool ok, const char* what) { if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL expansion " << what << '\n'; } };
	constexpr int count = 35;
	exact_expansion<double> e;
	for (int k = 0; k < count; ++k) e.add(std::ldexp(1.0, -1074 + 60 * k));
	check(e.size() == count, "holds 35 separate components past the inline 32");
	check(e.sign() == 1, "is positive");
	check(e.approx() == std::ldexp(1.0, -1074 + 60 * (count - 1)), "rounds to its leading component");
	exact_expansion<double> copy(e);
	check(copy.size() == count && copy.approx() == e.approx(), "copies when spilled");
	exact_expansion<double> assigned;
	assigned.add(1.0);
	assigned = e;
	check(assigned.size() == count, "assigns when spilled");
	for (int k = count - 1; k >= 0; --k) {
		copy.add(-std::ldexp(1.0, -1074 + 60 * k));
		if (copy.size() != static_cast<std::size_t>(k)) { check(false, "cancels one component at a time"); break; }
	}
	check(copy.size() == 0 && copy.sign() == 0 && copy.approx() == 0.0, "cancels to zero");
	check(e.size() == count, "is unchanged by changes to its copy");
	// a spilled expansion keeps working: adding a value that overlaps its top merges into it
	e.add(std::ldexp(1.0, -1074 + 60 * (count - 1)));
	check(e.size() == count && e.approx() == std::ldexp(1.0, -1074 + 60 * (count - 1) + 1), "merges an overlapping add");
	return fails;
}

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

	std::string test_suite  = "faithful floor, ceil, trunc, round and fmod";
	std::string test_tag    = "faithful";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	faithful<float> x(std::ldexp(1.0f, 30), 0.75f);
	std::cout << floor(x) << ' ' << round(x) << ' ' << fmod(x, faithful<float>(3.0f)) << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerValued<FloatRef>(reportTestCases), "faithful<float>", "floor ceil trunc round");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerValued<DoubleRef>(reportTestCases), "faithful<double>", "floor ceil trunc round");
	// the remainder is exact, then rounded once to the pair: a relative error below epsilon()
	nrOfFailedTestCases += ReportTestResult(VerifyFmod<FloatRef>(reportTestCases, std::ldexp(1.0, -46)), "faithful<float>", "fmod");
	nrOfFailedTestCases += ReportTestResult(VerifyFmod<DoubleRef>(reportTestCases, std::ldexp(1.0, -104)), "faithful<double>", "fmod");
	nrOfFailedTestCases += ReportTestResult(VerifyExpansionSpill(reportTestCases), "exact_expansion<double>", "past the inline capacity");
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
