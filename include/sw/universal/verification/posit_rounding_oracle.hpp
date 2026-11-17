#pragma once
// posit_rounding_oracle.hpp : posit results checked and rounded by the rounding rule itself
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

#include <universal/number/posit/posit_fwd.hpp>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/verification/exact_reference.hpp>

/*
   y is the correctly rounded value of an exact result X != 0 when y has X's sign and
       mid(y-1, y) <= |X| <= mid(y, y+1),
   where the midpoints are the (nbits+1)-bit patterns 2y-1 and 2y+1, and a tie (equality) goes to
   the even encoding. Nothing rounds to zero or NaR: there is no lower midpoint for minpos and no
   upper one for maxpos. An exact zero gives zero.

   The oracle has its own decoder, reading the encoding through bits(), and computes X exactly on
   32-bit words: a + b and a * b directly, a / b by comparing |a| with m * |b|, and sqrt(a) by
   comparing a with m * m. It shares no code with posit arithmetic. It started as the test
   static/tapered/posit/arithmetic/rounding_oracle.cpp (#1921), and is the exact reference of the
   random-operand verifiers (exact_reference.hpp, #2121).
*/

namespace sw {
namespace universal {
namespace rounding_oracle {

// an unsigned integer on 32-bit words, least significant first, no leading zero words
struct Nat {
	std::vector<std::uint32_t> w;
	void trim() { while (!w.empty() && w.back() == 0u) w.pop_back(); }
	bool zero() const { return w.empty(); }
};
inline Nat shl(const Nat& a, unsigned s) {
	if (a.zero()) return a;
	Nat r;
	r.w.assign(s / 32u, 0u);
	const unsigned b = s % 32u;
	std::uint32_t carry = 0;
	for (std::uint32_t x : a.w) {
		r.w.push_back((x << b) | carry);
		carry = (b == 0u) ? 0u : (x >> (32u - b));
	}
	r.w.push_back(carry);
	r.trim();
	return r;
}
inline int cmp(const Nat& a, const Nat& b) {
	if (a.w.size() != b.w.size()) return a.w.size() < b.w.size() ? -1 : 1;
	for (std::size_t i = a.w.size(); i-- > 0;) {
		if (a.w[i] != b.w[i]) return a.w[i] < b.w[i] ? -1 : 1;
	}
	return 0;
}
inline Nat add(const Nat& a, const Nat& b) {
	Nat r;
	std::uint64_t carry = 0;
	for (std::size_t i = 0; i < std::max(a.w.size(), b.w.size()); ++i) {
		const std::uint64_t s = carry + (i < a.w.size() ? a.w[i] : 0u) + (i < b.w.size() ? b.w[i] : 0u);
		r.w.push_back(static_cast<std::uint32_t>(s));
		carry = s >> 32;
	}
	r.w.push_back(static_cast<std::uint32_t>(carry));
	r.trim();
	return r;
}
inline Nat sub(const Nat& a, const Nat& b) {  // a >= b
	Nat r;
	std::int64_t borrow = 0;
	for (std::size_t i = 0; i < a.w.size(); ++i) {
		std::int64_t d = std::int64_t(a.w[i]) - borrow - std::int64_t(i < b.w.size() ? b.w[i] : 0u);
		borrow = (d < 0) ? 1 : 0;
		if (d < 0) d += (std::int64_t(1) << 32);
		r.w.push_back(static_cast<std::uint32_t>(d));
	}
	r.trim();
	return r;
}
inline Nat mul(const Nat& a, const Nat& b) {
	Nat r;
	if (a.zero() || b.zero()) return r;
	r.w.assign(a.w.size() + b.w.size(), 0u);
	for (std::size_t i = 0; i < a.w.size(); ++i) {
		std::uint64_t carry = 0;
		for (std::size_t j = 0; j < b.w.size(); ++j) {
			const std::uint64_t t = std::uint64_t(a.w[i]) * b.w[j] + r.w[i + j] + carry;
			r.w[i + j] = static_cast<std::uint32_t>(t);
			carry = t >> 32;
		}
		r.w[i + b.w.size()] = static_cast<std::uint32_t>(carry);
	}
	r.trim();
	return r;
}

// the dyadic (-1)^neg * m * 2^e
struct Dyadic {
	bool neg{ false };
	Nat  m;
	int  e{ 0 };
};
// |x| compared with |y|
inline int cmpMagnitude(const Dyadic& x, const Dyadic& y) {
	if (x.m.zero() || y.m.zero()) return x.m.zero() ? (y.m.zero() ? 0 : -1) : 1;
	const int e = std::min(x.e, y.e);
	return cmp(shl(x.m, static_cast<unsigned>(x.e - e)), shl(y.m, static_cast<unsigned>(y.e - e)));
}
inline Dyadic addExact(const Dyadic& x, const Dyadic& y) {
	if (x.m.zero()) return y;
	if (y.m.zero()) return x;
	const int e = std::min(x.e, y.e);
	const Nat a = shl(x.m, static_cast<unsigned>(x.e - e)), b = shl(y.m, static_cast<unsigned>(y.e - e));
	Dyadic r;
	r.e = e;
	if (x.neg == y.neg) {
		r.neg = x.neg;
		r.m = add(a, b);
	}
	else if (cmp(a, b) >= 0) {
		r.neg = x.neg;
		r.m = sub(a, b);
	}
	else {
		r.neg = y.neg;
		r.m = sub(b, a);
	}
	return r;
}
inline Dyadic mulExact(const Dyadic& x, const Dyadic& y) {
	Dyadic r;
	r.neg = x.neg != y.neg;
	r.m = mul(x.m, y.m);
	r.e = x.e + y.e;
	return r;
}

// the value of a positive posit pattern of L bits, bits[i] = bit i, bit L-1 (the sign) is 0
inline Dyadic decodePattern(const std::vector<bool>& bits, unsigned es) {
	const int L = static_cast<int>(bits.size());
	int i = L - 2;
	const bool r0 = bits[i];
	int run = 0;
	while (i >= 0 && bits[i] == r0) { ++run; --i; }
	--i;  // the terminator, if any
	const int k = r0 ? run - 1 : -run;
	int exponent = 0;
	for (unsigned j = 0; j < es; ++j) {
		exponent = (exponent << 1) | ((i >= 0 && bits[i]) ? 1 : 0);
		--i;
	}
	Dyadic d;
	Nat f;
	f.w.push_back(1u);  // the hidden bit, then the fraction bits below it
	int nf = 0;
	for (; i >= 0; --i, ++nf) {
		f = shl(f, 1u);
		if (bits[i]) f = add(f, Nat{ { 1u } });
	}
	d.m = f;
	d.e = k * (1 << es) + exponent - nf;
	return d;
}

template<typename Posit>
inline std::vector<bool> patternOf(const Posit& p) {
	std::vector<bool> b(Posit::nbits);
	const auto raw = p.bits();
	for (unsigned i = 0; i < Posit::nbits; ++i) b[i] = raw.test(i);
	return b;
}
// two's complement of a pattern, in place
inline void negatePattern(std::vector<bool>& b) {
	bool carry = true;
	for (std::size_t i = 0; i < b.size(); ++i) {
		const bool x = !b[i];
		b[i] = x != carry;
		carry = x && carry;
	}
}
// the magnitude pattern of a nonzero, non-NaR posit, and its sign
template<typename Posit>
inline std::vector<bool> magnitudePattern(const Posit& p, bool& negative) {
	std::vector<bool> b = patternOf(p);
	negative = b.back();
	if (negative) negatePattern(b);
	return b;
}
template<typename Posit>
inline Dyadic valueOf(const Posit& p) {
	if (p.iszero()) return Dyadic{};
	bool negative = false;
	Dyadic d = decodePattern(magnitudePattern(p, negative), Posit::es);
	d.neg = negative;
	return d;
}
inline Dyadic valueOf(double x) {
	Dyadic d;
	if (x == 0.0) return d;
	d.neg = x < 0;
	int e = 0;
	const double m = std::frexp(std::fabs(x), &e);
	const std::uint64_t M = static_cast<std::uint64_t>(std::ldexp(m, 53));
	d.m.w = { static_cast<std::uint32_t>(M), static_cast<std::uint32_t>(M >> 32) };
	d.m.trim();
	d.e = e - 53;
	return d;
}

// the midpoints of a positive pattern y: lower = 2y-1 and upper = 2y+1, at nbits+1 bits
inline std::vector<bool> upperMid(const std::vector<bool>& y) {
	std::vector<bool> m(y.size() + 1);
	m[0] = true;
	for (std::size_t i = 0; i < y.size(); ++i) m[i + 1] = y[i];
	return m;
}
inline std::vector<bool> lowerMid(std::vector<bool> y) {
	for (std::size_t i = 0; i < y.size(); ++i) {  // y - 1
		if (y[i]) { y[i] = false; break; }
		y[i] = true;
	}
	return upperMid(y);
}

// is y the correctly rounded value of the exact result? cmpTo(m) returns the sign of |X| - m
template<typename Posit, typename Compare>
bool correctlyRounded(const Posit& y, bool exactZero, bool exactNegative, Compare cmpTo) {
	if (exactZero) return y.iszero();
	if (y.iszero() || y.isnar()) return false;
	bool negative = false;
	const std::vector<bool> Y = magnitudePattern(y, negative);
	if (negative != exactNegative) return false;
	bool isMinpos = Y[0], isMaxpos = true;   // minpos = 0.00..01, maxpos = 0.11..11
	for (unsigned i = 1; i < Posit::nbits; ++i) isMinpos = isMinpos && !Y[i];
	for (unsigned i = 0; i + 1 < Posit::nbits; ++i) isMaxpos = isMaxpos && Y[i];
	const bool even = !Y[0];
	if (!isMinpos) {
		const int c = cmpTo(decodePattern(lowerMid(Y), Posit::es));
		if (c < 0 || (c == 0 && !even)) return false;
	}
	if (!isMaxpos) {
		const int c = cmpTo(decodePattern(upperMid(Y), Posit::es));
		if (c > 0 || (c == 0 && !even)) return false;
	}
	return true;
}

// the correctly rounded posit of an exact result X; cmpTo(m) returns the sign of |X| - m.
// The magnitude pattern y is built from the top bit down, keeping each bit for which
// value(y) <= |X|: that is the largest y at or below |X|, or zero below minpos. The result is
// then y, or y + 1 when |X| lies above the midpoint between them, or on it with y odd.
template<typename Posit, typename Compare>
void roundExact(bool exactZero, bool exactNegative, Compare cmpTo, Posit& ref) {
	constexpr unsigned nbits = Posit::nbits;
	blockbinary<nbits, typename Posit::BlockType, BinaryNumberType::Signed> raw;
	raw.clear();
	if (!exactZero) {
		std::vector<bool> Y(nbits, false);
		for (unsigned i = nbits - 1; i-- > 0;) {   // bit nbits-1 is the sign
			Y[i] = true;
			if (cmpTo(decodePattern(Y, Posit::es)) < 0) Y[i] = false;
		}
		bool isZero = true, isMaxpos = true;
		for (unsigned i = 0; i + 1 < nbits; ++i) {
			isZero = isZero && !Y[i];
			isMaxpos = isMaxpos && Y[i];
		}
		if (isZero) {
			Y[0] = true;   // below minpos: minpos, as nothing rounds to zero
		}
		else if (!isMaxpos) {
			const int c = cmpTo(decodePattern(upperMid(Y), Posit::es));
			if (c > 0 || (c == 0 && Y[0])) {   // y + 1, which stays below the sign bit
				for (unsigned i = 0; i < nbits; ++i) {
					Y[i] = !Y[i];
					if (Y[i]) break;
				}
			}
		}
		if (exactNegative) negatePattern(Y);
		for (unsigned i = 0; i < nbits; ++i) raw.setbit(i, Y[i]);
	}
	ref.setbits(raw);
}

}  // namespace rounding_oracle

// the exact reference of a posit (exact_reference.hpp): NaR for a NaR operand, a division by
// zero and the square root of a negative value, as posit arithmetic gives
template<unsigned nbits, unsigned es, typename bt>
struct exact_rounding<posit<nbits, es, bt>> {
	using Posit = posit<nbits, es, bt>;

