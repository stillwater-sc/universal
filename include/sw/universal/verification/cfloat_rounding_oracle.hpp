#pragma once
// cfloat_rounding_oracle.hpp : cfloat results rounded by the rounding rule itself (#2123)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstddef>
#include <cstdint>
#include <vector>

#include <universal/number/cfloat/cfloat_fwd.hpp>
#include <universal/number/shared/nan_encoding.hpp>
#include <universal/number/shared/specific_value_encoding.hpp>
#include <universal/verification/exact_reference.hpp>
#include <universal/verification/rounding_oracle_core.hpp>

/*
   The exact reference of a cfloat (exact_reference.hpp). The operands are read bit by bit and
   decoded into exact dyadics by this file's own decoder; the exact result X of + - * is a
   dyadic, and a quotient or square root is compared, never computed: |a/b| against m is |a|
   against m*|b|, and sqrt(a) against m is a against m*m. X is rounded to nearest, ties to even:

   - y is the largest finite magnitude encoding at or below |X|, found a bit at a time from the
     top and never past maxpos. The result is y, or y + 1 when |X| lies above the midpoint
     between them, or on it with y odd. The midpoint is the (nbits+1)-bit pattern 2y+1: one
     more fraction bit, which is the arithmetic midpoint also across a binade and below the
     smallest normal.
   - Past maxpos (|X| above maxpos's upper midpoint, or on it, maxpos being odd) the result
     overflows: inf, or maxpos for a saturating cfloat.
   - Without subnormals every encoding with a zero exponent field is zero, and |X| below the
     smallest normal flushes to zero before rounding, as cfloat's conversion and multiplication
     do: a product just below the smallest normal is zero, not the smallest normal.
   - A zero takes X's sign; an exact zero sum is +0, or -0 when both addends are -0.
   - IEEE-754 special values: a NaN operand, inf - inf, 0 * inf, 0/0, inf/inf and the square
     root of a negative value give NaN; inf op x and x/0 give inf; x/inf gives zero. A
     saturating cfloat clamps only a finite overflow: inf from an inf operand or a division by
     zero stays inf where the type has an inf encoding.

   Which encodings are inf and NaN is the type's own definition (isinf, isnan, setinf, setnan,
   maxpos): that is the encoding, not arithmetic, and it depends on hasMaxExpValues and
   isSaturating (#1780, #1896).
*/

namespace sw {
namespace universal {
namespace cfloat_oracle {

using rounding_oracle::Nat;
using rounding_oracle::Dyadic;
using rounding_oracle::cmpMagnitude;
using rounding_oracle::addExact;
using rounding_oracle::mulExact;
using rounding_oracle::upperMid;

// the value of a magnitude pattern of L bits (bit L-1, the sign, is 0) with es exponent bits
inline Dyadic decodeMagnitude(const std::vector<bool>& bits, unsigned es, bool hasSubnormals) {
	const int L = static_cast<int>(bits.size());
	const int fb = L - 1 - static_cast<int>(es);
	unsigned E = 0;
	for (int i = L - 2; i >= fb; --i) E = (E << 1) | (bits[static_cast<std::size_t>(i)] ? 1u : 0u);
	Nat f;
	for (int i = fb - 1; i >= 0; --i) {
		f = rounding_oracle::shl(f, 1u);
		if (bits[static_cast<std::size_t>(i)]) f = rounding_oracle::add(f, Nat{ { 1u } });
	}
	const int bias = (1 << (es - 1u)) - 1;
	Dyadic d;
	if (E == 0) {
		if (!hasSubnormals) return d;   // every zero-exponent encoding is zero
		d.m = f;
		d.e = 1 - bias - fb;
	}
	else {
		Nat hidden{ { 1u } };
		d.m = rounding_oracle::add(rounding_oracle::shl(hidden, static_cast<unsigned>(fb)), f);
		d.e = static_cast<int>(E) - bias - fb;
	}
	d.m.trim();
	return d;
}

template<typename Cfloat>
std::vector<bool> magnitudeOf(const Cfloat& c) {
	std::vector<bool> b(Cfloat::nbits);
	for (unsigned i = 0; i + 1 < Cfloat::nbits; ++i) b[i] = c.at(i);
	return b;
}

template<typename Cfloat>
Dyadic valueOf(const Cfloat& c) {   // a finite cfloat
	Dyadic d = decodeMagnitude(magnitudeOf(c), Cfloat::es, Cfloat::hasSubnormals);
	d.neg = c.sign();
	return d;
}

// a <= b for magnitude patterns of equal length, compared from the top
inline bool patternAtMost(const std::vector<bool>& a, const std::vector<bool>& b) {
	for (std::size_t i = a.size(); i-- > 0;) {
		if (a[i] != b[i]) return b[i];
	}
	return true;
}

template<typename Cfloat>
void setMagnitude(Cfloat& ref, const std::vector<bool>& Y, bool negative) {
	ref.setzero();
	for (unsigned i = 0; i + 1 < Cfloat::nbits; ++i) ref.setbit(i, Y[i]);
	ref.setbit(Cfloat::nbits - 1u, negative);
}

template<typename Cfloat>
void setOverflow(Cfloat& ref, bool negative) {
	if constexpr (Cfloat::isSaturating) {
		ref = Cfloat(negative ? SpecificValue::maxneg : SpecificValue::maxpos);
	}
	else {
		ref.setinf(negative);
	}
}

// the correctly rounded cfloat of an exact nonzero result X; cmpTo(m) returns the sign of |X| - m
template<typename Cfloat, typename Compare>
void roundExact(bool exactNegative, Compare cmpTo, Cfloat& ref) {
	constexpr unsigned nbits = Cfloat::nbits;
	constexpr unsigned es = Cfloat::es;
	constexpr unsigned fbits = Cfloat::fbits;
	constexpr bool sub = Cfloat::hasSubnormals;
	const std::vector<bool> maxposPattern = magnitudeOf(Cfloat(SpecificValue::maxpos));

	std::vector<bool> Y(nbits, false);
	for (unsigned i = nbits - 1; i-- > 0;) {   // bit nbits-1 is the sign
		Y[i] = true;
		if (!patternAtMost(Y, maxposPattern) || cmpTo(decodeMagnitude(Y, es, sub)) < 0) Y[i] = false;
	}

	bool normal = false;   // a nonzero exponent field
	for (unsigned i = fbits; i + 1 < nbits; ++i) normal = normal || Y[i];
	if (!sub && !normal) {   // below the smallest normal: flushed to zero
		setMagnitude(ref, std::vector<bool>(nbits, false), exactNegative);
		return;
	}

	const int c = cmpTo(decodeMagnitude(upperMid(Y), es, sub));
	const bool up = c > 0 || (c == 0 && Y[0]);
	if (up) {
		if (Y == maxposPattern) {
			setOverflow(ref, exactNegative);
			return;
		}
		for (unsigned i = 0; i < nbits; ++i) {   // y + 1, which stays at or below maxpos
			Y[i] = !Y[i];
			if (Y[i]) break;
		}
	}
	setMagnitude(ref, Y, exactNegative);
}

// inf from a special case (an inf operand, x/0): inf also for a saturating type
template<typename Cfloat>
void setInf(Cfloat& ref, bool negative) { ref.setinf(negative); }

template<typename Cfloat>
void setNaN(Cfloat& ref) { ref.setnan(NAN_TYPE_QUIET); }

template<typename Cfloat>
void setZero(Cfloat& ref, bool negative) {
	ref.setzero();
	ref.setbit(Cfloat::nbits - 1u, negative);
}

}  // namespace cfloat_oracle

// the exact reference of a cfloat (exact_reference.hpp)
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
struct exact_rounding<cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>> {
	using Cfloat = cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>;

