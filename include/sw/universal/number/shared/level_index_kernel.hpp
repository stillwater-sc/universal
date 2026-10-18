#pragma once
// level_index_kernel.hpp: the level-index map and its psi-domain addition and subtraction
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The level-index (LI) representation of a value X >= 0 is its generalized logarithm
// psi(X) = l + f: the integer level l and the index f in [0, 1).  The value map is
//
//     phi(x) = x               for 0 <= x < 1,       psi(X) = X                for 0 <= X < 1
//     phi(x) = e^phi(x - 1)    for x >= 1,           psi(X) = 1 + psi(ln X)    for X >= 1
//
// This kernel is shared by lindex (#1349) and slindex (#1350).  Its arithmetic never forms
// a value: phi(5) already has 1.6 million decimal digits in its exponent, past every
// floating-point format, so + and - run the Clenshaw-Olver sequences (1987) in the psi
// domain, where every quantity lies in [0, 2].  See docs/level-index-design.md.
//
// W is the working type.  It needs + - * /, comparison, construction from int and double,
// and exp, log and expm1 found by argument-dependent lookup (std:: for the native types):
// double for nbits <= 32, dd_cascade above that.  The kernel includes no I/O.
//
// A psi value is handled as a point { level, index } rather than as one W: the index is
// then held exactly by any W with as many significand bits as the format has index bits,
// whatever the level.  A result may land one level above the top level of a particular
// format; the format decides whether that saturates.
#include <cmath>
#include <cstddef>

namespace sw { namespace universal { namespace level_index {

// the deepest level the kernel works with (lbits <= 4)
constexpr int max_level = 15;

// psi = level + index, with index in [0, 1)
template<typename W>
struct point {
	int level;
	W index;
};

// X = phi(level + index).  Past the range of W this is W's overflow, +inf for double.
template<typename W>
W phi(const point<W>& x) {
	using std::exp;
	W v = x.index;
	for (int k = 0; k < x.level; ++k) v = exp(v);
	return v;
}

// psi(X) for a finite X >= 0
template<typename W>
point<W> psi(W X) {
	using std::log;
	int level = 0;
	while (X >= W(1) && level < max_level) {
		X = log(X);
		++level;
	}
	return { level, X };
}

// The a sequence of x = l + f, a_j = 1 / phi(x - j) for j = 0 .. l-1:
//     a_(l-1) = e^-f,    a_(j-1) = e^(-1 / a_j)
// written to a[0 .. l-1].  Deep in the sequence 1 / a_j outgrows W and a_(j-1) underflows
// to 0, which is the correct limit: 1 / phi(x - j + 1) is far below any W.
template<typename W>
void a_sequence(int l, const W& f, W* a) {
	using std::exp;
	if (l <= 0) return;
	a[l - 1] = exp(-f);
	for (int j = l - 1; j > 0; --j) a[j - 1] = (a[j] > W(0)) ? W(exp(-(W(1) / a[j]))) : W(0);
}

// The b sequence of y = m + g against the a sequence of x >= y, returning
// b_0 = phi(y) / phi(x):
//     b_(m-1) = a_(m-1) e^g,    b_(j-1) = e^(-(1 - b_j) / a_j)    (m = 0: b_0 = a_0 g)
template<typename W>
W b_zero(int m, const W& g, const W* a) {
	using std::exp;
	if (m == 0) return a[0] * g;
	W b = a[m - 1] * exp(g);
	for (int j = m - 1; j > 0; --j) b = (a[j] > W(0)) ? W(exp(-((W(1) - b) / a[j]))) : W(0);
	return b;
}

// d_0 = 1 - b_0, carried as d_j = 1 - b_j without forming b_j, so that it does not cancel
// when phi(y) is close to phi(x):  d_(j-1) = -expm1(-d_j / a_j).  It starts without rounding:
//     m = l:      d_(l-1) = 1 - e^(g - f) = -expm1(g - f),  and g - f is exact
//     m = l - 1:  d_(l-2) = 1 - e^(g - e^f) = -expm1((g - 1) - expm1(f))
// Requires 1 <= m and l - 1 <= m <= l.
template<typename W>
W d_zero(int l, const W& f, int m, const W& g, const W* a) {
	using std::expm1;
	W d = (m == l) ? W(-expm1(g - f)) : W(-expm1((g - W(1)) - expm1(f)));
	for (int j = m - 1; j > 0; --j) d = (a[j] > W(0)) ? W(-expm1(-(d / a[j]))) : W(1);
	return d;
}

namespace detail {

// the point whose level-n part phi(z - n) is w >= 0
template<typename W>
point<W> relevel(W w, int n) {
	using std::log;
	while (w >= W(1) && n <= max_level) {
		w = log(w);
		++n;
	}
	return { n, w };
}

// Run the c sequence from c_0, c_j = phi(z - j) / phi(x - j):
//     c_j = 1 + a_j ln c_(j-1)
// It stops at the first j with c_j < a_j, where the result is at level j with index
// c_j / a_j; otherwise phi(z - l) = f + ln c_(l-1), which can move up a level.
template<typename W>
point<W> c_sequence(int l, const W& f, const W* a, W c) {
	using std::log;
	for (int j = 0; j < l; ++j) {
		if (j > 0) {
			// a_j = 0 is the limit of an underflowed a_j: a_j ln c is then below any W
			if (a[j] > W(0)) c = W(1) + a[j] * log(c);
			else c = W(1);
		}
		if (c < a[j]) return { j, c / a[j] };
	}
	return relevel(W(f + log(c)), l);
}

}  // namespace detail

// psi(phi(x) + phi(y)) for x >= y >= 0
template<typename W>
point<W> add(const point<W>& x, const point<W>& y) {
	if (x.level == 0) return detail::relevel(W(x.index + y.index), 0);   // fixed point, exact
	W a[max_level + 1];
	a_sequence(x.level, x.index, a);
	const W b0 = b_zero(y.level, y.index, a);
	return detail::c_sequence(x.level, x.index, a, W(W(1) + b0));
}

// psi(phi(x) - phi(y)) for x >= y >= 0.  x - x is exactly 0: it is decided before the
// sequences, because at levels 6 and 7 every a_j underflows to 0 and the stop test
// c_j < a_j would never fire.
template<typename W>
point<W> sub(const point<W>& x, const point<W>& y) {
	if (x.level == y.level && x.index == y.index) return { 0, W(0) };
	if (x.level == 0) return { 0, W(x.index - y.index) };                // fixed point, exact
	W a[max_level + 1];
	a_sequence(x.level, x.index, a);
	// The operands are close in value only when y shares x's level or sits in the level
	// just below it; there 1 - b_0 cancels, so it is carried as d.  A y two or more levels
	// down has b_0 = phi(y) / phi(x) < 1 / e, and level 1 minus level 0 lands in level 0,
	// where the error is absolute.
	const bool close = (y.level >= 1) && (y.level >= x.level - 1);
	const W c0 = close ? d_zero(x.level, x.index, y.level, y.index, a)
	                   : W(W(1) - b_zero(y.level, y.index, a));
	return detail::c_sequence(x.level, x.index, a, c0);
}

}}}  // namespace sw::universal::level_index
