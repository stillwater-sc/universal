// reference.cpp: test suite runner checking erational arithmetic against exact int64 fractions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <numeric>   // std::gcd
#include <cstdint>
#include <limits>
#include <universal/number/erational/erational.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	// an exact fraction in lowest terms, the sign on the numerator, the denominator positive
	struct Fraction {
		long long n, d;
		Fraction(long long num, long long den) {
			if (den < 0) { num = -num; den = -den; }
			const long long g = std::gcd(num, den);
			n = num / g;
			d = den / g;
		}
	};
	inline Fraction add(Fraction a, Fraction b) { return Fraction(a.n * b.d + b.n * a.d, a.d * b.d); }
	inline Fraction sub(Fraction a, Fraction b) { return Fraction(a.n * b.d - b.n * a.d, a.d * b.d); }
	inline Fraction mul(Fraction a, Fraction b) { return Fraction(a.n * b.n, a.d * b.d); }
	inline Fraction div(Fraction a, Fraction b) { return Fraction(a.n * b.d, a.d * b.n); }

	// r must be ref in lowest terms, with no negative zero: toPair() carries the sign on the numerator
	inline bool matches(const erational& r, Fraction ref) {
		if (r.iszero() && r.isneg()) return false;
		const auto p = r.toPair();
		return p.first == ref.n && p.second == ref.d;
	}

	// + - * / on every pair of operands n/d with n in [-range, range] and d in [1, range]
	inline int VerifyBinaryOperators(long long range, bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		auto check = [&](const erational& r, Fraction ref, const erational& a, const char* op, const erational& b) {
			if (!matches(r, ref)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: " << a << ' ' << op << ' ' << b << " = " << r << (r.isneg() ? " (negative)" : "") << ", expected " << ref.n << '/' << ref.d << '\n';
			}
		};
		for (long long an = -range; an <= range; ++an) {
			for (long long ad = 1; ad <= range; ++ad) {
				const erational a(an, ad);
				const Fraction fa(an, ad);
				for (long long bn = -range; bn <= range; ++bn) {
					for (long long bd = 1; bd <= range; ++bd) {
						const erational b(bn, bd);
						const Fraction fb(bn, bd);
						check(a + b, add(fa, fb), a, "+", b);
						check(a - b, sub(fa, fb), a, "-", b);
						check(a * b, mul(fa, fb), a, "*", b);
						if (bn != 0) check(a / b, div(fa, fb), a, "/", b);
					}
				}
			}
		}
		return nrOfFailedTestCases;
	}

	// ++, --, negation, and each compound operator with itself as the operand: 1/2 ++ gave 2/2, and
	// x /= x read the numerator it had just changed (#1990)
	inline int VerifyUnaryAndAliasing(long long range, bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		auto check = [&](const erational& r, Fraction ref, long long n, long long d, const char* op) {
			if (!matches(r, ref)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: " << n << '/' << d << ' ' << op << " = " << r << (r.isneg() ? " (negative)" : "") << ", expected " << ref.n << '/' << ref.d << '\n';
			}
		};
		const Fraction one(1, 1);
		for (long long n = -range; n <= range; ++n) {
			for (long long d = 1; d <= range; ++d) {
				const Fraction f(n, d);
				erational x(n, d), y(n, d);
				const erational post = x++;
				check(x, add(f, one), n, d, "++ (postfix, new value)");
				if (post != erational(n, d)) {
					++nrOfFailedTestCases;
					if (reportTestCases) std::cerr << "FAIL: " << n << '/' << d << " ++ returned " << post << '\n';
				}
				check(++y, add(f, one), n, d, "++ (prefix)");
				x = erational(n, d); y = erational(n, d);
				x--;
				check(x, sub(f, one), n, d, "-- (postfix)");
				check(--y, sub(f, one), n, d, "-- (prefix)");
				// negation: the same value with the other sign, and never -0
				const erational neg = -erational(n, d);
				if (neg != erational(-n, d) || (neg.iszero() && neg.isneg())) {
					++nrOfFailedTestCases;
					if (reportTestCases) std::cerr << "FAIL: -(" << n << '/' << d << ") = " << neg << '\n';
				}
				x = erational(n, d); x += x; check(x, add(f, f), n, d, "+= itself");
				x = erational(n, d); x -= x; check(x, sub(f, f), n, d, "-= itself");
				x = erational(n, d); x *= x; check(x, mul(f, f), n, d, "*= itself");
				if (n != 0) { x = erational(n, d); x /= x; check(x, div(f, f), n, d, "/= itself"); }
			}
		}
		return nrOfFailedTestCases;
	}

	// zero results of every operator are +0: 0 / -3 was -0/1 (#1990)
	inline int VerifySignedZero(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		const erational zero(0), m3(-3), half(1, 2);
		const erational results[] = { zero / m3, zero * m3, m3 * zero, -zero, half - half, (-half) + half, m3 - m3 };
		for (const erational& r : results) {
			if (!r.iszero() || r.isneg()) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: zero result " << r << (r.isneg() ? " is negative" : " is not zero") << '\n';
			}
		}
		erational x(0); x /= erational(-3);
		erational y(0); y *= erational(-7, 2);
		if (x.isneg() || y.isneg()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: compound zero result is negative\n";
		}
		return nrOfFailedTestCases;
	}

	// toPair() keeps the sign and saturates a part beyond int64 (#1990)
	inline int VerifyToPair(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		constexpr int64_t maxll = std::numeric_limits<int64_t>::max(), minll = std::numeric_limits<int64_t>::min();
		auto check = [&](const erational& r, int64_t n, int64_t d, const char* label) {
			const auto p = r.toPair();
			if (p.first != n || p.second != d) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: toPair(" << label << ") = {" << p.first << ", " << p.second << "}, expected {" << n << ", " << d << "}\n";
			}
		};
		auto parsed = [](const char* s) { erational r; r.parse(s); return r; };
		check(erational(-3, 2), -3, 2, "-3/2");
		check(erational(3, -2), -3, 2, "3/-2");
		check(erational(0), 0, 1, "0");
		check(erational(maxll, int64_t(1)), maxll, 1, "INT64_MAX");
		check(erational(minll, int64_t(1)), minll, 1, "INT64_MIN");                  // magnitude 2^63, exact
		check(parsed("100000000000000000001/3"), maxll, 3, "1e20+1 over 3");
		check(parsed("-100000000000000000001/3"), minll, 3, "-(1e20+1) over 3");
		check(parsed("1/100000000000000000001"), 1, maxll, "1 over 1e20+1");
		check(parsed("9223372036854775808"), maxll, 1, "2^63");                    // one past INT64_MAX
		erational nan; nan.setnan();
		check(nan, 0, 0, "nan");
		return nrOfFailedTestCases;
	}

}}  // namespace sw::universal

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

	std::string test_suite  = "erational arithmetic against exact fractions";
	std::string test_tag    = "reference";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	erational a(1, 2);
	a++;
	std::cout << "1/2 ++ = " << a << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperators(4, reportTestCases), "erational", "+ - * /");
	nrOfFailedTestCases += ReportTestResult(VerifyUnaryAndAliasing(6, reportTestCases), "erational", "++ -- negate and x op= x");
	nrOfFailedTestCases += ReportTestResult(VerifySignedZero(reportTestCases), "erational", "signed zero");
	nrOfFailedTestCases += ReportTestResult(VerifyToPair(reportTestCases), "erational", "toPair");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyBinaryOperators(8, reportTestCases), "erational", "+ - * /");
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
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
