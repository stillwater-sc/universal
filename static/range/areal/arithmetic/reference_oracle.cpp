// reference_oracle.cpp: the areal arithmetic suites' reference helpers, against an exact oracle (#2060)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The level 2-4 areal addition, subtraction, multiplication and division suites compute their
// reference as ArealReference(s, err): the double result s of the op and the error err of s, from
// TwoSumError, fma or QuotientError (verification/areal_test_suite.hpp, #2022). The __int128
// oracle that justified those helpers was never committed, so this test states their contracts
// against an exact one, the einteger-backed dyadic of dyadic_exact.hpp:
//   - TwoSumError(a, b, s) is the exact error: s + err == a + b;
//   - QuotientError(a, b, q) has the sign of the exact a / b - q, and is zero exactly when q is;
//   - ArealReference(s, err) is the exact result s + err truncated toward zero to the areal, with
//     the ubit set exactly when bits were dropped. The truncation is found by search over every
//     encoding of the areal, not by the helper's own reasoning.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include <universal/number/areal/areal.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/areal_test_suite.hpp>
#include <universal/verification/dyadic_exact.hpp>

namespace {
	using namespace sw::universal;

	dyadic D(double v) { return dyadic::from_double(v); }
	int sign_of(const dyadic& v) { return v.iszero() ? 0 : (v.numerator.sign() ? -1 : 1); }

	// s + err == a + b, exactly
	int VerifyTwoSumError(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		std::mt19937_64 rng(2060);
		std::uniform_real_distribution<double> mant(-1.0, 1.0);
		std::uniform_int_distribution<int> expo(-80, 80);
		for (int i = 0; i < 20000; ++i) {
			const double a = std::ldexp(mant(rng), expo(rng)), b = std::ldexp(mant(rng), expo(rng));
			const double s = a + b;
			const double err = TwoSumError(a, b, s);
			if (D(s) + D(err) != D(a) + D(b)) {
				++nrOfFailedTestCases;
				if (reportTestCases && nrOfFailedTestCases < 5) std::cerr << "FAIL: TwoSumError(" << a << ", " << b << ") is not the exact error\n";
			}
		}
		return nrOfFailedTestCases;
	}

	// sign(err) == sign(a / b - q) == sign((a - q b) b), exactly
	int VerifyQuotientError(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		std::mt19937_64 rng(2061);
		std::uniform_real_distribution<double> mant(-1.0, 1.0);
		std::uniform_int_distribution<int> expo(-80, 80);
		auto one = [&](double a, double b) {
			const double q = a / b;
			const double err = QuotientError(a, b, q);
			const int want = sign_of(D(a) - D(q) * D(b)) * (b < 0 ? -1 : 1);
			const int got = (err > 0) - (err < 0);
			if (got != want) {
				++nrOfFailedTestCases;
				if (reportTestCases && nrOfFailedTestCases < 5) std::cerr << "FAIL: QuotientError(" << a << ", " << b << ") has sign " << got << ", the exact error " << want << '\n';
			}
		};
		for (int i = 0; i < 20000; ++i) one(std::ldexp(mant(rng), expo(rng)), std::ldexp(mant(rng), expo(rng)));
		// exact quotients have no error
		one(1.0, 4.0);
		one(-3.0, 0.75);
		one(6.0, -3.0);
		return nrOfFailedTestCases;
	}

	// ArealReference of every pair of an areal's exact values under + and /, against the truncation
	// found by search: the encoding of largest magnitude, of the exact result's sign, not above it
	template<typename Areal>
	int VerifyArealReference(bool reportTestCases) {
		constexpr unsigned nbits = Areal::nbits;
		// the finite exact values (ubit clear), as doubles: every value of a small areal is one
		std::vector<double> values;
		for (std::uint64_t b = 0; b < (1ull << nbits); b += 2) {
			Areal a;
			a.setbits(b);
			if (a.isinf() || a.isnan()) continue;
			values.push_back(double(a));
		}
		const double maxpos = double(Areal(SpecificValue::maxpos));
		int nrOfFailedTestCases = 0;
		// the expected truncation of the exact result, given a predicate that orders a candidate v
		// against it: cmp(v) = sign(v - exact)
		auto check = [&](const Areal& got, auto cmp, int exactSign, const char* what, double x, double y) {
			double best = 0.0;
			bool found = false;
			for (double v : values) {
				if (exactSign > 0 && (v < 0 || cmp(v) > 0)) continue;          // v <= exact, v >= 0
				if (exactSign < 0 && (v > 0 || cmp(v) < 0)) continue;          // v >= exact, v <= 0
				if (exactSign == 0 && v != 0) continue;
				if (!found || std::fabs(v) > std::fabs(best)) { best = v; found = true; }
			}
			const bool inexact = (cmp(best) != 0);
			Areal t(got);
			const bool ubit = t.at(0);
			t.reset(0);
			if (double(t) != best || ubit != inexact) {
				++nrOfFailedTestCases;
				if (reportTestCases && nrOfFailedTestCases < 8) {
					std::cerr << "FAIL: " << what << '(' << x << ", " << y << ") reference " << double(t) << (ubit ? " (ubit)" : "")
					          << " expected " << best << (inexact ? " (ubit)" : "") << '\n';
				}
			}
		};
		for (double a : values) {
			for (double b : values) {
				// sum: exact a + b, skipped where it leaves the range (the suites saturate there)
				{
					const double s = a + b;
					const dyadic exact = D(a) + D(b);
					if (std::fabs(s) < maxpos) {
						const Areal ref = ArealReference<Areal>(s, TwoSumError(a, b, s));
						check(ref, [&](double v) { return sign_of(D(v) - exact); }, sign_of(exact), "sum", a, b);
					}
				}
				// quotient: v against a / b by the sign of (v b - a) b, exactly
				if (b != 0.0) {
					const double q = a / b;
					if (std::fabs(q) < maxpos && std::isfinite(q)) {
						const Areal ref = ArealReference<Areal>(q, QuotientError(a, b, q));
						const int bs = (b < 0) ? -1 : 1;
						const int exactSign = (a == 0.0) ? 0 : (((a < 0) != (b < 0)) ? -1 : 1);
						check(ref, [&](double v) { return sign_of(D(v) * D(b) - D(a)) * bs; }, exactSign, "quotient", a, b);
					}
				}
			}
		}
		// the non-finite results and an exact error pass through: ArealReference(s, 0) is areal(s), exact
		{
			const Areal r = ArealReference<Areal>(1.5, 0.0);
			if (r.at(0) || double(r) != 1.5) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: ArealReference(1.5, 0) is not an exact 1.5\n";
			}
		}
		return nrOfFailedTestCases;
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
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "areal reference helpers against an exact oracle (#2060)";
	std::string test_tag    = "reference oracle";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyArealReference<areal<8, 3>>(reportTestCases), "areal<8,3>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyTwoSumError(reportTestCases), "TwoSumError", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyQuotientError(reportTestCases), "QuotientError", test_tag);
	// areal<9,6>: operand exponents that span 63 binades, where a + b rounds in double (#2022)
	nrOfFailedTestCases += ReportTestResult(VerifyArealReference<areal<9, 6>>(reportTestCases), "areal<9,6>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyArealReference<areal<8, 3>>(reportTestCases), "areal<8,3>", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyArealReference<areal<10, 4>>(reportTestCases), "areal<10,4>", test_tag);
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
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
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
