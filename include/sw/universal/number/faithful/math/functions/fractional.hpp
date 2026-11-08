#pragma once
// fractional.hpp: fmod for faithful
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// fmod(x, y) = x - n y with n = trunc(x / y), exact (#1954). Rounding x / y to the
// FloatingPointType would take the remainder of a different quotient. Instead the remainder is
// reduced in an exact expansion of FloatingPointType components, Shewchuk's GROW-EXPANSION:
//   1. r = |x|, held exactly;
//   2. while r >= |y|: take q, an integer just below r / |y|, so the step never overshoots and r
//      stays nonnegative; subtract q |y| exactly, as four twoProd terms; q is at least 1;
//   3. round r once to a pair and give it x's sign.
// Each step removes about digits(Real) - 5 bits of the quotient, so a quotient of 2^1000 takes
// about 20 steps for double and 50 for float.
//
// fmod(x, 0) and fmod(+-inf, y) are NaN with EDOM, fmod(x, +-inf) is x, and a NaN operand gives
// NaN, as std::fmod. The exactness holds while no product q * part falls below the smallest
// subnormal, which takes a divisor within digits(Real) bits of the bottom of the range.
//
// Only a native FloatingPointType: the quotient chunk is taken with std::trunc.
#include <cerrno>        // errno, EDOM
#include <algorithm>     // std::copy
#include <cmath>         // std::trunc, std::frexp, std::ldexp
#include <limits>        // std::numeric_limits
#include <type_traits>   // std::is_floating_point_v
#include <vector>
#include <universal/number/faithful/core.hpp>
#include <universal/number/faithful/math/functions/truncate.hpp>   // faithful_detail::canonical
#include <universal/numerics/twosum.hpp>
#include <universal/numerics/twoprod.hpp>

namespace sw { namespace universal {

namespace faithful_detail {
	// An exact sum of FloatingPointType components, nonoverlapping and ordered from the smallest
	// magnitude up, with zeros eliminated, so the last component carries the sign of the whole.
	// The components live in an inline buffer: a std::vector cost a heap allocation per
	// expansion, and fmod copies one on every step of its loop and exp, exp10 and pow build
	// several per call (#1969). The largest expansion the library's functions form is 12
	// components for float and 11 for double, so 32 inline components cover them; a larger one,
	// which a long chain of add() calls can still make, moves to the heap, and stays correct.
	template<typename Real>
	class exact_expansion {
	public:
		exact_expansion() noexcept : n(0) {}
		exact_expansion(const exact_expansion& o) : n(0) { copy_from(o); }
		exact_expansion& operator=(const exact_expansion& o) {
			if (this != &o) copy_from(o);
			return *this;
		}
		void add(Real b) {
			if (b == Real(0)) return;
			Real* c = data();
			Real q = b;
			std::size_t j = 0;
			for (std::size_t i = 0; i < n; ++i) {
				Real a(q), e(c[i]), s, r;
				twoSum(a, e, s, r);
				q = s;
				if (r != Real(0)) c[j++] = r;
			}
			n = j;
			if (q != Real(0)) push(q);
		}
		// -1, 0 or +1
		int sign() const { return n == 0 ? 0 : (data()[n - 1] < Real(0) ? -1 : 1); }
		// the number of components
		std::size_t size() const noexcept { return n; }
		// the sum rounded, accumulated from the smallest component up
		Real approx() const {
			const Real* c = data();
			Real s(0);
			for (std::size_t i = 0; i < n; ++i) s += c[i];
			return s;
		}
		// the expansion rounded to a pair: the leading part the rounded sum, the trailing part the
		// rounded sum of what remains
		faithful<Real> to_pair() const {
			const Real hi = approx();
			exact_expansion rest(*this);
			rest.add(-hi);
			Real h(hi), l(rest.approx()), s, r;
			twoSum(h, l, s, r);
			return faithful<Real>(s, r);
		}
	private:
		static constexpr std::size_t inline_capacity = 32;
		Real buf[inline_capacity];   // components [0, n) while heap is empty; the rest is never read
		std::vector<Real> heap;      // all components, once there have been more than inline_capacity
		std::size_t n;

