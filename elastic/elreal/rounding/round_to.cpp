// round_to.cpp: regression tests for elreal Phase 8 real floating-point conversion (#932).
//
// round_to<Target>(ZBCL<FpType>, RoundingMode) rounds an exact-real elreal stream
// to double / float / dd / qd. This suite validates:
//   - correctly-rounded: every result matches the exact value rounded to the
//     target's significand bits, verified against the independent einteger dyadic
//     oracle, for all four rounding modes;
//   - idempotence: round_to<T>(round_to<T>(x)) == round_to<T>(x);
//   - monotonicity across nested precisions: (double,float), (dd,double), (qd,dd);
//   - lossless roundtrip: round_to<double>(from_native<double>(x)) == x;
//   - determinism: the result depends only on the value and mode, not the path
//     that produced the ZBCL.
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

#include <universal/number/elreal/elreal.hpp>
#include <universal/number/dd/dd.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/verification/elreal_reference_digits.hpp>   // dyadic, zbcl_to_dyadic
#include <universal/verification/test_suite.hpp>

namespace {

	using namespace sw::universal;

	// ---- exact-dyadic reference machinery (the independent oracle) ------------
	int dcmp(const dyadic& a, const dyadic& b) {
		dyadic d = a - b;
		if (d.numerator.iszero()) return 0;
		return d.numerator.sign() ? -1 : 1;
	}
	using bigint = einteger<std::uint32_t>;
	bigint shl(bigint a, int n) { a <<= n; return a; }
	bigint shr(bigint a, int n) { a >>= n; return a; }

	// round an exact dyadic to P significand bits under `mode` -> exact dyadic
	dyadic ref_round_P(dyadic D, int P, RoundingMode mode) {
		if (D.numerator.iszero()) return D;
		bigint n = D.numerator;
		bool   neg = n.sign();
		if (neg) n = -n;
		int msb = -1;
		{ bigint t = n; int b = 0; while (!t.iszero()) { t >>= 1; ++b; } msb = b - 1; }
		int e0 = msb + D.scale;
		int cut = e0 - P + 1;
		int shift = cut - D.scale;
		if (shift <= 0) return D;                          // already fits in P bits
		bigint keep = shr(n, shift);
		bigint rem  = n - shl(keep, shift);
		bigint half = shl(bigint(1), shift - 1);
		bool remZero = rem.iszero();
		int rc = 0; { bigint d2 = rem - half; if (d2.iszero()) rc = 0; else rc = d2.sign() ? -1 : 1; }
		bool up = false;
		switch (mode) {
		case RoundingMode::RoundToZero:         break;
		case RoundingMode::RoundTowardPositive: up = !neg && !remZero; break;
		case RoundingMode::RoundTowardNegative: up =  neg && !remZero; break;
		case RoundingMode::RoundToNearest:
		default:
			if (rc > 0) up = true;
			else if (rc == 0) { bigint k2 = keep; bool odd = !((k2 - shl(shr(k2, 1), 1)).iszero()); up = odd; }
			break;
		}
		if (up) keep = keep + bigint(1);
		return dyadic(neg ? -keep : keep, cut);
	}

	dyadic dd_to_dyadic(const dd& v) { return dyadic::from_double(v.high()) + dyadic::from_double(v.low()); }
	dyadic qd_to_dyadic(const qd& v) { dyadic d; for (int i = 0; i < 4; ++i) d = d + dyadic::from_double(v[i]); return d; }

	// build a ZBCL that exactly represents a dd / qd value (its limbs are already
	// non-overlapping doubles), so a rounded result can be fed back in.
	ZBCL<double> zbcl_of(const dd& v) { return add(from_native<double>(v.high()), from_native<double>(v.low())); }
	ZBCL<double> zbcl_of(const qd& v) {
		ZBCL<double> z;
		for (int i = 0; i < 4; ++i) z = add(z, from_native<double>(v[i]));
		return z;
	}

	// a random exact-real elreal value built from a few scaled rationals
	elreal<double> random_value(std::mt19937_64& rng, int maxTerms = 6) {
		elreal<double> v(0.0);
		int nt = 1 + static_cast<int>(rng() % static_cast<unsigned>(maxTerms));
		for (int j = 0; j < nt; ++j) {
			double m = static_cast<double>(static_cast<std::int64_t>(rng() % 8000) - 4000) / static_cast<double>(1 + (rng() % 256));
			int sc = static_cast<int>(rng() % 60) - 30;
			v = v + elreal<double>(std::ldexp(m, sc));
			v.precision(12);
		}
		return v;
	}

