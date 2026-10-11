#pragma once
// test_suite_multidouble_exact.hpp: verification of the exact functions of the multi-double types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Shared by the regression suites of dd, qd, dd_cascade, td_cascade and qd_cascade (#1667).  The
// references are independent of the code under test:
//
//   - double arguments: fmod and remainder are exact in IEEE arithmetic, so libm's result is the
//     answer, bit for bit, across the whole double range;
//   - multi-limb arguments: x is built as k y + r from a chosen integer k, and the expected
//     remainder x - k y is formed exactly in ereal from its limbs, by addition and
//     multiplication only;
//   - floor, ceil, trunc and round are checked by their defining properties: every limb an
//     integer, and the exact difference from x in range.
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <universal/number/ereal/ereal.hpp>

namespace sw { namespace universal { namespace multidouble_verification {

	using Exact = ereal<19>;   // exact for the sums and the products of limbs formed here

	template<typename Real>
	constexpr int nlimbs = std::numeric_limits<Real>::digits / 53;

	template<typename Real>
	Exact exact(const Real& x) {
		Exact e(0.0);
		for (int i = 0; i < nlimbs<Real>; ++i) e += Exact(x[i]);
		return e;
	}

	inline Exact eabs(const Exact& v) { return v < Exact(0.0) ? -v : v; }

	// a value with up to N limbs, the lower ones at random gaps below the last bit of the one above
	template<typename Real>
	Real random_value(std::mt19937_64& rng, int emin, int emax) {
		std::uniform_real_distribution<double> u(1.0, 2.0);
		std::uniform_int_distribution<int> e(emin, emax), gap(1, 40), coin(0, 3);
		const double lead = std::ldexp(u(rng), e(rng)) * (coin(rng) < 2 ? -1.0 : 1.0);
		Real v(lead);
		double prev = lead;
		for (int i = 1; i < nlimbs<Real>; ++i) {
			if (coin(rng) == 0) break;
			const int ex = std::ilogb(prev) - 53 - gap(rng);
			if (ex < -1000) break;
			const double next = std::ldexp(u(rng), ex) * (coin(rng) < 2 ? -1.0 : 1.0);
			v += Real(next);
			prev = next;
		}
		return v;
	}

	template<typename Real>
	bool is_integral(const Real& v) {
		for (int i = 0; i < nlimbs<Real>; ++i) if (std::floor(v[i]) != v[i]) return false;
		return true;
	}

	// floor, ceil, trunc and round of random and crafted values, by their defining properties
	template<typename Real>
	int VerifyTruncation(const std::string& tag, int samples, bool reportTestCases) {
		std::mt19937_64 rng(0x1667u);
		int fails = 0;
		auto check = [&](const Real& x) {
			const Exact ex = exact(x);
			const Real f = floor(x), c = ceil(x), t = trunc(x), r = round(x);
			const Exact ef = exact(f), ec = exact(c), et = exact(t), er = exact(r);
			bool ok = is_integral(f) && is_integral(c) && is_integral(t) && is_integral(r);
			ok = ok && ef <= ex && ex - ef < Exact(1.0);                 // floor: x - 1 < f <= x
			ok = ok && ec >= ex && ec - ex < Exact(1.0);                 // ceil:  x <= c < x + 1
			ok = ok && et == (ex < Exact(0.0) ? ec : ef);                // trunc toward zero
			const Exact d = eabs(ex - er);                                // round: nearest, ties away
			ok = ok && d <= Exact(0.5) && (d < Exact(0.5) || eabs(er) > eabs(ex));
			if (!ok) {
				++fails;
				if (reportTestCases) std::cerr << "  FAIL " << tag << " truncation of " << x[0] << " + ...: floor " << double(f) << " ceil " << double(c) << " trunc " << double(t) << " round " << double(r) << '\n';
			}
		};
		// crafted: integers beyond 2^53 with halves, and halves perturbed in the lowest limb
		for (int e : { 0, 20, 52, 53, 60, 80, 120 }) {
			for (double frac : { 0.0, 0.5, -0.5, 0.25, 0.75 }) {
				for (double tiny : { 0.0, 1.0e-40, -1.0e-40 }) {
					const Real base = Real(std::ldexp(1.0, e)) + Real(3.0);
					const Real x = base + Real(frac) + Real(tiny);
					check(x);
					check(Real(-x));
				}
			}
		}
		for (int i = 0; i < samples; ++i) check(random_value<Real>(rng, -10, 160));
		return fails;
	}

