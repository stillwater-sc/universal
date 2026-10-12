#pragma once
// multidouble_exact.hpp: exact integer-valued and remainder functions of the multi-double types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Shared by dd, qd, dd_cascade, td_cascade and qd_cascade: each is an unevaluated sum of N
// doubles, its limbs, reachable as x[0] .. x[N-1] with x[0] the largest.  These functions used
// to round the argument to double and call libm, which is wrong, not just imprecise: trunc and
// round of a value above 2^53 gave a different integer, and fmod and remainder took the
// remainder of different numbers (#1667).  Here every one is computed on the limbs:
//
//   floor, ceil, trunc, round   exact: the integer is assembled limb by limb and verified
//                               against the exact difference x - n
//   fmod, remainder             exact: x - n y is reduced in an exact expansion, so the result
//                               is the true remainder rounded once to N limbs, whatever x / y
//   hypot                       scaled by a power of two, so the squares cannot overflow
//   log1p                       2 atanh(a / (2 + a)) for |a| < 1/4, where log(1 + a) would
//                               drop every limb of a below 1's last bit
//
// The exact functions keep that promise as long as no partial product of a limb falls below
// the smallest subnormal double, which takes a limb near 2^-1000.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <universal/numerics/error_free_ops.hpp>
#include <universal/number/shared/multidouble_special_constants.hpp>

namespace sw { namespace universal { namespace multidouble {

	// the number of limbs of a multi-double type
	template<typename Real>
	constexpr int limbs = std::numeric_limits<Real>::digits / 53;

	// a four-limb constant, rounded to Real's precision
	template<typename Real>
	Real constant(const double (&c)[4]) {
		Real r(c[3]);
		r += Real(c[2]);
		r += Real(c[1]);
		r += Real(c[0]);
		return r;
	}

	// An exact sum of doubles: Shewchuk's GROW-EXPANSION with zero elimination keeps the
	// components nonoverlapping, ordered from the smallest magnitude up, so the last one carries
	// the sign of the whole.  Capacity is generous: a remainder needs a handful of components.
	class exact_sum {
	public:
		static constexpr int capacity = 96;

		void add(double b) {
			if (b == 0.0) return;
			double q = b;
			int j = 0;
			for (int i = 0; i < n; ++i) {
				double h;
				q = two_sum(q, c[i], h);
				if (h != 0.0) c[j++] = h;
			}
			if (q != 0.0) c[j++] = q;
			n = j;
			if (n == capacity) {
				// out of room: fold the smallest component into the next.  It cannot happen for
				// the remainders and integer parts formed here, which need a few components.
				c[1] += c[0];
				for (int i = 1; i < n; ++i) c[i - 1] = c[i];
				--n;
			}
		}

		// add s * v for a limb sequence v of a multi-double, s = +1 or -1
		template<typename Real>
		void add(const Real& v, double s) { for (int i = 0; i < limbs<Real>; ++i) add(s * v[i]); }

		// add s * every component of another exact sum: exact for s = +1, -1 or 2
		void add(const exact_sum& o, double s) { for (int i = 0; i < o.n; ++i) add(s * o.c[i]); }

		// add m 2^e v exactly: m an integer below 2^26, so each m * limb product is exact
		template<typename Real>
		void add_multiple(const Real& v, double m, int e) {
			for (int i = 0; i < limbs<Real>; ++i) {
				const double scaled = std::ldexp(v[i], e);     // exact: scaling up
				double err;
				const double p = two_prod(m, scaled, err);
				add(p);
				add(err);
			}
		}

		int sign() const { return n == 0 ? 0 : (c[n - 1] > 0.0 ? 1 : -1); }

		// the value to about a double's precision, from the smallest component up
		double estimate() const {
			double s = 0.0;
			for (int i = 0; i < n; ++i) s += c[i];
			return s;
		}

		// the value rounded to a multi-double, from the largest component down
		template<typename Real>
		Real to() const {
			Real r(0.0);
			for (int i = n - 1; i >= 0; --i) r += Real(c[i]);
			return r;
		}

	private:
		double c[capacity]{};
		int n = 0;
	};

	// sign of a - b, exactly
	template<typename Real>
	int compare(const exact_sum& a, const Real& b) {
		exact_sum d = a;
		d.add(b, -1.0);
		return d.sign();
	}

	// floor, exactly: the limbs are floored from the top down to the first that is not an
	// integer, which for a normalized expansion is the whole answer; the exact difference
	// x - n confirms it, and moves n by one if a limb ordering was unusual
	template<typename Real>
	Real floor(const Real& x) {
		if (x.isnan() || x.isinf() || x.iszero()) return x;
		exact_sum n;
		for (int i = 0; i < limbs<Real>; ++i) {
			const double f = std::floor(x[i]);
			n.add(f);
			if (f != x[i]) break;
		}
		for (int k = 0; k < 4; ++k) {
			exact_sum d = n;                                   // d = n - x, wanted in (-1, 0]
			d.add(x, -1.0);
			if (d.sign() > 0) { n.add(-1.0); continue; }
			d.add(1.0);
			if (d.sign() <= 0) { n.add(1.0); continue; }
			break;
		}
		return n.template to<Real>();
	}

	template<typename Real>
	Real ceil(const Real& x) { return -multidouble::floor(Real(-x)); }

	template<typename Real>
	Real trunc(const Real& x) { return x[0] < 0.0 ? multidouble::ceil(x) : multidouble::floor(x); }

