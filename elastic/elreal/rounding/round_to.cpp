// round_to.cpp: regression tests for elreal Phase 8 real floating-point conversion (#932, #1182).
//
// round_to<Target>(ZBCL<FpType>, RoundingMode) rounds an exact-real elreal stream
// to double / float / dd / qd (#932) or to an arbitrary cfloat<nbits,es,...>
// (#1182). This suite validates:
//   - correctly-rounded: every result matches the exact value rounded to the
//     target's significand bits, verified against the independent einteger dyadic
//     oracle, for all four rounding modes;
//   - idempotence: round_to<T>(round_to<T>(x)) == round_to<T>(x);
//   - monotonicity across nested precisions: (double,float), (dd,double), (qd,dd);
//   - lossless roundtrip: round_to<double>(from_native<double>(x)) == x, and for
//     cfloat, round_to<Cf>(exact-value-of-c) == c for every finite c;
//   - determinism: the result depends only on the value and mode, not the path
//     that produced the ZBCL;
//   - cfloat exponent-range handling: overflow to +/-inf (or maxpos/maxneg),
//     gradual underflow to subnormals, and flush-to-zero, across a spread of
//     <nbits,es> configs and subnormal/max-exponent flag combinations.
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
#include <universal/number/cfloat/cfloat.hpp>
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

	// ==== cfloat targets (#1182) ==============================================
	//
	// A cfloat has a finite es-bit exponent range, so round_to<cfloat> has to
	// classify the value as normal, subnormal, overflow, or flush-to-zero, and
	// round the significand at the right absolute bit position for that region.
	// The oracle below re-derives the correctly-rounded cfloat *independently*
	// from the exact dyadic using bigint arithmetic, then we compare bit patterns.

	// expected_cfloat<Cf>(D, mode): round the exact dyadic D onto cfloat<Cf>'s
	// representable grid under `mode`, returning the target cfloat. This mirrors
	// the number system's encoding but derives the mantissa entirely with einteger
	// (a different substrate than the round.hpp double-expansion implementation).
	template<typename Cf>
	Cf expected_cfloat(dyadic D, RoundingMode mode) {
		Cf r;
		if (D.numerator.iszero()) { r.setzero(); return r; }
		bigint n = D.numerator;
		bool   neg = n.sign();
		if (neg) n = -n;
		int msb = -1; { bigint t = n; int b = 0; while (!t.iszero()) { t >>= 1; ++b; } msb = b - 1; }
		int e0 = msb + D.scale;                                  // true binary exponent

		constexpr int  fbits             = static_cast<int>(Cf::fbits);
		constexpr int  MIN_EXP_NORMAL    = Cf::MIN_EXP_NORMAL;
		constexpr int  MIN_EXP_SUBNORMAL = Cf::MIN_EXP_SUBNORMAL;
		constexpr int  MAX_EXP           = Cf::MAX_EXP;
		constexpr int  EXP_BIAS          = Cf::EXP_BIAS;
		constexpr bool hasSubnormals     = Cf::hasSubnormals;

		int  cut; bool normalRegion;
		if (e0 >= MIN_EXP_NORMAL) { cut = e0 - fbits;        normalRegion = true;  }
		else if (hasSubnormals)   { cut = MIN_EXP_SUBNORMAL; normalRegion = false; }
		else                      { cut = MIN_EXP_NORMAL;    normalRegion = false; }

		// round n * 2^scale at absolute bit `cut` -> non-negative integer mantissa
		int   shift = cut - D.scale;
		bigint keep, rem;
		if (shift <= 0) { keep = shl(n, -shift); rem = bigint(0); }
		else            { keep = shr(n, shift);  rem = n - shl(keep, shift); }
		bool up = false;
		if (shift > 0 && !rem.iszero()) {
			bigint half = shl(bigint(1), shift - 1);
			bigint d2 = rem - half; int rc = d2.iszero() ? 0 : (d2.sign() ? -1 : 1);
			switch (mode) {
			case RoundingMode::RoundToZero:         break;
			case RoundingMode::RoundTowardPositive: up = !neg; break;
			case RoundingMode::RoundTowardNegative: up =  neg; break;
			case RoundingMode::RoundToNearest:
			default:
				if (rc > 0) up = true;
				else if (rc == 0) { bigint k2 = keep; up = !((k2 - shl(shr(k2, 1), 1)).iszero()); }
				break;
			}
		}
		if (up) keep = keep + bigint(1);
		std::uint64_t absM = 0; { bigint t = keep; int b = 0; while (!t.iszero()) { std::uint64_t bit = ((t - shl(shr(t, 1), 1)).iszero() ? 0ull : 1ull); absM |= (bit << b); t >>= 1; ++b; } }

		auto assemble = [&](std::uint64_t be, std::uint64_t fr) -> Cf {
			std::uint64_t raw = (neg ? 1ull : 0ull); raw <<= Cf::es; raw |= be; raw <<= Cf::fbits; raw |= fr;
			Cf c; c.setbits(raw);
			if constexpr (Cf::isSaturating) { if (c.isnan()) { if (neg) c.maxneg(); else c.maxpos(); } }
			else { if (c.isnan()) c.setinf(neg); }
			return c;
		};
		auto overflow = [&]() -> Cf {
			Cf c; bool toInf;
			switch (mode) {
			case RoundingMode::RoundToZero:         toInf = false; break;
			case RoundingMode::RoundTowardPositive: toInf = !neg;  break;
			case RoundingMode::RoundTowardNegative: toInf =  neg;  break;
			default:                                toInf = true;  break;
			}
			if constexpr (Cf::isSaturating) toInf = false;
			if (toInf) c.setinf(neg); else { if (neg) c.maxneg(); else c.maxpos(); }
			return c;
		};

		if (normalRegion) {
			int expv = e0;
			if (absM == (1ull << (fbits + 1))) { absM >>= 1; ++expv; }
			if (expv > MAX_EXP) return overflow();
			return assemble(static_cast<std::uint64_t>(expv + EXP_BIAS), absM & Cf::ALL_ONES_FR);
		}
		else {
			if (absM == 0) { Cf c; c.setzero(); c.setsign(neg); return c; }
			if constexpr (!hasSubnormals) return assemble(static_cast<std::uint64_t>(MIN_EXP_NORMAL + EXP_BIAS), 0ull);
			if (absM >= (1ull << fbits)) return assemble(static_cast<std::uint64_t>(MIN_EXP_NORMAL + EXP_BIAS), 0ull);
			return assemble(0ull, absM);
		}
	}

	// build a ZBCL that exactly represents a finite cfloat value. Exact when the
	// value's exponent fits the double range and fbits <= 52 (all configs tested).
	template<typename Cf>
	ZBCL<double> zbcl_of_cfloat(const Cf& c) { return from_native<double>(double(c)); }

	// ---- cfloat correctly-rounded vs the independent bigint oracle ------------
	template<typename Cf>
	int VerifyCfloatCorrectlyRounded(bool reportTestCases, int nrTests, std::uint64_t seed, int scaleSpread) {
		int fails = 0;
		std::mt19937_64 rng(seed);
		for (int t = 0; t < nrTests; ++t) {
			elreal<double> v(0.0);
			int nt = 1 + static_cast<int>(rng() % 5);
			for (int j = 0; j < nt; ++j) {
				double m = static_cast<double>(static_cast<std::int64_t>(rng() % 8000) - 4000) / static_cast<double>(1 + (rng() % 256));
				int sc = static_cast<int>(rng() % static_cast<unsigned>(2 * scaleSpread)) - scaleSpread;
				v = v + elreal<double>(std::ldexp(m, sc));
				v.precision(12);
			}
			if (v.iszero()) continue;
			dyadic D = zbcl_to_dyadic(v.stream());
			for (int mi = 0; mi < 4; ++mi) {
				Cf got = round_to<Cf>(v.stream(), kModes[mi]);
				Cf exp = expected_cfloat<Cf>(D, kModes[mi]);
				if (!(got == exp)) {
					if (reportTestCases) std::cout << "    FAIL cfloat correctly-rounded " << kModeName[mi] << " got=" << to_binary(got) << " exp=" << to_binary(exp) << '\n';
					++fails;
				}
			}
		}
		return fails;
	}

	// ---- cfloat lossless roundtrip: round_to<Cf>(exact-value-of-c) == c -------
	// exhaustive over all encodings for narrow configs (strongest encoding check)
	template<typename Cf>
	int VerifyCfloatRoundtrip(bool reportTestCases) {
		int fails = 0;
		static_assert(Cf::nbits <= 16, "exhaustive roundtrip only for narrow cfloat configs");
		const unsigned NR = (1u << Cf::nbits);
		for (unsigned i = 0; i < NR; ++i) {
			Cf c; c.setbits(i);
			if (c.isnan() || c.isinf() || c.iszero()) continue;
			ZBCL<double> z = zbcl_of_cfloat(c);
			for (int mi = 0; mi < 4; ++mi) {
				Cf got = round_to<Cf>(z, kModes[mi]);
				if (!(got == c)) {
					if (reportTestCases) std::cout << "    FAIL cfloat roundtrip " << kModeName[mi] << " c=" << to_binary(c) << " got=" << to_binary(got) << '\n';
					++fails;
				}
			}
		}
		return fails;
	}

	// ---- cfloat idempotence: round_to<Cf>(exact-value-of round_to<Cf>(x)) ----
	template<typename Cf>
	int VerifyCfloatIdempotence(bool reportTestCases, int nrTests, std::uint64_t seed) {
		int fails = 0;
		std::mt19937_64 rng(seed);
		for (int t = 0; t < nrTests; ++t) {
			elreal<double> v = random_value(rng);
			if (v.iszero()) continue;
			for (int mi = 0; mi < 4; ++mi) {
				Cf a = round_to<Cf>(v.stream(), kModes[mi]);
				if (a.isnan() || a.isinf() || a.iszero()) continue;
				Cf b = round_to<Cf>(zbcl_of_cfloat(a), kModes[mi]);
				if (!(b == a)) {
					if (reportTestCases) std::cout << "    FAIL cfloat idempotence " << kModeName[mi] << " a=" << to_binary(a) << " b=" << to_binary(b) << '\n';
					++fails;
				}
			}
		}
		return fails;
	}

	// ---- cfloat overflow / underflow boundaries ------------------------------
	template<typename Cf>
	int VerifyCfloatBoundaries(bool reportTestCases, double big, double tiny) {
		int fails = 0;
		for (int s = 0; s < 2; ++s) {                            // overflow: +-big -> inf/maxpos
			bool sign = (s != 0);
			elreal<double> e(sign ? -big : big);
			for (int mi = 0; mi < 4; ++mi) {
				Cf got = round_to<Cf>(e.stream(), kModes[mi]);
				bool toInf; switch (kModes[mi]) { case RoundingMode::RoundToZero: toInf = false; break; case RoundingMode::RoundTowardPositive: toInf = !sign; break; case RoundingMode::RoundTowardNegative: toInf = sign; break; default: toInf = true; }
				if (Cf::isSaturating) toInf = false;
				Cf exp; if (toInf) exp.setinf(sign); else { if (sign) exp.maxneg(); else exp.maxpos(); }
				if (!(got == exp)) { if (reportTestCases) std::cout << "    FAIL cfloat overflow " << kModeName[mi] << " got=" << to_binary(got) << " exp=" << to_binary(exp) << '\n'; ++fails; }
			}
		}
		for (int s = 0; s < 2; ++s) {                            // underflow: +-tiny -> zero/minpos
			bool sign = (s != 0);
			elreal<double> e(sign ? -tiny : tiny);
			for (int mi = 0; mi < 4; ++mi) {
				Cf got = round_to<Cf>(e.stream(), kModes[mi]);
				Cf exp;
				if      (kModes[mi] == RoundingMode::RoundTowardPositive && !sign) exp.minpos();
				else if (kModes[mi] == RoundingMode::RoundTowardNegative &&  sign) exp.minneg();
				else { exp.setzero(); exp.setsign(sign); }
				if (!(got == exp)) { if (reportTestCases) std::cout << "    FAIL cfloat underflow " << kModeName[mi] << " got=" << to_binary(got) << " exp=" << to_binary(exp) << '\n'; ++fails; }
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
	std::string test_suite = "elreal Phase 8 (#932, #1182) real floating-point conversion round_to<double|float|dd|qd|cfloat>";
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
	base = 5000;
#endif

	nrOfFailedTestCases += ReportTestResult(VerifyCorrectlyRounded(reportTestCases, base), "round_to correctly-rounded (all targets/modes)", "round_to");
	nrOfFailedTestCases += ReportTestResult(VerifyIdempotence(reportTestCases, base), "round_to idempotence", "idempotent");
	nrOfFailedTestCases += ReportTestResult(VerifyMonotonicity(reportTestCases, base), "round_to monotonicity (double,float)/(dd,double)/(qd,dd)", "monotone");
	nrOfFailedTestCases += ReportTestResult(VerifyLosslessRoundtrip(reportTestCases, base), "round_to<double> lossless roundtrip", "roundtrip");
	nrOfFailedTestCases += ReportTestResult(VerifyDeterminism(reportTestCases, base), "round_to determinism", "determinism");

	// ---- cfloat targets (#1182): a spread of <nbits,es> and flag combinations
	using cf32   = cfloat<32, 8, std::uint32_t, true,  true,  false>;   // IEEE single-like
	using cf64   = cfloat<64, 11, std::uint64_t, true, true,  false>;   // IEEE double-like
	using cf16   = cfloat<16, 5, std::uint16_t, true,  true,  false>;   // IEEE half-like
	using cf11   = cfloat<11, 5, std::uint16_t, true,  true,  false>;   // narrow exponent range
	using cf16ns = cfloat<16, 5, std::uint16_t, false, true,  false>;   // no subnormals (flush-to-zero)
	using cf16nn = cfloat<16, 5, std::uint16_t, false, false, false>;   // no subnormals, no max-exp values

	// correctly-rounded vs the independent bigint oracle (spread of scales to reach
	// the normal / subnormal / overflow / flush regions of each config)
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatCorrectlyRounded<cf32>(reportTestCases, base, 0xC0FFEEu, 120), "round_to<cfloat<32,8>> correctly-rounded", "cfloat<32,8>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatCorrectlyRounded<cf64>(reportTestCases, base, 0xBADF00Du, 120), "round_to<cfloat<64,11>> correctly-rounded", "cfloat<64,11>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatCorrectlyRounded<cf16>(reportTestCases, base, 0x5EEDu, 25), "round_to<cfloat<16,5>> correctly-rounded", "cfloat<16,5>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatCorrectlyRounded<cf11>(reportTestCases, base, 0xABCDu, 25), "round_to<cfloat<11,5>> correctly-rounded", "cfloat<11,5>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatCorrectlyRounded<cf16ns>(reportTestCases, base, 0xF00Du, 25), "round_to<cfloat<16,5,no-sub>> correctly-rounded", "cfloat<16,5,no-sub>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatCorrectlyRounded<cf16nn>(reportTestCases, base, 0x1234u, 25), "round_to<cfloat<16,5,no-sub,no-sup>> correctly-rounded", "cfloat<16,5,no-sub,no-sup>");

	// exhaustive lossless roundtrip over every finite encoding (narrow configs)
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatRoundtrip<cf16>(reportTestCases), "round_to<cfloat<16,5>> exhaustive roundtrip", "cfloat<16,5>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatRoundtrip<cf11>(reportTestCases), "round_to<cfloat<11,5>> exhaustive roundtrip", "cfloat<11,5>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatRoundtrip<cf16ns>(reportTestCases), "round_to<cfloat<16,5,no-sub>> exhaustive roundtrip", "cfloat<16,5,no-sub>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatRoundtrip<cf16nn>(reportTestCases), "round_to<cfloat<16,5,no-sub,no-sup>> exhaustive roundtrip", "cfloat<16,5,no-sub,no-sup>");

	// idempotence and overflow/underflow boundaries
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatIdempotence<cf32>(reportTestCases, base, 0x1DEu), "round_to<cfloat<32,8>> idempotence", "cfloat<32,8>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatIdempotence<cf16>(reportTestCases, base, 0x2DEu), "round_to<cfloat<16,5>> idempotence", "cfloat<16,5>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatBoundaries<cf16>(reportTestCases, 1e30, 1e-30), "round_to<cfloat<16,5>> overflow/underflow", "cfloat<16,5>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatBoundaries<cf32>(reportTestCases, 1e300, 1e-300), "round_to<cfloat<32,8>> overflow/underflow", "cfloat<32,8>");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatBoundaries<cf11>(reportTestCases, 1e30, 1e-30), "round_to<cfloat<11,5>> overflow/underflow", "cfloat<11,5>");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
	std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