	static bool binary(char op, const Posit& a, const Posit& b, Posit& ref) {
		using namespace rounding_oracle;
		if (op != '+' && op != '-' && op != '*' && op != '/') return false;
		if (a.isnar() || b.isnar() || (op == '/' && b.iszero())) {
			ref.setnar();
			return true;
		}
		const Dyadic x = valueOf(a), y = valueOf(b);
		if (op == '/') {
			// |a / b| against m is |a| against m * |b|
			roundExact(x.m.zero(), x.neg != y.neg, [&](const Dyadic& m) { return cmpMagnitude(x, mulExact(m, y)); }, ref);
			return true;
		}
		Dyadic s;
		if (op == '*') {
			s = mulExact(x, y);
		}
		else {
			Dyadic t = y;
			if (op == '-') t.neg = !t.neg;
			s = addExact(x, t);
		}
		roundExact(s.m.zero(), s.neg, [&](const Dyadic& m) { return cmpMagnitude(s, m); }, ref);
		return true;
	}

	static bool sqrt(const Posit& a, Posit& ref) {
		using namespace rounding_oracle;
		if (a.isnar() || a.isneg()) {
			ref.setnar();
			return true;
		}
		const Dyadic x = valueOf(a);
		// sqrt(a) against m is a against m * m
		roundExact(x.m.zero(), false, [&](const Dyadic& m) { return cmpMagnitude(x, mulExact(m, m)); }, ref);
		return true;
	}
};

}  // namespace universal
}  // namespace sw
