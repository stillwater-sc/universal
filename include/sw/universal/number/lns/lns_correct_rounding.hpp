#pragma once
// lns_correct_rounding.hpp: correctly rounded lns + - exp exp2, at every width (#2128)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// lns rounds in the log domain: a result's exponent k is the integer nearest to
// z = 2^rbits * log2|X|, ties to even, and then the Behavior's range rule applies (Saturating:
// k >= kmax gives maxpos, k <= kmin gives zero; Wrapping: k modulo 2^(nbits-1)). Products,
// quotients and square roots are exact integer arithmetic on k. Sums, differences and the
// exponentials are transcendental in z:
//
//     a +- b:  z = ka + 2^rbits * log2(1 +- 2^-d),  ka >= kb, d = (ka - kb) * 2^-rbits
//     exp2(x): z = 2^rbits * x,          exp(x): z = 2^rbits * x * log2(e)
//
// They used to go through double (DoubleTripAddSub: double(a) + double(b), converted back;
// exp: std::exp(double(x))), which is not correctly rounded past about 45 fraction bits
// (lns<64,59> sums were wrong in 99.7% of cases, by up to 88234 units) and overflows double's
// range (lns<32,20> reaches 2^1024). USER decision on #2128: correctly rounded at every width.
//
// Here z is decided by Ziv's strategy:
//   - a fast path in double, for nbits <= 64 and rbits <= 40: the correction term (or z) is
//     computed with sw::math::constexpr_math's exp2 and log2, whose truncation error is below an
//     ulp, and accepted only when the nearest integer is clear of a tie by a margin of 2^-42
//     relative, about 2^11 times the double evaluation's actual error;
//   - otherwise efloat at W bits with a rigorous error margin (itself an efloat), W doubling
//     from 2 * rbits + 96 to the 2048-bit cap. A z that sits within the margin of a tie even at
//     the cap -- the table maker's dilemma puts hard cases near 2^-(2p), far below it -- takes
//     the cap's nearest integer.
// The exponent stays an exact efloat until the range rule has applied, so any width works.
//
// The exact reference in verification/lns_rounding_oracle.hpp (#2125) is written separately,
// with its own margins and decoder, and checks this code over every 8-bit pair and at random.
#include <cstdint>
#include <universal/behavior/arithmetic.hpp>
#include <universal/number/efloat/efloat.hpp>
#include <universal/number/efloat/mathlib.hpp>
#include <math/constexpr_math.hpp>