	const RoundingMode kModes[4] = {
		RoundingMode::RoundToNearest, RoundingMode::RoundToZero,
		RoundingMode::RoundTowardPositive, RoundingMode::RoundTowardNegative
	};
	const char* kModeName[4] = { "RN", "RZ", "R+", "R-" };

	// ---- 1. correctly-rounded vs the exact dyadic oracle, all targets/modes ---
	int VerifyCorrectlyRounded(bool reportTestCases, int nrTests) {
		int fails = 0;
		std::mt19937_64 rng(0xC0FFEE);
		for (int t = 0; t < nrTests; ++t) {
			elreal<double> v = random_value(rng);
			if (v.iszero()) continue;
			dyadic D = zbcl_to_dyadic(v.stream());
			for (int mi = 0; mi < 4; ++mi) {
				RoundingMode m = kModes[mi];
				if (dcmp(dyadic::from_double(round_to<double>(v.stream(), m)), ref_round_P(D, 53, m)) != 0) {
					if (reportTestCases) { std::cout << "    FAIL round_to<double> " << kModeName[mi] << "\n"; } ++fails;
				}
				if (dcmp(dyadic::from_double(static_cast<double>(round_to<float>(v.stream(), m))), ref_round_P(D, 24, m)) != 0) {
					if (reportTestCases) { std::cout << "    FAIL round_to<float> " << kModeName[mi] << "\n"; } ++fails;
				}
				if (dcmp(dd_to_dyadic(round_to<dd>(v.stream(), m)), ref_round_P(D, 106, m)) != 0) {
					if (reportTestCases) { std::cout << "    FAIL round_to<dd> " << kModeName[mi] << "\n"; } ++fails;
				}
				if (dcmp(qd_to_dyadic(round_to<qd>(v.stream(), m)), ref_round_P(D, 212, m)) != 0) {
					if (reportTestCases) { std::cout << "    FAIL round_to<qd> " << kModeName[mi] << "\n"; } ++fails;
				}
			}
		}
		return fails;
	}

	// ---- 2. idempotence: round_to<T>(round_to<T>(x)) == round_to<T>(x) --------
	int VerifyIdempotence(bool reportTestCases, int nrTests) {
		int fails = 0;
		std::mt19937_64 rng(0x1DE);
		for (int t = 0; t < nrTests; ++t) {
			elreal<double> v = random_value(rng);
			if (v.iszero()) continue;
			for (int mi = 0; mi < 4; ++mi) {
				RoundingMode m = kModes[mi];
				double rd = round_to<double>(v.stream(), m);
				if (round_to<double>(from_native<double>(rd), m) != rd) { if (reportTestCases) { std::cout << "    FAIL idempotent double " << kModeName[mi] << "\n"; } ++fails; }
				dd rdd = round_to<dd>(v.stream(), m);
				if (dcmp(dd_to_dyadic(round_to<dd>(zbcl_of(rdd), m)), dd_to_dyadic(rdd)) != 0) { if (reportTestCases) { std::cout << "    FAIL idempotent dd " << kModeName[mi] << "\n"; } ++fails; }
				qd rqd = round_to<qd>(v.stream(), m);
				if (dcmp(qd_to_dyadic(round_to<qd>(zbcl_of(rqd), m)), qd_to_dyadic(rqd)) != 0) { if (reportTestCases) { std::cout << "    FAIL idempotent qd " << kModeName[mi] << "\n"; } ++fails; }
			}
		}
		return fails;
	}

