#pragma once
// rational_exact.hpp: exact arithmetic and comparison for the fixed-width rationals, saturating (#2132)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Each operation computes its exact result P / Q in a wide signed blockbinary, which holds every
// product and sum of two operands, reduces it by the gcd, and fits it back into the type:
//   - a result past the type's range saturates at maxpos or maxneg (USER decision on #2132; it used
//     to wrap silently: rational<8> 100 + 100 was -56);
//   - a result inside the range whose reduced numerator or denominator does not fit is the closest
//     rational the type holds (best_rational.hpp), as a conversion from a native float is;
//   - a zero denominator is NaN, in every base (the digit bases used to keep n/0), or throws
//     rational_divide_by_zero when RATIONAL_THROW_ARITHMETIC_EXCEPTION is set.
// Comparisons cross-multiply in the same wide type, exactly; they used to go through double, which
// cannot tell 2^53 + 1 from 2^53.
//
// A rational type R plugs in with:
//   using Wide;                                     the wide signed blockbinary
//   Wide exact_numerator() const, exact_denominator() const
//   static Wide exact_bound(bool negative, bool numerator)   the largest magnitude best_rational may use
//   static Wide exact_limit(bool negative)          the largest integer magnitude in range: maxpos or -maxneg
//   bool exact_store(const Wide& p, const Wide& q, bool negative)   store |p| / |q| with the sign, if it fits
//   void setzero(), setnan(), maxpos(), maxneg()
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/number/rational/best_rational.hpp>
#include <universal/number/rational/exceptions.hpp>

namespace sw { namespace universal { namespace rational_exact {

template<typename Wide>
Wide gcd(Wide a, Wide b) {   // a, b >= 0, not both zero
	while (!b.iszero()) {
		Wide r = a % b;
		a = b;
		b = r;
	}
	return a;
}

template<typename Wide>
Wide magnitude(const Wide& v) {
	Wide m = v;
	if (m.sign()) m.twosComplement();
	return m;
}

// a digit component (blockdigit, sign and magnitude) as a signed Wide
template<typename Wide, typename Component>
Wide digits_to_wide(const Component& c) {
	Wide v, radix, digit;
	v.clear();
	radix.clear();
	radix.setbits(Component::radix);
	for (unsigned i = Component::ndigits; i-- > 0;) {
		digit.clear();
		digit.setbits(static_cast<unsigned long long>(c.digit(i)));
		v = v * radix + digit;
	}
	if (c.isneg()) v.twosComplement();
	return v;
}

// radix^ndigits - 1, the largest digit component
template<typename Wide, typename Component>
Wide digit_bound() {
	Wide bound, radix, one;
	bound.clear();
	radix.clear();
	one.clear();
	bound.setbit(0);
	one.setbit(0);
	radix.setbits(Component::radix);
	for (unsigned i = 0; i < Component::ndigits; ++i) bound = bound * radix;
	return bound - one;
}

// n / d = -+p / q with a positive denominator, if both fit
template<typename Wide, typename Component>
bool store_digits(const Wide& p, const Wide& q, bool negative, Component& n, Component& d) {
	const Wide bound = digit_bound<Wide, Component>();
	if (p > bound || q > bound) return false;
	n = to_digit_component<Component>(p);
	if (negative) n = -n;
	d = to_digit_component<Component>(q);
	return true;
}

// r = P / Q, exactly where it fits, saturating past the range, the closest rational otherwise
template<typename R>
void assign(R& r, const typename R::Wide& P, const typename R::Wide& Q, bool divisionByZeroThrows) {
	using Wide = typename R::Wide;
	if (Q.iszero()) {
#if RATIONAL_THROW_ARITHMETIC_EXCEPTION
		if (divisionByZeroThrows) throw rational_divide_by_zero();
#else
		(void)divisionByZeroThrows;
#endif
		r.setnan();
		return;
	}
	if (P.iszero()) { r.setzero(); return; }
	const bool negative = (P.sign() != Q.sign());
	Wide p = magnitude(P), q = magnitude(Q);
	const Wide g = rational_exact::gcd(p, q);   // qualified: ADL also finds the integer gcd
	p /= g;
	q /= g;
	// past the range: |P / Q| > limit is p > limit * q
	const Wide limit = R::exact_limit(negative);
	if (p > limit * q) {
		if (negative) r.maxneg(); else r.maxpos();
		return;
	}
	if (r.exact_store(p, q, negative)) return;
	// inside the range but too wide: the closest rational the type holds
	Wide bp, bq;
	best_rational(p, q, R::exact_bound(negative, true), R::exact_bound(negative, false), bp, bq);
	if (bp.iszero()) { r.setzero(); return; }
	r.exact_store(bp, bq, negative);
}

// lhs = lhs op rhs, for op one of '+', '-', '*', '/'
template<typename R>
void operate(R& lhs, const R& rhs, char op) {
	using Wide = typename R::Wide;
	if (lhs.isnan() || rhs.isnan()) { lhs.setnan(); return; }
	const Wide a = lhs.exact_numerator(), b = lhs.exact_denominator();
	const Wide c = rhs.exact_numerator(), d = rhs.exact_denominator();
	switch (op) {
	case '+': assign(lhs, Wide(a * d + c * b), Wide(b * d), false); return;
	case '-': assign(lhs, Wide(a * d - c * b), Wide(b * d), false); return;
	case '*': assign(lhs, Wide(a * c), Wide(b * d), false); return;
	default:  assign(lhs, Wide(a * d), Wide(b * c), true); return;   // a zero divisor is a division by zero
	}
}

// -1, 0, 1 for lhs <, ==, > rhs; neither may be NaN
template<typename R>
int compare(const R& lhs, const R& rhs) {
	using Wide = typename R::Wide;
	const Wide a = lhs.exact_numerator(), b = lhs.exact_denominator();
	const Wide c = rhs.exact_numerator(), d = rhs.exact_denominator();
	// a/b against c/d is a*d against c*b when b*d > 0, reversed when b*d < 0
	const Wide left = a * d, right = c * b;
	int cmp = (left < right) ? -1 : ((right < left) ? 1 : 0);
	if (b.sign() != d.sign()) cmp = -cmp;
	return cmp;
}

}}}  // namespace sw::universal::rational_exact