	// round to nearest, halfway cases away from zero
	template<typename Real>
	Real round(const Real& x) {
		if (x.isnan() || x.isinf() || x.iszero()) return x;
		const bool negative = x[0] < 0.0;
		const Real a = negative ? Real(-x) : x;
		const Real f = multidouble::floor(a);
		exact_sum d;                                           // a - f - 1/2
		d.add(a, 1.0);
		d.add(f, -1.0);
		d.add(-0.5);
		const Real r = (d.sign() >= 0) ? Real(f + Real(1.0)) : f;
		return negative ? Real(-r) : r;
	}

	namespace detail {

		// Reduce r by multiples of y until |r| < |y| with the sign of the dividend, or zero.
		// Each step subtracts q y exactly, q = m 2^e with m an integer below 2^26: about 25
		// bits of the quotient per step, so even DBL_MAX / 2^-1074 takes under 90 steps.
		// Returns the parity of the integer quotient n in x = n y + r.
		template<typename Real>
		bool reduce(exact_sum& r, const Real& y, int dividend_sign) {
			const double yv = y[0];
			const int ye = std::ilogb(yv);
			const double ym = std::ldexp(yv, -ye);
			bool odd = false;
			for (int step = 0; step < 200; ++step) {
				const double rv = r.estimate();
				if (rv == 0.0) break;
				const int re = std::ilogb(rv);
				const int E = re - ye;                            // the quotient is about 2^E
				if (E < 0) break;
				const double ratio = std::ldexp(rv, -re) / ym;    // in (1/2, 2)
				const int shift = (E > 25) ? E - 25 : 0;
				const double m = std::trunc(std::ldexp(ratio, E - shift));
				if (m == 0.0) break;
				r.add_multiple(y, -m, shift);
				if (shift == 0 && std::fmod(m, 2.0) != 0.0) odd = !odd;
			}
			// the estimates leave r within a step or two of the range: settle it exactly
			const double ys = (yv < 0.0) ? -1.0 : 1.0;             // y = ys |y|
			for (int k = 0; k < 8; ++k) {
				const int rs = r.sign();
				if (rs != 0 && rs != dividend_sign) {
					// r on the wrong side of zero: add |y| toward the dividend's sign
					r.add(y, ys * dividend_sign);
					odd = !odd;
					continue;
				}
				exact_sum t = r;                                   // |r| - |y|
				t.add(y, -ys * dividend_sign);
				if (rs != 0 && t.sign() * dividend_sign >= 0) {
					r = t;
					odd = !odd;
					continue;
				}
				break;
			}
			return odd;
		}

	}  // namespace detail

	// fmod(x, y) = x - n y, n = trunc(x / y): the sign of x and |result| < |y|, exactly
	template<typename Real>
	Real fmod(const Real& x, const Real& y) {
		if (x.isnan() || y.isnan() || x.isinf() || y.iszero()) return Real(std::numeric_limits<double>::quiet_NaN());
		if (y.isinf() || x.iszero()) return x;
		exact_sum r;
		r.add(x, 1.0);
		detail::reduce(r, y, x[0] < 0.0 ? -1 : 1);
		if (r.sign() == 0) return x[0] < 0.0 ? Real(-0.0) : Real(0.0);
		return r.template to<Real>();
	}

	// remainder(x, y) = x - n y, n the integer nearest x / y, ties to even: |result| <= |y| / 2
	template<typename Real>
	Real remainder(const Real& x, const Real& y) {
		if (x.isnan() || y.isnan() || x.isinf() || y.iszero()) return Real(std::numeric_limits<double>::quiet_NaN());
		if (y.isinf() || x.iszero()) return x;
		const int xs = x[0] < 0.0 ? -1 : 1;
		const double ys = (y[0] < 0.0) ? -1.0 : 1.0;           // y = ys |y|
		exact_sum r;
		r.add(x, 1.0);
		const bool odd = detail::reduce(r, y, xs);
		// now |r| < |y| with the sign of x: step past the midpoint when r lies beyond it, or on
		// it with an odd quotient
		if (r.sign() != 0) {
			exact_sum h = r;                                   // 2 r - xs |y|
			h.add(r, 1.0);
			h.add(y, -xs * ys);
			const int c = h.sign() * xs;                       // sign of 2 |r| - |y|
			if (c > 0 || (c == 0 && odd)) r.add(y, -xs * ys);
		}
		if (r.sign() == 0) return xs < 0 ? Real(-0.0) : Real(0.0);
		return r.template to<Real>();
	}

	// sqrt(x^2 + y^2), scaled by a power of two so that neither square overflows or underflows
	template<typename Real>
	Real hypot(const Real& x, const Real& y) {
		if (x.isinf() || y.isinf()) return Real(std::numeric_limits<double>::infinity());
		if (x.isnan() || y.isnan()) return Real(std::numeric_limits<double>::quiet_NaN());
		if (x.iszero()) return y[0] < 0.0 ? Real(-y) : y;
		if (y.iszero()) return x[0] < 0.0 ? Real(-x) : x;
		const int k = std::max(std::ilogb(x[0]), std::ilogb(y[0]));
		const Real xs = ldexp(x, -k), ys = ldexp(y, -k);
		return ldexp(sqrt(xs * xs + ys * ys), k);
	}

	// log(1 + a) for |a| < 1/4, as 2 atanh(s) with s = a / (2 + a), |s| < 1/7: every term of the
	// series is relative to a, so the limbs of a below 1's last bit are kept
	template<typename Real>
	Real log1p_small(const Real& a) {
		const Real s = a / (Real(2.0) + a);
		const Real s2 = s * s;
		Real term = s, sum = s;
		for (int k = 3; k < 1000; k += 2) {
			term = term * s2;
			const Real t = term / Real(static_cast<double>(k));
			sum += t;
			if (std::fabs(t[0]) < std::ldexp(std::fabs(sum[0]), -(std::numeric_limits<Real>::digits + 6))) break;
		}
		return sum + sum;
	}

}}}  // namespace sw::universal::multidouble