	// ---- 3. monotonicity across nested precisions ----------------------------
	//   round_to<P2>(round_to<P1>(x)) == round_to<P2>(x)  for P1 >= P2:
	//   (double,float), (dd,double), (qd,dd) -- i.e. no double-rounding error.
	int VerifyMonotonicity(bool reportTestCases, int nrTests) {
		int fails = 0;
		std::mt19937_64 rng(0x3EE);
		for (int t = 0; t < nrTests; ++t) {
			elreal<double> v = random_value(rng);
			if (v.iszero()) continue;
			for (int mi = 0; mi < 4; ++mi) {
				RoundingMode m = kModes[mi];
				// (double, float)
				float viaD = round_to<float>(from_native<double>(round_to<double>(v.stream(), m)), m);
				float direct = round_to<float>(v.stream(), m);
				if (viaD != direct) { if (reportTestCases) { std::cout << "    FAIL monotone (double,float) " << kModeName[mi] << "\n"; } ++fails; }
				// (dd, double)
				double viaDD = round_to<double>(zbcl_of(round_to<dd>(v.stream(), m)), m);
				double directD = round_to<double>(v.stream(), m);
				if (viaDD != directD) { if (reportTestCases) { std::cout << "    FAIL monotone (dd,double) " << kModeName[mi] << "\n"; } ++fails; }
				// (qd, dd)
				dd viaQD = round_to<dd>(zbcl_of(round_to<qd>(v.stream(), m)), m);
				dd directDD = round_to<dd>(v.stream(), m);
				if (dcmp(dd_to_dyadic(viaQD), dd_to_dyadic(directDD)) != 0) { if (reportTestCases) { std::cout << "    FAIL monotone (qd,dd) " << kModeName[mi] << "\n"; } ++fails; }
			}
		}
		return fails;
	}

	// ---- 4. lossless roundtrip: round_to<double>(from_native(x)) == x --------
	int VerifyLosslessRoundtrip(bool reportTestCases, int nrTests) {
		int fails = 0;
		std::mt19937_64 rng(0x105);
		std::uniform_real_distribution<double> dist(-1e30, 1e30);
		for (int t = 0; t < nrTests; ++t) {
			double x = (t == 0) ? 0.0 : dist(rng);
			for (int mi = 0; mi < 4; ++mi) {
				if (round_to<double>(from_native<double>(x), kModes[mi]) != x) {
					if (reportTestCases) { std::cout << "    FAIL roundtrip " << x << " " << kModeName[mi] << "\n"; } ++fails;
				}
			}
		}
		return fails;
	}

	// ---- 5. determinism: two constructions of the same value round equally ----
	int VerifyDeterminism(bool reportTestCases, int nrTests) {
		int fails = 0;
		std::mt19937_64 rng(0xDE7);
		for (int t = 0; t < nrTests; ++t) {
			// build the same value two ways: a+b+c vs c+b+a
			double a = std::ldexp(1.0 + (rng() % 1000) / 1000.0, static_cast<int>(rng() % 20));
			double b = std::ldexp(1.0 + (rng() % 1000) / 1000.0, static_cast<int>(rng() % 20) - 40);
			double c = std::ldexp(1.0 + (rng() % 1000) / 1000.0, static_cast<int>(rng() % 20) - 80);
			elreal<double> v1 = elreal<double>(a) + elreal<double>(b) + elreal<double>(c); v1.precision(12);
			elreal<double> v2 = elreal<double>(c) + elreal<double>(b) + elreal<double>(a); v2.precision(12);
			for (int mi = 0; mi < 4; ++mi) {
				RoundingMode m = kModes[mi];
				if (round_to<double>(v1.stream(), m) != round_to<double>(v2.stream(), m)) { if (reportTestCases) { std::cout << "    FAIL determinism double " << kModeName[mi] << "\n"; } ++fails; }
				if (dcmp(qd_to_dyadic(round_to<qd>(v1.stream(), m)), qd_to_dyadic(round_to<qd>(v2.stream(), m))) != 0) { if (reportTestCases) { std::cout << "    FAIL determinism qd " << kModeName[mi] << "\n"; } ++fails; }
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
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;
	std::string test_suite = "elreal Phase 8 (#932) real floating-point conversion round_to<double|float|dd|qd>";
	int nrOfFailedTestCases = 0;
	bool reportTestCases = true;
	ReportTestSuiteHeader(test_suite, reportTestCases);

	int base = 2000;
#if REGRESSION_LEVEL_2
	base = 5000;
#endif

	nrOfFailedTestCases += ReportTestResult(VerifyCorrectlyRounded(reportTestCases, base), "round_to correctly-rounded (all targets/modes)", "round_to");
	nrOfFailedTestCases += ReportTestResult(VerifyIdempotence(reportTestCases, base), "round_to idempotence", "idempotent");
	nrOfFailedTestCases += ReportTestResult(VerifyMonotonicity(reportTestCases, base), "round_to monotonicity (double,float)/(dd,double)/(qd,dd)", "monotone");
	nrOfFailedTestCases += ReportTestResult(VerifyLosslessRoundtrip(reportTestCases, base), "round_to<double> lossless roundtrip", "roundtrip");
	nrOfFailedTestCases += ReportTestResult(VerifyDeterminism(reportTestCases, base), "round_to determinism", "determinism");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
	std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
