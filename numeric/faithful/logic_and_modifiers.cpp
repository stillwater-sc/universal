// logic_and_modifiers.cpp: test suite runner for faithful's comparisons, abs, ++/--, modifiers and classification
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Until #1936 these functions of faithful<Real> were stubs: == and < were always false, so a == a
// was false and <= and >= always true; abs() returned zero; ++ and -- printed "TBD" and left the
// value alone; reset() did nothing; isinf() and isnan() were always false; and components()
// called a fraction() that faithful does not have. An infinity also did not survive arithmetic:
// renormalizing it left a NaN error, so faithful(inf) + 1 converted to NaN.
//
// The reference is exact: a faithful<float> is a float value plus a float error, and their sum
// fits a double exactly, so the double sum orders and measures every pair without rounding.
#include <universal/utility/directives.hpp>
#include <universal/number/faithful/faithful.hpp>
#include <universal/verification/test_suite.hpp>
#include <cmath>
#include <limits>
#include <random>
#include <string>
#include <vector>

namespace {

using Faithful = sw::universal::faithful<float>;

// the exact value a faithful<float> represents
double exact(const Faithful& v) { return double(v.high()) + double(v.low()); }

float ulpOf(float v) {
	float a = std::fabs(v);
	return std::nextafter(a, INFINITY) - a;
}

// a random faithful<float> with a head in [1, 2) * 2^k and a random sign. canonical: the error is
// at most half an ulp of the head. Otherwise the error may be several ulps, which a pair built
// with faithful(v, e) can be, and the comparisons have to handle it.
Faithful randomFaithful(std::mt19937_64& rng, bool canonical) {
	std::uniform_real_distribution<float> mant(1.0f, 2.0f);
	std::uniform_int_distribution<int>    expo(-4, 4);
	std::uniform_real_distribution<float> frac(canonical ? -0.5f : -8.0f, canonical ? 0.5f : 8.0f);
	std::bernoulli_distribution           neg(0.5);
	float head = std::ldexp(mant(rng), expo(rng));
	if (neg(rng)) head = -head;
	return Faithful(head, frac(rng) * ulpOf(head));
}

// each comparison of x and y against the order of their exact sums
int compareAll(const Faithful& x, const Faithful& y, bool reportTestCases, int& reported) {
	const double a = exact(x), b = exact(y);
	const bool got[6]  = { x == y, x != y, x < y, x > y, x <= y, x >= y };
	const bool want[6] = { a == b, a != b, a < b, a > b, a <= b, a >= b };
	static const char* names[6] = { "==", "!=", "<", ">", "<=", ">=" };
	int fails = 0;
	for (int i = 0; i < 6; ++i) {
		if (got[i] == want[i]) continue;
		++fails;
		if (reportTestCases && reported++ < 8) {
			std::cerr << "FAIL " << x << ' ' << names[i] << ' ' << y << " = " << got[i]
			          << ", exact " << a << " vs " << b << '\n';
		}
	}
	return fails;
}

// == and < and everything derived from them order by value + error
int VerifyComparisons(bool reportTestCases) {
	int fails = 0, reported = 0;
	const float u = std::ldexp(1.0f, -24);   // half an ulp of 1
	// the reproduction in #1936
	{
		const Faithful a(1.5f), b(2.5f);
		fails += compareAll(a, a, reportTestCases, reported);
		fails += compareAll(a, b, reportTestCases, reported);
		fails += compareAll(b, a, reportTestCases, reported);
	}
	// a tie of the values: the errors decide
	fails += compareAll(Faithful(1.0f, u / 2), Faithful(1.0f, -u / 2), reportTestCases, reported);
	fails += compareAll(Faithful(1.0f, u / 2), Faithful(1.0f, u / 2), reportTestCases, reported);
	fails += compareAll(Faithful(-1.0f, u / 4), Faithful(-1.0f, u / 2), reportTestCases, reported);
	// the same sum from different pairs: one is not canonical, and comparing the values first
	// without renormalizing would order them
	{
		const Faithful p(1.0f, 4 * u), q(1.0f + 4 * u, 0.0f);
		fails += compareAll(p, q, reportTestCases, reported);
		fails += compareAll(Faithful(1.0f, 6 * u), Faithful(1.0f + 2 * u, 0.0f), reportTestCases, reported);
		fails += compareAll(Faithful(1.0f + 2 * u, -6 * u), Faithful(1.0f, 0.0f), reportTestCases, reported);
	}
	// signed zeros are equal
	fails += compareAll(Faithful(0.0f), Faithful(-0.0f), reportTestCases, reported);
	// random pairs, canonical and not, with pairs close enough to share a value
	std::mt19937_64 rng(1936);
	for (int i = 0; i < 20000; ++i) {
		const bool canonical = (i % 2) == 0;
		const Faithful x = randomFaithful(rng, canonical);
		fails += compareAll(x, randomFaithful(rng, canonical), reportTestCases, reported);
		// a neighbour of x: the same value, the error moved by a few ulps of the error's binade
		const Faithful y(x.high(), x.low() + static_cast<float>(i % 5 - 2) * ulpOf(x.high()) / 8);
		fails += compareAll(x, y, reportTestCases, reported);
	}
	return fails;
}

// a NaN is unordered: every comparison is false except !=. Infinities order at the ends.
int VerifyNonFiniteComparisons(bool reportTestCases) {
	int fails = 0;
	auto check = [&](bool ok, const char* what) {
		if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL " << what << '\n'; }
	};
	Faithful nan, pinf, ninf;
	nan.setnan();
	pinf.setinf();
	ninf.setinf(true);
	const Faithful one(1.0f), big(std::numeric_limits<float>::max(), 0.0f);
	for (const Faithful& x : { nan, one, pinf }) {
		check(!(nan == x) && (nan != x) && !(nan < x) && !(nan > x) && !(nan <= x) && !(nan >= x), "NaN op x");
		check(!(x == nan) && (x != nan) && !(x < nan) && !(x > nan) && !(x <= nan) && !(x >= nan), "x op NaN");
	}
	check(pinf == pinf && pinf <= pinf && !(pinf < pinf), "+inf == +inf");
	check(big < pinf && pinf > big && ninf < -big && ninf < pinf, "infinities at the ends");
	check(ninf != pinf && ninf == -pinf, "-inf");
	return fails;
}

// abs() negates both parts when the represented value is negative
int VerifyAbs(bool reportTestCases) {
	int fails = 0, reported = 0;
	auto checkAbs = [&](const Faithful& x) {
		const Faithful a = abs(x);
		if (exact(a) == std::fabs(exact(x)) && !std::signbit(exact(a))) return;
		++fails;
		if (reportTestCases && reported++ < 8) std::cerr << "FAIL abs(" << x << ") = " << a << '\n';
	};
	checkAbs(Faithful(-2.5f));          // the reproduction in #1936
	checkAbs(Faithful(2.5f));
	checkAbs(Faithful(-0.0f));
	checkAbs(Faithful(0.0f));
	// the value is positive but the sum negative, and the other way around
	checkAbs(Faithful(std::ldexp(1.0f, -30), -std::ldexp(1.0f, -29)));
	checkAbs(Faithful(-std::ldexp(1.0f, -30), std::ldexp(1.0f, -29)));
	checkAbs(Faithful(1.0f, -2.0f));
	checkAbs(Faithful(1.0f, -1.0f));
	std::mt19937_64 rng(36);
	for (int i = 0; i < 20000; ++i) checkAbs(randomFaithful(rng, (i % 2) == 0));
	Faithful ninf;
	ninf.setinf(true);
	if (!abs(ninf).isinf() || abs(ninf).sign()) {
		++fails;
		if (reportTestCases) std::cerr << "FAIL abs(-inf) = " << abs(ninf) << '\n';
	}
	return fails;
}

// ++ and -- add and subtract one as compensated additions; the postfix forms return the old value
int VerifyIncrementDecrement(bool reportTestCases) {
	int fails = 0, reported = 0;
	auto check = [&](bool ok, const Faithful& x, const char* what) {
		if (ok) return;
		++fails;
		if (reportTestCases && reported++ < 8) std::cerr << "FAIL " << what << " on " << x << '\n';
	};
	// the reproduction in #1936
	{
		Faithful a(1.5f);
		++a;
		check(exact(a) == 2.5, a, "++1.5");
	}
	// 2^24 + 1 is not a float: the head rounds and the error keeps the one
	{
		const Faithful start(std::ldexp(1.0f, 24));
		Faithful a(start);
		++a;
		check(exact(a) == std::ldexp(1.0, 24) + 1.0, a, "++2^24");
		--a;
		check(exact(a) == std::ldexp(1.0, 24), a, "--(2^24 + 1)");
		Faithful b(start);
		const Faithful old = b++;
		check(exact(old) == exact(start) && exact(b) == exact(start) + 1.0, b, "2^24++");
		const Faithful old2 = b--;
		check(exact(old2) == exact(start) + 1.0 && exact(b) == exact(start), b, "(2^24 + 1)--");
	}
	// an error below the head's ulp survives the increment
	{
		Faithful a(1.0f, std::ldexp(1.0f, -30));
		++a;
		check(exact(a) == 2.0 + std::ldexp(1.0, -30), a, "++(1 + 2^-30)");
		--a;
		--a;
		check(exact(a) == std::ldexp(1.0, -30), a, "--(2 + 2^-30) twice");
	}
	// random canonical values: x + 1 and then - 1 to within the pair's precision, relative to the
	// larger operand, max(|x|, 1), since x near -1 cancels
	std::mt19937_64 rng(1);
	for (int i = 0; i < 20000; ++i) {
		const Faithful x = randomFaithful(rng, true);
		const double tol = std::ldexp(std::fmax(std::fabs(exact(x)), 1.0), -44);
		Faithful y(x);
		++y;
		const bool okUp = std::fabs(exact(y) - (exact(x) + 1.0)) <= tol;
		--y;
		const bool okDown = std::fabs(exact(y) - exact(x)) <= 2 * tol;
		check(okUp && okDown, x, "x + 1 - 1");
	}
	return fails;
}

// reset, setzero, setnan, setinf, and the classification of the results
int VerifyModifiersAndClassification(bool reportTestCases) {
	int fails = 0;
	auto check = [&](bool ok, const char* what) {
		if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL " << what << '\n'; }
	};
	Faithful a(1.5f, 1.0e-8f);
	a.reset();
	check(a.iszero() && a.high() == 0.0f && a.low() == 0.0f, "reset() is zero");
	a = Faithful(1.5f, 1.0e-8f);
	a.setzero();
	check(a.iszero(), "setzero() is zero");
	a.setnan();
	check(a.isnan() && !a.isinf() && !a.iszero(), "setnan() is NaN");
	a.setinf();
	check(a.isinf() && !a.isnan() && !a.sign(), "setinf() is +inf");
	a.setinf(true);
	check(a.isinf() && a.sign(), "setinf(true) is -inf");
	check(Faithful(1.5f).isinf() == false && Faithful(1.5f).isnan() == false, "1.5 is finite");
	// the sign and zero of the represented value, for pairs that are not canonical
	check(Faithful(1.0f, -1.0f).iszero(), "(1, -1) is zero");
	check(Faithful(1.0f, -2.0f).sign() && Faithful(1.0f, -2.0f).isneg(), "(1, -2) is negative");
	check(!Faithful(-1.0f, 2.0f).sign() && !Faithful(-1.0f, 2.0f).isneg(), "(-1, 2) is positive");
	check(Faithful(std::numeric_limits<float>::infinity()).isinf(), "faithful(inf) is inf");
	check(Faithful(std::numeric_limits<float>::quiet_NaN()).isnan(), "faithful(NaN) is NaN");
	// an infinity survives arithmetic: renormalizing it used to leave a NaN error, and the
	// conversion value + error then gave NaN
	{
		const Faithful pinf(std::numeric_limits<float>::infinity());
		const Faithful r = pinf + Faithful(1.0f);
		check(r.isinf() && !r.isnan() && r.low() == 0.0f, "inf + 1 is inf");
		check(std::isinf(float(r)) && float(r) > 0, "float(inf + 1) is +inf");
		const Faithful m = pinf * Faithful(-2.0f);
		check(m.isinf() && m.sign() && std::isinf(float(m)), "inf * -2 is -inf");
		const Faithful o = Faithful(std::numeric_limits<float>::max()) * Faithful(2.0f);
		check(o.isinf() && std::isinf(float(o)), "an overflow is inf");
		check((pinf - pinf).isnan(), "inf - inf is NaN");
	}
	return fails;
}

// type_tag, components and to_binary print the type and both parts
int VerifyText(bool reportTestCases) {
	using namespace sw::universal;
	int fails = 0;
	auto check = [&](const std::string& got, const std::string& want) {
		if (got == want) return;
		++fails;
		if (reportTestCases) std::cerr << "FAIL got \"" << got << "\", expected \"" << want << "\"\n";
	};
	check(type_tag(Faithful()), "faithful<float>");
	check(type_tag(faithful<double>()), "faithful<double>");
	check(components(Faithful(-1.5f, std::ldexp(1.0f, -30))), "( value -1.5, error 9.31322575e-10)");
	check(components(Faithful(0.0f)), "zero");
	// a pair that is not canonical: the sign and scale are those of the represented -1, and the
	// parts print as given
	check(components(Faithful(1.0f, -2.0f)), "( value 1, error -2)");
	check(components(Faithful(-1.0f, 3.0f)), "( value -1, error 3)");
	Faithful x;
	x.setnan();
	check(components(x), "nan");
	x.setinf(true);
	check(components(x), "-inf");

	// to_binary prints both parts as IEEE-754 encodings, sign.exponent.fraction (4fa9814). It was
	// printed by arithmetic.cpp but never checked (#2010); the expected strings are the encodings
	// written out by hand: 1.5f is 0.01111111.1000..., 2^-30 has the biased exponent 97, 01100001
	check(to_binary(Faithful(1.5f, std::ldexp(1.0f, -30))),
	      "(0b0.01111111.10000000000000000000000, 0b0.01100001.00000000000000000000000)");
	check(to_binary(Faithful(-1.5f, -std::ldexp(1.0f, -30))),
	      "(0b1.01111111.10000000000000000000000, 0b1.01100001.00000000000000000000000)");
	// a tail from arithmetic: 1 - 2^-26 rounds to 1 and keeps -2^-26 (biased exponent 101) as its error
	check(to_binary(Faithful(1.0f) - Faithful(std::ldexp(1.0f, -26))),
	      "(0b0.01111111.00000000000000000000000, 0b1.01100101.00000000000000000000000)");
	check(to_binary(Faithful(1.5f, std::ldexp(1.0f, -30)), true),
	      "(0b0.0111'1111.100'0000'0000'0000'0000'0000, 0b0.0110'0001.000'0000'0000'0000'0000'0000)");
	// faithful<double>: 2^-60 has the biased exponent 963, 01111000011; -3 is 1.10000000000.1000...
	check(to_binary(faithful<double>(1.0, std::ldexp(1.0, -60))),
	      "(0b0.01111111111.0000000000000000000000000000000000000000000000000000, 0b0.01111000011.0000000000000000000000000000000000000000000000000000)");
	check(to_binary(faithful<double>(-3.0, std::ldexp(1.0, -54))),
	      "(0b1.10000000000.1000000000000000000000000000000000000000000000000000, 0b0.01111001001.0000000000000000000000000000000000000000000000000000)");
	check(to_binary(Faithful(0.0f)), "( zero, 0)");
	check(to_binary(Faithful(-0.0f)), "( zero, 0)");
	x.setinf(true);
	check(to_binary(x), "(-inf, 0)");
	x.setinf(false);
	check(to_binary(x), "(+inf, 0)");
	x.setnan();
	check(to_binary(x), "( nan, 0)");
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

	std::string test_suite  = "faithful comparisons, abs, ++/--, modifiers and classification";
	std::string test_tag    = "faithful<float>";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	Faithful a(1.5f), b(2.5f);
	std::cout << (a == a) << (a != a) << (a < b) << (b < a) << (a <= b) << (a >= b)
	          << " abs(-2.5)=" << float(abs(Faithful(-2.5f))) << '\n';
	++a;
	std::cout << float(a) << ' ' << components(a) << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyComparisons(reportTestCases), test_tag, "comparisons");
	nrOfFailedTestCases += ReportTestResult(VerifyNonFiniteComparisons(reportTestCases), test_tag, "NaN and inf comparisons");
	nrOfFailedTestCases += ReportTestResult(VerifyAbs(reportTestCases), test_tag, "abs");
	nrOfFailedTestCases += ReportTestResult(VerifyIncrementDecrement(reportTestCases), test_tag, "++ and --");
	nrOfFailedTestCases += ReportTestResult(VerifyModifiersAndClassification(reportTestCases), test_tag, "modifiers, isinf, isnan");
	nrOfFailedTestCases += ReportTestResult(VerifyText(reportTestCases), test_tag, "type_tag, components and to_binary");
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