	static bool binary(char op, const Cfloat& a, const Cfloat& b, Cfloat& ref) {
		using namespace cfloat_oracle;
		if (op != '+' && op != '-' && op != '*' && op != '/') return false;
		if (a.isnan() || b.isnan()) { setNaN(ref); return true; }
		const bool na = a.sign();
		const bool nb = (op == '-') ? !b.sign() : b.sign();
		const bool ia = a.isinf(), ib = b.isinf();
		const bool za = a.iszero(), zb = b.iszero();
		switch (op) {
		case '+':
		case '-':
			if (ia && ib) {
				if (na != nb) setNaN(ref);
				else setInf(ref, na);
				return true;
			}
			if (ia) { setInf(ref, na); return true; }
			if (ib) { setInf(ref, nb); return true; }
			if (za && zb) { setZero(ref, na && nb); return true; }
			break;
		case '*':
			if ((ia && zb) || (za && ib)) { setNaN(ref); return true; }
			if (ia || ib) { setInf(ref, na != nb); return true; }
			if (za || zb) { setZero(ref, na != nb); return true; }
			break;
		default:  // '/'
			if ((za && zb) || (ia && ib)) { setNaN(ref); return true; }
			if (ia || zb) { setInf(ref, na != nb); return true; }
			if (za || ib) { setZero(ref, na != nb); return true; }
			break;
		}
		const Dyadic x = valueOf(a);
		Dyadic y = valueOf(b);
		y.neg = nb;
		if (op == '/') {
			roundExact(x.neg != y.neg, [&](const Dyadic& m) { return cmpMagnitude(x, mulExact(m, y)); }, ref);
			return true;
		}
		const Dyadic s = (op == '*') ? mulExact(x, y) : addExact(x, y);
		if (s.m.zero()) { setZero(ref, false); return true; }   // x - x is +0
		roundExact(s.neg, [&](const Dyadic& m) { return cmpMagnitude(s, m); }, ref);
		return true;
	}

	static bool sqrt(const Cfloat& a, Cfloat& ref) {
		using namespace cfloat_oracle;
		if (a.isnan()) { setNaN(ref); return true; }
		if (a.iszero()) { setZero(ref, a.sign()); return true; }
		if (a.sign()) { setNaN(ref); return true; }
		if (a.isinf()) { setInf(ref, false); return true; }
		const Dyadic x = valueOf(a);
		roundExact(false, [&](const Dyadic& m) { return cmpMagnitude(x, mulExact(m, m)); }, ref);
		return true;
	}
};

}  // namespace universal
}  // namespace sw
