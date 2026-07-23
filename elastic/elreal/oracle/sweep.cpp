// sweep.cpp: elreal Phase 9 (#933) high-precision sweep oracle.
//
// Randomised evaluation of elreal ZBCL arithmetic against an INDEPENDENT exact
// oracle. Universal is dependency-free, so in place of mpfr the oracle is the
// exact dyadic-rational type (verification/dyadic_exact.hpp): dyadics are closed
// under +, -, * with no rounding, so they pin down the exact value of any finite
// binary computation. zbcl_to_dyadic() widens a materialised ZBCL to that exact
// value, and we compare.
//
// Checks (swept over host FpType in {double, float}):
//   - construction: a ZBCL built from a fold of exact-in-host doubles equals the
//     exact dyadic sum (the 0-overlap invariant carries no rounding);
//   - add() is EXACT: dyadic(a + b) == dyadic(a) + dyadic(b) to the bit;
//   - sub() (negate + add) is EXACT;
//   - mul(a, b, depth) agrees with the exact dyadic product a*b to a high number
//     of decimal digits (mul truncates at `depth`, so this is a floor, not an
//     equality).
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <utility>

#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/elreal_reference_digits.hpp>   // zbcl_to_dyadic, agreed_decimal_digits
#include <universal/verification/test_suite.hpp>

namespace {

	using namespace sw::universal;

	int dcmp(const dyadic& a, const dyadic& b) {
		dyadic d = a - b;
		if (d.numerator.iszero()) return 0;
		return d.numerator.sign() ? -1 : 1;
	}

	// a value that is exactly representable in every host FpType we sweep (mantissa
	// well within float's 24 bits), so from_native<FpType> carries it losslessly.
	double exact_host_value(std::mt19937_64& rng) {
		int k = static_cast<int>(rng() % (1u << 20)) - (1 << 19);   // |k| < 2^19
		int e = static_cast<int>(rng() % 61) - 30;                  // scale in [-30, 30]
		return std::ldexp(static_cast<double>(k), e);
	}

	// build a random ZBCL together with its exact dyadic value (a fold of exact
	// host values through the exact add() combinator).
	template<typename FpType>
	std::pair<ZBCL<FpType>, dyadic> random_zbcl(std::mt19937_64& rng, int maxTerms = 5) {
		ZBCL<FpType> z = from_native<FpType>(0.0);
		dyadic D;
		int nt = 1 + static_cast<int>(rng() % static_cast<unsigned>(maxTerms));
		for (int j = 0; j < nt; ++j) {
			double m = exact_host_value(rng);
			z = add(z, from_native<FpType>(m));
			D = D + dyadic::from_double(m);
		}
		return { z, D };
	}

	// construction + add() + sub() exactness
	template<typename FpType>
	int VerifyExactAddSub(const char* host, bool reportTestCases, int nrTests) {
		int fails = 0;
		std::mt19937_64 rng(0xE1 + std::hash<std::string>{}(host));
		for (int t = 0; t < nrTests; ++t) {
			auto [za, Da] = random_zbcl<FpType>(rng);
			auto [zb, Db] = random_zbcl<FpType>(rng);
			// construction sanity: the ZBCL exactly equals its dyadic value
			if (dcmp(zbcl_to_dyadic(za), Da) != 0) { if (reportTestCases) std::cout << "    FAIL construction " << host << '\n'; ++fails; }
			// add is exact
			if (dcmp(zbcl_to_dyadic(add(za, zb)), Da + Db) != 0) { if (reportTestCases) std::cout << "    FAIL add-exact " << host << '\n'; ++fails; }
			// sub via negate is exact
			if (dcmp(zbcl_to_dyadic(add(za, negate(zb))), Da - Db) != 0) { if (reportTestCases) std::cout << "    FAIL sub-exact " << host << '\n'; ++fails; }
		}
		return fails;
	}

	// mul() agreement with the exact dyadic product, to a high digit floor
	template<typename FpType>
	int VerifyMulAgreement(const char* host, bool reportTestCases, int nrTests, std::size_t depth, int minDigits) {
		int fails = 0;
		std::mt19937_64 rng(0x33 + std::hash<std::string>{}(host));
		for (int t = 0; t < nrTests; ++t) {
			auto [za, Da] = random_zbcl<FpType>(rng, 3);
			auto [zb, Db] = random_zbcl<FpType>(rng, 3);
			if (Da.iszero() || Db.iszero()) continue;
			ZBCL<FpType> zc = mul(za, zb, depth);
			int digits = agreed_decimal_digits(zbcl_to_dyadic(zc), Da * Db);
			if (digits < minDigits) {
				if (reportTestCases) std::cout << "    FAIL mul-agreement " << host << " digits=" << digits << " < " << minDigits << '\n';
				++fails;
			}
		}
		return fails;
	}

}  // anonymous namespace

#define MANUAL_TESTING 0
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
	std::string test_suite = "elreal Phase 9 (#933) high-precision sweep oracle (vs exact dyadic)";
	int nrOfFailedTestCases = 0;
	bool reportTestCases = true;
	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// TODO: place hand-run diagnostics here (this branch ignores failures)

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

	int base = 500;
#if REGRESSION_LEVEL_2
	base = 2000;
#endif
	// mul truncates at `depth`; each host block carries k bits, so a modest digit
	// floor is comfortably reached without demanding full convergence.
	nrOfFailedTestCases += ReportTestResult(VerifyExactAddSub<double>("double", reportTestCases, base), "elreal<double> add/sub exact vs dyadic", "add/sub");
	nrOfFailedTestCases += ReportTestResult(VerifyExactAddSub<float>("float", reportTestCases, base), "elreal<float> add/sub exact vs dyadic", "add/sub");
	nrOfFailedTestCases += ReportTestResult(VerifyMulAgreement<double>("double", reportTestCases, base, 24, 40), "elreal<double> mul agreement vs dyadic", "mul");
	nrOfFailedTestCases += ReportTestResult(VerifyMulAgreement<float>("float", reportTestCases, base, 24, 30), "elreal<float> mul agreement vs dyadic", "mul");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
	std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
