#pragma once
// best_rational.hpp: the closest rational p / q to a native floating-point value, p and q bounded
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>
#include <cstdint>
#include <type_traits>
#include <universal/native/ieee754_core.hpp>
#include <universal/native/long_double_significand.hpp>
#include <universal/internal/blockbinary/blockbinary.hpp>

namespace sw { namespace universal {

	// what a native value maps to in a bounded rational
	enum class rational_conversion {
		finite,     // p / q holds it
		zero,       // +-0, or too small for any p / q: the closest is 0 / 1
		nan,        // not a number
		infinite,   // +-inf
		overflow    // larger than bound / 1: the caller saturates
	};

	// The closest p / q to the exact dyadic A / B with p and q at most bound.
	//
	// The continued fraction of A / B gives its convergents, and each convergent is the closest
	// rational of its denominator or smaller. So the answer is the last convergent that still fits,
	// or the largest semiconvergent that fits when that one is closer. |A/B - p/q| is compared as
	// |A q - p B| * q', in twice the width, so the products cannot overflow. The expansion is the
	// Euclidean algorithm on (A, B), so it terminates; an exact A / B ends with y == 0.
	//
	// Preconditions: A >= 0, B > 0, bound > 0, and Wide holds A, B and bound * bound.
	template<typename Wide>
	void best_rational(const Wide& A, const Wide& B, const Wide& bound, Wide& p, Wide& q) {
		using Wider = blockbinary<2 * Wide::nbits, typename Wide::BlockType, BinaryNumberType::Signed>;
		auto widen  = [](const Wide& v) {
			Wider w;
			w.clear();
			for (unsigned i = 0; i < Wide::nbits; ++i)
				if (v.test(i)) w.setbit(i);
			return w;
		};
		// is p1 / q1 closer to A / B than p2 / q2? a tie keeps p2 / q2, the smaller denominator
		auto closer = [&](const Wide& p1, const Wide& q1, const Wide& p2, const Wide& q2) {
			const Wider a = widen(A), b = widen(B);
			auto distance = [&](const Wide& pp, const Wide& qq) {
				Wider e = a * widen(qq) - widen(pp) * b;
				if (e.sign()) e.twosComplement();
				return e;
			};
			return (distance(p1, q1) * widen(q2)) < (distance(p2, q2) * widen(q1));
		};

		Wide one;
		one.clear();
		one.setbit(0);
		Wide h = one, hPrev, k, kPrev = one, x = A, y = B;
		hPrev.clear();
		k.clear();
		p.clear();
		q = one;
		while (!y.iszero()) {
			const Wide quotient  = x / y;
			const Wide remainder = x % y;
			const bool fits      = (quotient <= bound) && ((quotient * h + hPrev) <= bound) && ((quotient * k + kPrev) <= bound);
			if (!fits) {
				Wide t = quotient;  // the largest semiconvergent that fits: hPrev + t h over kPrev + t k
				if (!h.iszero()) {
					const Wide room = (bound - hPrev) / h;
					if (room < t) t = room;
				}
				if (!k.iszero()) {
					const Wide room = (bound - kPrev) / k;
					if (room < t) t = room;
				}
				p = h;
				q = k;
				if (!t.iszero()) {
					const Wide semiP = hPrev + t * h, semiQ = kPrev + t * k;
					if (closer(semiP, semiQ, h, k)) {
						p = semiP;
						q = semiQ;
					}
				}
				return;
			}
			const Wide nextH = quotient * h + hPrev, nextK = quotient * k + kPrev;
			hPrev = h;
			h     = nextH;
			kPrev = k;
			k     = nextK;
			x     = y;
			y     = remainder;
		}
		p = h;  // A / B itself fits
		q = k;
		if (q.iszero()) q = one;  // a zero value
	}

	// a non-negative Wide value as digits, low digit first. The caller has bounded it by
	// radix^ndigits - 1, so it fits.
	template<typename Component, typename Wide>
	Component to_digit_component(const Wide& value) {
		Component c;
		c.clear();
		Wide x = value, r;
		r.clear();
		r.setbits(Component::radix);
		for (unsigned i = 0; i < Component::ndigits && !x.iszero(); ++i) {
			c.setdigit(i, static_cast<typename Component::Digit>((x % r).to_ull()));
			x = x / r;
		}
		return c;
	}

	// The closest p / q to a native float, double or long double, with p and q at most bound.
	// Wide must hold bound * bound and the value's significand, so bound's width plus the
	// significand's plus a little: for a binary128 long double that is 113 bits.
	template<typename Wide, typename Real>
	rational_conversion best_rational_from_native(Real v, const Wide& bound, bool& negative, Wide& p, Wide& q) {
		negative = false;
		p.clear();
		q.clear();
		q.setbit(0);
		if (v != v) return rational_conversion::nan;
		if (std::isinf(v)) {
			negative = (v < 0);
			return rational_conversion::infinite;
		}
		if (v == 0) return rational_conversion::zero;

		// |v| = significand * 2^exponent, exactly
		Wide significand;
		significand.clear();
		int exponent = 0;
#if LONG_DOUBLE_SUPPORT
		if constexpr (std::is_same_v<Real, long double>) {
			// every word of it: extractFields() hands out only the leading 64 bits, whose last is a
			// round-to-odd sticky bit on IEEE binary128 and double-double (#1517)
			long_double_significand sig(v);
			negative               = sig.negative();
			const int significandBits = static_cast<int>(Wide::nbits) - 2;
			int       read            = 0;
			while (!sig.empty() && read + 64 <= significandBits) {
				significand <<= 64;
				const uint64_t bits = sig.next();
				for (int i = 0; i < 64; ++i)
					if ((bits >> i) & 1ull) significand.setbit(static_cast<unsigned>(i));
				read += 64;
			}
			exponent = sig.scale() - (read - 1);
		}
		else
#endif
		{
			bool     s{false};
			uint64_t bits{0}, rawExponent{0}, rawFraction{0};
			extractFields(v, s, rawExponent, rawFraction, bits);
			negative            = s;
			constexpr int fbits = ieee754_parameter<Real>::fbits;
			if (rawExponent == 0) {  // subnormal: fraction * 2^(1 - bias - fbits)
				significand.setbits(rawFraction);
				exponent = 1 - ieee754_parameter<Real>::bias - fbits;
			}
			else {
				significand.setbits(rawFraction | ieee754_parameter<Real>::hmask);
				exponent = static_cast<int>(rawExponent) - ieee754_parameter<Real>::bias - fbits;
			}
		}

		// |v| = A / B, bounded: past bound the caller saturates, below 1 / (2 bound) it is zero
		const int significandBits = significand.msb() + 1;
		const int boundBits       = bound.msb() + 1;
		Wide      A, B;
		A.clear();
		B.clear();
		if (exponent >= 0) {
			if (significandBits + exponent > boundBits + 1) return rational_conversion::overflow;
			A = significand;
			A <<= exponent;
			B.setbit(0);
		}
		else {
			if (-exponent - significandBits > boundBits + 1) return rational_conversion::zero;
			A = significand;
			B.setbit(static_cast<unsigned>(-exponent));
		}
		if ((A / B) > bound) return rational_conversion::overflow;

		best_rational(A, B, bound, p, q);
		if (p.iszero()) return rational_conversion::zero;
		return rational_conversion::finite;
	}

}}  // namespace sw::universal