	// fmod and remainder of doubles: libm's exact result, across the whole range
	template<typename Real>
	int VerifyRemainderDoubles(const std::string& tag, int samples, bool reportTestCases) {
		std::mt19937_64 rng(0x1668u);
		std::uniform_real_distribution<double> u(1.0, 2.0);
		std::uniform_int_distribution<int> e(-1074, 1023), coin(0, 1);
		int fails = 0;
		auto check = [&](double a, double b) {
			const Real fm = fmod(Real(a), Real(b)), rm = remainder(Real(a), Real(b));
			const double efm = std::fmod(a, b), erm = std::remainder(a, b);
			const bool ok_f = (fm[0] == efm) && (std::signbit(fm[0]) == std::signbit(efm)) && exact(fm) == Exact(efm);
			const bool ok_r = (rm[0] == erm) && exact(rm) == Exact(erm);
			if (!ok_f || !ok_r) {
				++fails;
				if (reportTestCases) std::cerr << "  FAIL " << tag << " fmod/remainder(" << a << ", " << b << ") = " << double(fm) << ", " << double(rm) << "; libm " << efm << ", " << erm << '\n';
			}
		};
		for (double a : { 2.5, 3.5, -2.5, 7.0, 0.41943359375, 1.0e300, -1.0e300, std::numeric_limits<double>::max() }) {
			for (double b : { 1.0, -1.0, 2.0, 3.0, 5.1200389862060547e-05, 0.1, std::numeric_limits<double>::denorm_min(), 1.0e-300 }) check(a, b);
		}
		for (int i = 0; i < samples; ++i) {
			const double a = std::ldexp(u(rng) - (coin(rng) ? 0.0 : 3.0), e(rng));
			double b = std::ldexp(u(rng), e(rng));
			if (coin(rng)) b = -b;
			if (!std::isfinite(a) || !std::isfinite(b)) continue;
			check(a, b);
		}
		return fails;
	}

	// fmod and remainder of multi-limb values: x = k y + r for a chosen integer k, the expected
	// remainder formed exactly, and the result its rounding to N limbs
	template<typename Real>
	int VerifyRemainderLimbs(const std::string& tag, int samples, bool reportTestCases) {
		std::mt19937_64 rng(0x1669u);
		std::uniform_real_distribution<double> u(0.0, 1.0);
		std::uniform_int_distribution<int> kb(0, 50), ks(0, 150), coin(0, 1);
		int fails = 0, checked = 0;
		const Exact tol = Exact(std::ldexp(1.0, -(std::numeric_limits<Real>::digits - 2)));
		for (int i = 0; i < samples; ++i) {
			const Real y = random_value<Real>(rng, -40, 40);
			const Exact ey = exact(y), ay = eabs(ey);
			// k = k0 2^s + k1, an integer of up to ~200 bits with a gap in the middle
			const double k0 = std::floor(std::ldexp(u(rng), kb(rng)));
			const double k1 = std::floor(std::ldexp(u(rng), 50));
			const Real k = Real(std::ldexp(k0, ks(rng))) + Real(k1);
			const Exact ek = exact(k);
			const bool rem = coin(rng) != 0;
			// fmod: r in [0, 1) |y| with y's sign; remainder: r in (-1/2, 1/2) |y|
			const double frac = rem ? (u(rng) - 0.5) * 0.999 : u(rng);
			Real x = (k + Real(frac)) * y;                       // x / y = k + frac
			if (coin(rng)) x = -x;
			const Exact ex = exact(x);
			// the exact remainder for this k: x was rounded, so confirm it is still in range
			const Exact sk = (ex < Exact(0.0)) == (ey < Exact(0.0)) ? ek : -ek;
			const Exact er = ex - sk * ey;
			bool in_range;
			if (rem) in_range = eabs(er) * Exact(2.0) < ay;
			else     in_range = eabs(er) < ay && (er.iszero() || (er < Exact(0.0)) == (ex < Exact(0.0)));
			if (!in_range) continue;
			++checked;
			const Real got = rem ? remainder(x, y) : fmod(x, y);
			const Exact diff = eabs(exact(got) - er);
			if (diff > eabs(er) * tol) {
				++fails;
				if (reportTestCases) std::cerr << "  FAIL " << tag << (rem ? " remainder" : " fmod") << " of " << double(x) << " by " << double(y) << ": " << double(got) << ", expected " << double(er) << '\n';
			}
		}
		if (checked < samples / 2) {
			++fails;
			if (reportTestCases) std::cerr << "  FAIL " << tag << " only " << checked << " of " << samples << " remainder cases were usable\n";
		}
		return fails;
	}