namespace sw { namespace universal { namespace lns_rounding {

using E = efloat<72>;   // 2304 bits: the 2048-bit cap and guard bits

inline constexpr unsigned zivCap = 2048;

// ---------------------------------------------------------------------------------------------
// the exponent field

// the exponent of a finite nonzero lns as an exact efloat, any width
template<typename Lns>
constexpr E exponentOf(const Lns& v) {
	constexpr unsigned ebits = Lns::nbits - 1u;
	E k(0.0);
	for (unsigned i = 0; i + 1u < ebits; ++i) if (v.at(i)) k += ldexp(E(1.0), static_cast<int>(i));
	if (v.at(ebits - 1u)) k -= ldexp(E(1.0), static_cast<int>(ebits - 1u));
	return k;
}

// the exponent of a finite nonzero lns of at most 64 bits
template<typename Lns>
constexpr int64_t exponentOf64(const Lns& v) {
	static_assert(Lns::nbits <= 64u);
	constexpr unsigned ebits = Lns::nbits - 1u;
	int64_t k = 0;
	for (unsigned i = 0; i + 1u < ebits; ++i) if (v.at(i)) k |= (int64_t(1) << i);
	if (v.at(ebits - 1u)) k -= (int64_t(1) << (ebits - 1u));
	return k;
}

// write the exponent field from the low ebits bits of r >= 0 (an exact integer efloat), and the sign
template<typename Lns>
constexpr void writeField(Lns& v, E r, bool negative) {
	constexpr unsigned ebits = Lns::nbits - 1u;
	v.clear();
	for (unsigned i = 0; i < ebits; ++i) {
		const E half = floor(ldexp(r, -1));
		v.setbit(i, r != ldexp(half, 1));
		r = half;
	}
	v.setbit(Lns::nbits - 1u, negative);
}

// the encoding of an exact integer exponent n of any size, by the Behavior's range rule
template<typename Lns>
constexpr void encode(const E& n, bool negative, Lns& v) {
	constexpr unsigned ebits = Lns::nbits - 1u;
	const E m = ldexp(E(1.0), static_cast<int>(ebits));
	const E kmax = ldexp(E(1.0), static_cast<int>(ebits) - 1) - E(1.0);
	const E kmin = -ldexp(E(1.0), static_cast<int>(ebits) - 1);
	if constexpr (Lns::behavior == Behavior::Saturating) {
		if (n >= kmax) { writeField(v, kmax, negative); return; }
		if (n <= kmin) { v.setzero(); return; }
		writeField(v, (n < E(0.0)) ? n + m : n, negative);
	}
	else {
		writeField(v, n - m * floor(n / m), negative);   // n modulo 2^ebits, exactly
	}
}

// the same for a 64-bit exponent, without efloat
template<typename Lns>
constexpr void encode64(int64_t k, bool negative, Lns& v) {
	static_assert(Lns::nbits <= 64u);
	constexpr unsigned ebits = Lns::nbits - 1u;
	if constexpr (Lns::behavior == Behavior::Saturating) {
		constexpr int64_t kmax = (int64_t(1) << (ebits - 1u)) - 1;
		constexpr int64_t kmin = -(int64_t(1) << (ebits - 1u));
		if (k >= kmax) k = kmax;
		if (k <= kmin) { v.setzero(); return; }
	}
	v.clear();
	const uint64_t bits = static_cast<uint64_t>(k);   // two's complement, reduced modulo 2^ebits
	for (unsigned i = 0; i < ebits; ++i) v.setbit(i, ((bits >> i) & 1u) != 0);
	v.setbit(Lns::nbits - 1u, negative);
}

// ---------------------------------------------------------------------------------------------
// Ziv's retry in efloat

// the nearest integer to the exact value z(W) approximates within margin(W, |z|): decided when it
// is the only integer within 1/2 - margin; at the cap, the cap's nearest integer either way
template<typename Evaluate, typename Margin>
constexpr E nearestByZiv(Evaluate&& evaluate, Margin&& margin, unsigned startBits) {
	E candidate(0.0);
	for (unsigned W = (startBits < zivCap ? startBits : zivCap); ; W = (2u * W < zivCap ? 2u * W : zivCap)) {
		const E z = evaluate(W);
		candidate = floor(z + E(0.5));
		const bool enoughFractionBits = z.iszero() || z.scale() + 40 < static_cast<int64_t>(W);
		if (enoughFractionBits && abs(z - candidate) < E(0.5) - margin(W, abs(z))) return candidate;
		if (W == zivCap) break;
	}
	return candidate;
}

// floor(x + 1/2) for |x| < 2^52, in constexpr-safe double arithmetic
constexpr double roundHalfUp(double x) {
	const double y = x + 0.5;
	double i = static_cast<double>(static_cast<int64_t>(y));
	if (i > y) i -= 1.0;
	return i;
}

// ---------------------------------------------------------------------------------------------
// + and -

// lhs = lhs + rhs, or lhs - rhs when subtract, correctly rounded
template<typename Lns>
constexpr Lns& addsub(Lns& lhs, const Lns& rhs, bool subtract) {
	constexpr unsigned nbits = Lns::nbits;
	constexpr unsigned rbits = Lns::rbits;
	if (lhs.isnan() || rhs.isnan()) { lhs.setnan(); return lhs; }
	const bool sa = lhs.sign();
	const bool sb = subtract ? !rhs.sign() : rhs.sign();
	if (rhs.iszero()) return lhs;
	if (lhs.iszero()) { lhs = rhs; if (subtract) lhs = -lhs; return lhs; }
	const bool effectiveAdd = (sa == sb);

	if constexpr (nbits <= 64u) {
		int64_t ka = exponentOf64(lhs), kb = exponentOf64(rhs);
		bool negative = sa;
		if (kb > ka) { const int64_t t = ka; ka = kb; kb = t; negative = sb; }
		const int64_t dk = ka - kb;
		if (dk == 0) {
			if (!effectiveAdd) { lhs.setzero(); return lhs; }   // a - a
			// a + a = 2a: z = ka + 2^rbits, past int64 when rbits = 63
			if constexpr (rbits <= 61u) { encode64(ka + (int64_t(1) << rbits), negative, lhs); return lhs; }
			else { encode(E(static_cast<long long>(ka)) + ldexp(E(1.0), static_cast<int>(rbits)), negative, lhs); return lhs; }
		}
		// d >= rbits + 2: 2^rbits * |log2(1 +- 2^-d)| < 1/2, so the larger operand is the answer
		if constexpr (rbits <= 56u) {
			if (dk >= (static_cast<int64_t>(rbits) + 2) << rbits) { encode64(ka, negative, lhs); return lhs; }
		}
		else {   // the threshold passes int64: compare in double, 1/64 clear of it
			const double threshold = (static_cast<double>(rbits) + 2.0) * sw::math::constexpr_math::exp2(static_cast<double>(rbits));
			if (static_cast<double>(dk) >= threshold * (1.0 + 1.0 / 64.0)) { encode64(ka, negative, lhs); return lhs; }
		}
		if constexpr (rbits <= 40u) {
			const double d = static_cast<double>(dk) * sw::math::constexpr_math::exp2(-static_cast<double>(rbits));
			const double u = sw::math::constexpr_math::exp2(-d);
			const double c = sw::math::constexpr_math::log2(effectiveAdd ? 1.0 + u : 1.0 - u);
			const double scale = sw::math::constexpr_math::exp2(static_cast<double>(rbits));
			const double t = c * scale;
			const double n = roundHalfUp(t);
			const double distance = (t > n) ? t - n : n - t;
			const double absc = (c < 0.0) ? -c : c;
			const double margin = scale * (absc + u / (1.0 - u) + 1.0) * 0x1p-42;
			if (distance < 0.5 - margin) { encode64(ka + static_cast<int64_t>(n), negative, lhs); return lhs; }
		}
		// efloat, at every width the fast path does not reach
		const int64_t dkm = dk;
		auto evaluate = [ka, dkm, effectiveAdd](unsigned W) {
			E dneg(static_cast<long long>(-dkm));
			dneg.set_precision(W);
			const E u = exp2(ldexp(dneg, -static_cast<int>(rbits)));
			E one(1.0);
			one.set_precision(W);
			E base(static_cast<long long>(ka));
			base.set_precision(W);
			return base + ldexp(log2(effectiveAdd ? one + u : one - u), static_cast<int>(rbits));
		};
		auto margin = [](unsigned W, const E& absz) {
			const E two_r = ldexp(E(1.0), static_cast<int>(rbits));
			return ldexp(two_r * (E(8.0) * two_r + E(4.0 * rbits + 32.0)) + E(4.0) * absz, -static_cast<int>(W));
		};
		encode(nearestByZiv(evaluate, margin, 2u * rbits + 96u), negative, lhs);
		return lhs;
	}
	else {
		E ka = exponentOf(lhs), kb = exponentOf(rhs);
		bool negative = sa;
		if (kb > ka) { const E t = ka; ka = kb; kb = t; negative = sb; }
		const E dk = ka - kb;
		if (dk.iszero()) {
			if (!effectiveAdd) { lhs.setzero(); return lhs; }
			encode(ka + ldexp(E(1.0), static_cast<int>(rbits)), negative, lhs);
			return lhs;
		}
		if (dk >= E(static_cast<long long>(rbits) + 2) * ldexp(E(1.0), static_cast<int>(rbits))) { encode(ka, negative, lhs); return lhs; }
		auto evaluate = [ka, dk, effectiveAdd](unsigned W) {
			E dneg = -dk;
			dneg.set_precision(W);
			const E u = exp2(ldexp(dneg, -static_cast<int>(rbits)));
			E one(1.0);
			one.set_precision(W);
			E base = ka;
			base.set_precision(W);
			return base + ldexp(log2(effectiveAdd ? one + u : one - u), static_cast<int>(rbits));
		};
		auto margin = [](unsigned W, const E& absz) {
			E two_r = ldexp(E(1.0), static_cast<int>(rbits));
			return ldexp(two_r * (E(8.0) * two_r + E(4.0 * rbits + 32.0)) + E(4.0) * absz, -static_cast<int>(W));
		};
		encode(nearestByZiv(evaluate, margin, 2u * rbits + 96u + nbits), negative, lhs);
		return lhs;
	}
}

// ---------------------------------------------------------------------------------------------
// exp and exp2

// exp2(x) for f == '2', exp(x) for f == 'e', correctly rounded; an underflow takes the rounding
// rule (zero below half of minpos, as conversion and multiplication), USER decision on #2128
template<typename Lns>
Lns exponential(char f, const Lns& x) {
	constexpr unsigned nbits = Lns::nbits;
	constexpr unsigned rbits = Lns::rbits;
	constexpr unsigned ebits = nbits - 1u;
	Lns result;
	if (x.isnan()) { result.setnan(); return result; }
	if (x.iszero()) { encode(E(0.0), false, result); return result; }   // e^0 = 2^0 = 1
	const bool negx = x.sign();
	const E kx = exponentOf(x);
	const E one_r = ldexp(E(1.0), static_cast<int>(rbits));

	// exp2 of a power of two, x = +-2^j: z = +-2^(rbits + j) is a dyadic, which can sit exactly on a
	// tie (exp2(2^-4) at rbits = 3 is z = 1/2); round it exactly
	if (f == '2') {
		const E j = kx / one_r;
		if (floor(j) == j) {
			const double e = double(j) + double(rbits);   // the exponent of |z|, exactly as long as it is moderate
			if (e <= -1.0) { encode(E(0.0), false, result); return result; }   // |z| <= 1/2: 0, the tie to even
			if (e >= double(ebits)) {                                          // a multiple of 2^ebits
				if constexpr (Lns::behavior == Behavior::Saturating) {
					if (negx) result.setzero(); else encode(ldexp(E(1.0), static_cast<int>(ebits)), false, result);
				}
				else {
					encode(E(0.0), false, result);
				}
				return result;
			}
			const E z = ldexp(E(1.0), static_cast<int>(e));
			encode(negx ? -z : z, false, result);
			return result;
		}
	}

	// |z| from a double estimate: far past the field, Saturating clamps; Wrapping reduces z exactly
	// while it has few enough integer bits to round at the cap, and past that takes the old
	// overflow convention (maxpos for x > 0, zero for x < 0)
	const double log2x = double(kx) * std::ldexp(1.0, -static_cast<int>(rbits));
	const double log2z = log2x + double(rbits) + (f == 'e' ? 0.5287663729448977 : 0.0);   // + log2(log2(e))
	if (log2z > double(nbits) + 4.0) {
		if constexpr (Lns::behavior == Behavior::Saturating) {
			if (negx) result.setzero(); else encode(ldexp(E(1.0), static_cast<int>(ebits)), false, result);
			return result;
		}
		else {
			if (log2z > double(zivCap) - 256.0) {
				if (negx) result.setzero(); else result.maxpos();
				return result;
			}
		}
	}

	// the fast path in double
	if constexpr (nbits <= 64u && rbits <= 40u) {
		if (log2z < 50.0) {
			const double xd = std::exp2(log2x);
			double z = std::ldexp(xd, static_cast<int>(rbits)) * (f == 'e' ? 1.4426950408889634 : 1.0);
			if (negx) z = -z;
			const double n = roundHalfUp(z);
			const double distance = (z > n) ? z - n : n - z;
			const double margin = ((z < 0.0 ? -z : z) + 1.0) * 0x1p-42;
			if (distance < 0.5 - margin) { encode(E(n), false, result); return result; }
		}
	}

	const unsigned intbits = log2z > 0.0 ? static_cast<unsigned>(log2z) + 2u : 0u;
	auto evaluate = [&](unsigned W) {
		E t = kx;
		t.set_precision(W);
		E v = exp2(ldexp(t, -static_cast<int>(rbits)));   // |x|
		if (f == 'e') {
			E two(2.0);
			two.set_precision(W);
			v = v / log(two);                              // |x| * log2(e)
		}
		const E z = ldexp(v, static_cast<int>(rbits));
		return negx ? -z : z;
	};
	auto margin = [](unsigned W, const E& absz) { return ldexp(E(16.0) * absz + E(16.0), -static_cast<int>(W)); };
	encode(nearestByZiv(evaluate, margin, 2u * rbits + 96u + intbits), false, result);
	return result;
}

}}}  // namespace sw::universal::lns_rounding