		Real* data() noexcept { return heap.empty() ? buf : heap.data(); }
		// Only the live components [0, n). A spilled expansion's heap can hold more than n
		// entries, since add() shrinks n without shrinking the vector, and copying the whole
		// vector copied those stale entries too (#1991). One that fits goes back inline.
		void copy_from(const exact_expansion& o) {
			const Real* src = o.data();
			if (o.n <= inline_capacity) {
				std::copy(src, src + o.n, buf);
				heap.clear();
			}
			else {
				heap.assign(src, src + o.n);
			}
			n = o.n;
		}
		const Real* data() const noexcept { return heap.empty() ? buf : heap.data(); }
		void push(Real q) {
			if (heap.empty()) {
				if (n < inline_capacity) { buf[n++] = q; return; }
				heap.assign(buf, buf + n);   // spill: from here on the components live on the heap
			}
			heap.resize(n);
			heap.push_back(q);
			++n;
		}
	};
}

template<typename FloatingPointType>
	requires std::is_floating_point_v<FloatingPointType>
inline faithful<FloatingPointType> fmod(const faithful<FloatingPointType>& x, const faithful<FloatingPointType>& y) {
	using Real = FloatingPointType;
	using F = faithful<Real>;
	constexpr int digits = std::numeric_limits<Real>::digits;
	if (x.isnan() || y.isnan()) return F(std::numeric_limits<Real>::quiet_NaN(), Real(0));
	if (x.isinf() || y.iszero()) {
		errno = EDOM;
		return F(std::numeric_limits<Real>::quiet_NaN(), Real(0));
	}
	if (y.isinf() || x.iszero()) return x;

	const F ay = faithful_detail::canonical(y.sign() ? -y : y);
	const F ax = x.sign() ? -x : x;
	faithful_detail::exact_expansion<Real> r;
	r.add(ax.low());
	r.add(ax.high());

	// Each step subtracts q * (|y| * 2^k), an integer multiple of |y|, with q an integer just below
	// r / (|y| 2^k). q is biased low by 2^-(digits - 5), well beyond the few roundings in r.approx()
	// and in the division, so the step never overshoots and r stays nonnegative. q and k come from
	// the mantissas and exponents: the quotient r / |y| itself can be beyond Real's range, as
	// 2^230 is for float in fmod(max / 4, 2^-105). Above 2^digits, q keeps digits bits and the
	// divisor is scaled up by 2^k instead, which is exact, even from a subnormal, and keeps every
	// product at most r.
	const Real bias = Real(1) - std::ldexp(Real(1), -(digits - 5));
	for (;;) {
		faithful_detail::exact_expansion<Real> d(r);
		d.add(-ay.low());
		d.add(-ay.high());
		if (d.sign() < 0) break;                     // r < |y|: r is the remainder
		int er = 0, ey = 0;
		const Real mr = std::frexp(r.approx(), &er), my = std::frexp(ay.high(), &ey);
		const Real m = (mr / my) * bias;              // in (0.25, 2)
		const int shift = er - ey;
		const int k = (shift > digits) ? shift - digits : 0;
		Real q = std::trunc(std::ldexp(m, shift - k));
		if (q < Real(1)) q = Real(1);                 // r >= |y|, so one |y| always fits
		const Real yh = std::ldexp(ay.high(), k), yl = std::ldexp(ay.low(), k);
		Real p1, e1, p2, e2;
		twoProd(q, yh, p1, e1);
		twoProd(q, yl, p2, e2);
		r.add(-e2);
		r.add(-p2);
		r.add(-e1);
		r.add(-p1);
	}

	const F m = r.to_pair();
	if (m.iszero()) return F(x.sign() ? -Real(0) : Real(0), Real(0));   // the zero keeps x's sign
	return x.sign() ? -m : m;
}

}} // namespace sw::universal