	// hypot against the exact sum of squares, and min and max returning an operand whole
	template<typename Real>
	int VerifyHypotMinMax(const std::string& tag, int samples, bool reportTestCases) {
		std::mt19937_64 rng(0x166au);
		int fails = 0;
		const double tol = std::ldexp(1.0, -(std::numeric_limits<Real>::digits - 4));
		for (int i = 0; i < samples; ++i) {
			const Real x = random_value<Real>(rng, -20, 20), y = random_value<Real>(rng, -20, 20);
			const Real h = hypot(x, y);
			// h^2 against x^2 + y^2, both exact in ereal: relative error of h is half of h^2's
			const Exact eh = exact(h), s = exact(x) * exact(x) + exact(y) * exact(y);
			const double rel = double(eabs(eh * eh - s) / s);
			if (!(rel <= 2.0 * tol)) {
				++fails;
				if (reportTestCases) std::cerr << "  FAIL " << tag << " hypot(" << double(x) << ", " << double(y) << ") relative error " << rel << '\n';
			}
			const Real mn = min(x, y), mx = max(x, y);
			const bool xs = exact(x) < exact(y);
			const Real emn = xs ? x : y, emx = xs ? y : x;
			if (!(exact(mn) == exact(emn)) || !(exact(mx) == exact(emx))) {
				++fails;
				if (reportTestCases) std::cerr << "  FAIL " << tag << " min/max of " << double(x) << ", " << double(y) << '\n';
			}
		}
		// far beyond double's range for the squares
		for (double scale : { 1.0e300, 1.0e-300 }) {
			const Real h = hypot(Real(3.0 * scale), Real(4.0 * scale));
			if (!(std::fabs(double(h) / (5.0 * scale) - 1.0) < 1.0e-15)) {
				++fails;
				if (reportTestCases) std::cerr << "  FAIL " << tag << " hypot at scale " << scale << ": " << double(h) << '\n';
			}
		}
		return fails;
	}

	// log1p near 0: against its series below 2^-60, where x^5 / 5 is beyond quad-double's reach
	template<typename Real>
	int VerifyLog1pSmall(const std::string& tag, int samples, bool reportTestCases) {
		std::mt19937_64 rng(0x166bu);
		int fails = 0;
		const double tol = std::ldexp(1.0, -(std::numeric_limits<Real>::digits - 6));
		for (int i = 0; i < samples; ++i) {
			const Real a = random_value<Real>(rng, -300, -60);
			const Exact x = exact(a);
			const Exact ref = x - x * x / Exact(2.0) + x * x * x / Exact(3.0) - x * x * x * x / Exact(4.0);
			const double rel = double(eabs(exact(log1p(a)) - ref) / eabs(ref));
			if (!(rel <= tol)) {
				++fails;
				if (reportTestCases) std::cerr << "  FAIL " << tag << " log1p(" << double(a) << ") relative error " << rel << '\n';
			}
		}
		return fails;
	}

}}}  // namespace sw::universal::multidouble_verification
