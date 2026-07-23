// fma.cpp: functional tests for the quad-double fused multiply-add fma(a,b,c)
//
// qd's fma was declared but never defined (a latent link error, #1190). This
// suite validates the new implementation against an INDEPENDENT exact oracle:
// the dyadic-rational type, which is closed under +, -, * with no rounding, so
// dyadic(a)*dyadic(b) + dyadic(c) is the exact real value of a*b + c. We check:
//   - correctly-rounded: fma(a,b,c) agrees with the exact value to full qd
//     precision (~63 decimal digits), for full-precision random operands;
//   - fused vs naive: under cancellation (c ~ -a*b) fma keeps the low-order
//     product bits that a plain `a*b + c` rounds away, so fma is strictly more
//     accurate than the two-rounding form;
//   - identities and non-finite handling.
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

#include <universal/number/qd/qd.hpp>
#include <universal/verification/dyadic_exact.hpp>              // dyadic
#include <universal/verification/elreal_reference_digits.hpp>   // agreed_decimal_digits(dyadic, dyadic)
#include <universal/verification/test_suite.hpp>

namespace {

	using namespace sw::universal;

	// exact value of a quad-double as a dyadic rational (its four limbs, summed)
	dyadic qd_to_dyadic(const qd& v) {
		dyadic d;
		for (int i = 0; i < 4; ++i) d = d + dyadic::from_double(v[i]);
		return d;
	}

	std::uniform_real_distribution<double> U(-1.0, 1.0);

	// a full 4-limb quad-double: staggered full-mantissa doubles, so products
	// genuinely exceed qd precision and force a rounding.
	qd random_full_qd(std::mt19937_64& rng) {
		int base = static_cast<int>(rng() % 40) - 20;
		qd v(0.0);
		for (int k = 0; k < 4; ++k) v = v + qd(std::ldexp(U(rng), base - 53 * k));
		return v;
	}

	// ---- 1. correctly-rounded to full qd precision vs the exact oracle --------
	int VerifyFmaCorrectlyRounded(bool reportTestCases, int nrTests) {
		int fails = 0;
		std::mt19937_64 rng(0xF3A5C0DE);
		for (int t = 0; t < nrTests; ++t) {
			qd a = random_full_qd(rng), b = random_full_qd(rng), c = random_full_qd(rng);
			dyadic exact = qd_to_dyadic(a) * qd_to_dyadic(b) + qd_to_dyadic(c);
			if (exact.iszero()) continue;
			int digits = agreed_decimal_digits(qd_to_dyadic(fma(a, b, c)), exact, 90);
			if (digits < 60) {   // qd carries ~63 decimal digits; allow faithful-rounding slack
				if (reportTestCases) std::cout << "    FAIL fma correctly-rounded: only " << digits << " digits agree\n";
				++fails;
			}
		}
		return fails;
	}

	// ---- 2. fused: under cancellation fma beats the two-rounding a*b + c -------
	int VerifyFmaFusedBeatsNaive(bool reportTestCases, int nrTests) {
		int fails = 0;
		std::mt19937_64 rng(0xCACE1A7E);
		int tested = 0;
		for (int t = 0; t < nrTests; ++t) {
			qd a = random_full_qd(rng), b = random_full_qd(rng);
			qd prod = a * b;                 // rounded product
			qd c = -prod;                    // cancels the rounded product exactly
			dyadic exact = qd_to_dyadic(a) * qd_to_dyadic(b) + qd_to_dyadic(c);
			if (exact.iszero()) continue;    // product was exactly representable -> no residual
			++tested;
			int df = agreed_decimal_digits(qd_to_dyadic(fma(a, b, c)), exact, 90);
			int dn = agreed_decimal_digits(qd_to_dyadic(a * b + c), exact, 90);
			// fma must keep the sub-qd product residual (naive rounds it away -> ~0 digits)
			if (!(df > dn && df >= 40)) {
				if (reportTestCases) std::cout << "    FAIL fma-vs-naive: fma=" << df << " naive=" << dn << " digits\n";
				++fails;
			}
		}
		if (tested == 0 && reportTestCases) std::cout << "    NOTE: no cancellation residual exercised\n";
		return fails;
	}

	// ---- 3. identities and non-finite handling --------------------------------
	int VerifyFmaIdentities(bool reportTestCases, int nrTests) {
		int fails = 0;
		std::mt19937_64 rng(0x1DE7);
		auto close = [](const qd& x, const qd& y) {   // agree to full qd precision
			dyadic dx = dyadic(); for (int i = 0; i < 4; ++i) dx = dx + dyadic::from_double(x[i]);
			dyadic dy = dyadic(); for (int i = 0; i < 4; ++i) dy = dy + dyadic::from_double(y[i]);
			if (dx.iszero() && dy.iszero()) return true;
			return agreed_decimal_digits(dx, dy, 90) >= 60;
		};
		for (int t = 0; t < nrTests; ++t) {
			qd a = random_full_qd(rng), b = random_full_qd(rng), c = random_full_qd(rng);
			// fma(a, 1, c) == a + c
			if (!close(fma(a, qd(1.0), c), a + c)) { if (reportTestCases) std::cout << "    FAIL fma(a,1,c)!=a+c\n"; ++fails; }
			// fma(a, b, 0) == a * b
			if (!close(fma(a, b, qd(0.0)), a * b)) { if (reportTestCases) std::cout << "    FAIL fma(a,b,0)!=a*b\n"; ++fails; }
			// fma(a, 0, c) == c
			if (!close(fma(a, qd(0.0), c), c)) { if (reportTestCases) std::cout << "    FAIL fma(a,0,c)!=c\n"; ++fails; }
		}
		// non-finite operands defer to a*b + c semantics
		qd inf(SpecificValue::infpos), nan(SpecificValue::qnan), one(1.0), two(2.0);
		if (!fma(inf, one, one).isinf()) { if (reportTestCases) std::cout << "    FAIL fma(inf,1,1) not inf\n"; ++fails; }
		if (!fma(nan, one, one).isnan()) { if (reportTestCases) std::cout << "    FAIL fma(nan,1,1) not nan\n"; ++fails; }
		if (!(fma(two, two, one) == qd(5.0)))   { if (reportTestCases) std::cout << "    FAIL fma(2,2,1)!=5\n"; ++fails; }
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
	std::string test_suite = "quad-double (qd) fused multiply-add fma(a,b,c) (#1190)";
	int nrOfFailedTestCases = 0;
	bool reportTestCases = true;
	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// TODO: place hand-run diagnostics here (this branch ignores failures)

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

	int base = 2000;
#if REGRESSION_LEVEL_2
	base = 10000;
#endif

	nrOfFailedTestCases += ReportTestResult(VerifyFmaCorrectlyRounded(reportTestCases, base), "fma correctly-rounded vs exact dyadic", "fma");
	nrOfFailedTestCases += ReportTestResult(VerifyFmaFusedBeatsNaive(reportTestCases, base), "fma fused (beats a*b+c under cancellation)", "fma");
	nrOfFailedTestCases += ReportTestResult(VerifyFmaIdentities(reportTestCases, base / 4 + 1), "fma identities + non-finite", "fma");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
	std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
