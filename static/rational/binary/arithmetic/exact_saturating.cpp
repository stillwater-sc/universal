// exact_saturating.cpp: rational arithmetic exact and saturating, comparisons exact, specific values (#2132)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <vector>
#include <universal/number/rational/rational.hpp>
#include <universal/verification/test_suite.hpp>

/*
   Each rational operation computes its exact result in a wide type and fits it back: exact where
   it fits, saturating at maxpos/maxneg past the range (USER decision on #2132), the closest
   representable rational in between, NaN for a zero divisor in every base. Comparisons are
   exact. This test holds the four bases to that against a brute-force reference: every pair of
   values of rational<5,base2> and random pairs of base10, base8 and base16 rationals, each result compared with
   the set of all representable values.
*/

namespace sw { namespace universal { namespace rational_test {

// an exact fraction in long long, the reference arithmetic
struct Frac {
	long long n{ 0 }, d{ 1 };
};
Frac reduce(long long n, long long d) {
	if (d < 0) { n = -n; d = -d; }
	const long long g = std::gcd(n < 0 ? -n : n, d);
	return Frac{ n / g, d / g };
}
// sign of x - y
int cmp(const Frac& x, const Frac& y) {
	const long long l = x.n * y.d, r = y.n * x.d;
	return (l < r) ? -1 : ((l > r) ? 1 : 0);
}
// |x - y| compared with |x - z|: -1, 0, 1
int closer(const Frac& x, const Frac& y, const Frac& z) {
	// |x - y| = |x.n y.d - y.n x.d| / (x.d y.d)
	const long long a = std::llabs(x.n * y.d - y.n * x.d) * z.d;
	const long long b = std::llabs(x.n * z.d - z.n * x.d) * y.d;
	return (a < b) ? -1 : ((a > b) ? 1 : 0);
}

template<typename R>
Frac value(const R& r) {
	return reduce(static_cast<long long>(r.numerator()), static_cast<long long>(r.denominator()));
}

// every value R can hold, reduced, from its field pairs
template<typename R>
std::vector<Frac> representable(long long nmin, long long nmax, long long dmax) {
	std::vector<Frac> v;
	for (long long n = nmin; n <= nmax; ++n) {
		for (long long d = -dmax; d <= dmax; ++d) {
			if (d == 0) continue;
			const Frac f = reduce(n, d);
			bool seen = false;
			for (const Frac& g : v) if (g.n == f.n && g.d == f.d) { seen = true; break; }
			if (!seen) v.push_back(f);
		}
	}
	return v;
}

// the reference: x past [lo, hi] saturates, otherwise the closest representable value; true when r is that
bool expected(const Frac& x, const Frac& lo, const Frac& hi, const std::vector<Frac>& values, const Frac& r) {
	if (cmp(x, hi) > 0) return cmp(r, hi) == 0;
	if (cmp(x, lo) < 0) return cmp(r, lo) == 0;
	// r must be at the minimal distance from x
	for (const Frac& v : values) if (closer(x, v, r) < 0) return false;
	return true;
}

template<typename R>
int VerifyAgainstReference(const std::vector<std::pair<R, R>>& pairs, const std::vector<Frac>& values,
                           const Frac& lo, const Frac& hi, bool reportTestCases) {
	int fails = 0;
	for (const auto& [a, b] : pairs) {
		const Frac x = value(a), y = value(b);
		for (char op : { '+', '-', '*', '/' }) {
			R r;
			Frac exact;
			switch (op) {
			case '+': r = a + b; exact = reduce(x.n * y.d + y.n * x.d, x.d * y.d); break;
			case '-': r = a - b; exact = reduce(x.n * y.d - y.n * x.d, x.d * y.d); break;
			case '*': r = a * b; exact = reduce(x.n * y.n, x.d * y.d); break;
			default:
				if (y.n == 0) {
					r = a / b;
					if (!r.isnan()) { ++fails; if (reportTestCases) std::cerr << "FAIL " << x.n << '/' << x.d << " / 0 is not NaN\n"; }
					continue;
				}
				r = a / b;
				exact = reduce(x.n * y.d, x.d * y.n);
				break;
			}
			if (r.isnan() || !expected(exact, lo, hi, values, value(r))) {
				++fails;
				if (reportTestCases && fails < 8) {
					const Frac got = r.isnan() ? Frac{ 0, 0 } : value(r);
					std::cerr << "FAIL " << x.n << '/' << x.d << ' ' << op << ' ' << y.n << '/' << y.d << " = " << got.n << '/' << got.d
					          << ", exact " << exact.n << '/' << exact.d << '\n';
				}
			}
		}
	}
	return fails;
}

// every pair of values of rational<5,base2>: fields in [-16, 15], so values n/d with |d| <= 16
int VerifyBinaryExhaustive(bool reportTestCases) {
	using R = rational<5, base2, std::uint8_t>;
	const std::vector<Frac> values = representable<R>(-16, 15, 16);
	std::vector<R> all;
	for (long long n = -16; n <= 15; ++n)
		for (long long d = 1; d <= 15; ++d) { R r; r.set(n, d); all.push_back(r); }
	std::vector<std::pair<R, R>> pairs;
	for (const R& a : all) for (const R& b : all) pairs.emplace_back(a, b);
	return VerifyAgainstReference<R>(pairs, values, Frac{ -16, 1 }, Frac{ 15, 1 }, reportTestCases);
}

// random pairs of a digit-base rational whose fields reach +-bound
template<typename R>
int VerifyDigitRandom(long long bound, int trials, bool reportTestCases) {
	std::vector<Frac> values;
	for (long long d = 1; d <= bound; ++d)
		for (long long n = -bound; n <= bound; ++n)
			if (std::gcd(n < 0 ? -n : n, d) == 1) values.push_back(Frac{ n, d });
	std::mt19937_64 eng(0x2132u + static_cast<unsigned>(bound));
	std::vector<std::pair<R, R>> pairs;
	const unsigned long long span = static_cast<unsigned long long>(2 * bound + 1);
	for (int t = 0; t < trials; ++t) {
		R a, b;
		a.set(static_cast<long long>(eng() % span) - bound, static_cast<long long>(eng() % static_cast<unsigned long long>(bound)) + 1);
		b.set(static_cast<long long>(eng() % span) - bound, static_cast<long long>(eng() % static_cast<unsigned long long>(bound)) + 1);
		pairs.emplace_back(a, b);
	}
	return VerifyAgainstReference<R>(pairs, values, Frac{ -bound, 1 }, Frac{ bound, 1 }, reportTestCases);
}

// the specific value constructors, in every base
template<typename R>
int VerifySpecificValues(bool reportTestCases) {
	int fails = 0;
	auto expect = [&](const char* what, bool ok) { if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL " << what << '\n'; } };
	expect("zero", R(SpecificValue::zero).iszero());
	expect("maxpos", R(SpecificValue::maxpos) == R().maxpos());
	expect("minpos", R(SpecificValue::minpos) == R().minpos());
	expect("maxneg", R(SpecificValue::maxneg) == R().maxneg());
	expect("minneg", R(SpecificValue::minneg) == R().minneg());
	expect("infpos saturates at maxpos", R(SpecificValue::infpos) == R().maxpos());
	expect("infneg saturates at maxneg", R(SpecificValue::infneg) == R().maxneg());
	expect("qnan", R(SpecificValue::qnan).isnan());
	expect("no specific value but NaN is NaN", !R(SpecificValue::maxpos).isnan() && !R(SpecificValue::zero).isnan());
	const R one(1), zero(0);
	expect("1 / 0 is NaN", (one / zero).isnan());
	expect("0 / 0 is NaN", (zero / zero).isnan());
	const R nan(SpecificValue::qnan);
	expect("NaN == NaN is false", !(nan == nan));
	expect("NaN != NaN is true", nan != nan);
	expect("NaN < 1 is false", !(nan < one) && !(one < nan));
	expect("NaN + 1 is NaN", (nan + one).isnan());
	expect("maxpos + maxpos saturates", (R().maxpos() + R().maxpos()) == R().maxpos());
	expect("maxneg + maxneg saturates", (R().maxneg() + R().maxneg()) == R().maxneg());
	expect("maxpos * maxneg saturates", (R().maxpos() * R().maxneg()) == R().maxneg());
	return fails;
}

// comparisons exact past double's 53 bits
int VerifyExactComparison(bool reportTestCases) {
	using R = rational<64, base2, std::uint64_t>;
	const R a(9007199254740993ll), b(9007199254740992ll);
	int fails = 0;
	if (a == b) { ++fails; if (reportTestCases) std::cerr << "FAIL 2^53 + 1 == 2^53\n"; }
	if (!(b < a) || a < b || !(a > b) || !(b <= a) || !(a >= b)) { ++fails; if (reportTestCases) std::cerr << "FAIL ordering of 2^53 + 1 and 2^53\n"; }
	using D = rational<20, base10, std::uint8_t>;
	const D c(9007199254740993ll), d(9007199254740992ll);
	if (c == d || !(d < c)) { ++fails; if (reportTestCases) std::cerr << "FAIL base10 2^53 + 1 against 2^53\n"; }
	// a negative denominator, which the binary pair keeps when moving the sign would overflow (#1525)
	using S = rational<4, base2, std::uint8_t>;
	S e;
	e.set(-8, 7);
	e = -e;        // 8/7, stored -8/-7
	S f;
	f.set(1, 1);
	if (!(f < e) || e < f || e == f) { ++fails; if (reportTestCases) std::cerr << "FAIL 8/7 (stored -8/-7) against 1\n"; }
	return fails;
}

// the saturation test may not overflow the wide type: 1/16384 squared in a rational<16> reduces to
// 1/2^28, and its closest representable value is zero, not maxpos
int VerifyWideQuotientNotSaturated(bool reportTestCases) {
	using R = rational<16, base2, std::uint8_t>;
	R x;
	x.set(1, 16384);
	const R sq = x * x;
	int fails = 0;
	if (!sq.iszero()) { ++fails; if (reportTestCases) std::cerr << "FAIL (1/16384)^2 is " << double(sq) << ", not 0\n"; }
	R y;
	y.set(-1, 16384);
	if (!(y * x).iszero()) { ++fails; if (reportTestCases) std::cerr << "FAIL -(1/16384)^2 is not 0\n"; }
	// and real overflow still saturates
	R big(200);
	if (!((big * big) == R().maxpos())) { ++fails; if (reportTestCases) std::cerr << "FAIL 200 * 200 does not saturate\n"; }
	return fails;
}

template<typename R>
int VerifyNumericLimits(bool reportTestCases) {
	using L = std::numeric_limits<R>;
	int fails = 0;
	// by value: at two bits 1/2 exists only as -1/-2
	if (double(L::round_error()) != 0.5) { ++fails; if (reportTestCases) std::cerr << "FAIL round_error is " << double(L::round_error()) << ", not 1/2\n"; }
	if (!L::is_bounded) { ++fails; if (reportTestCases) std::cerr << "FAIL is_bounded\n"; }
	if (!(L::infinity() == L::max())) { ++fails; if (reportTestCases) std::cerr << "FAIL infinity is not max\n"; }
	if (!L::has_quiet_NaN || !L::quiet_NaN().isnan()) { ++fails; if (reportTestCases) std::cerr << "FAIL quiet_NaN\n"; }
	return fails;
}

}}}  // namespace sw::universal::rational_test

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
	using namespace sw::universal::rational_test;

	std::string test_suite  = "rational exact, saturating arithmetic";
	std::string test_tag    = "exact saturating";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryExhaustive(reportTestCases), "rational<5,base2>", test_tag);
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryExhaustive(reportTestCases), "rational<5,base2>", "+ - * / exhaustive");
	nrOfFailedTestCases += ReportTestResult(VerifyDigitRandom<rational<2, base10, std::uint8_t>>(99, 3000, reportTestCases), "rational<2,base10>", "+ - * / random");
	nrOfFailedTestCases += ReportTestResult(VerifyDigitRandom<rational<2, base8, std::uint8_t>>(63, 3000, reportTestCases), "rational<2,base8>", "+ - * / random");
	nrOfFailedTestCases += ReportTestResult(VerifyDigitRandom<rational<1, base16, std::uint8_t>>(15, 3000, reportTestCases), "rational<1,base16>", "+ - * / random");

	nrOfFailedTestCases += ReportTestResult(VerifySpecificValues<rational<8, base2, std::uint8_t>>(reportTestCases), "rational<8,base2>", "specific values");
	nrOfFailedTestCases += ReportTestResult(VerifySpecificValues<rational<3, base8, std::uint8_t>>(reportTestCases), "rational<3,base8>", "specific values");
	nrOfFailedTestCases += ReportTestResult(VerifySpecificValues<rational<3, base10, std::uint8_t>>(reportTestCases), "rational<3,base10>", "specific values");
	nrOfFailedTestCases += ReportTestResult(VerifySpecificValues<rational<3, base16, std::uint8_t>>(reportTestCases), "rational<3,base16>", "specific values");

	nrOfFailedTestCases += ReportTestResult(VerifyExactComparison(reportTestCases), "rational", "exact comparison");

	nrOfFailedTestCases += ReportTestResult(VerifyWideQuotientNotSaturated(reportTestCases), "rational<16,base2>", "no false saturation");
	nrOfFailedTestCases += ReportTestResult(VerifyNumericLimits<rational<8, base2, std::uint8_t>>(reportTestCases), "rational<8,base2>", "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyNumericLimits<rational<2, base2, std::uint8_t>>(reportTestCases), "rational<2,base2>", "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyNumericLimits<rational<3, base10, std::uint8_t>>(reportTestCases), "rational<3,base10>", "numeric_limits");
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
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
