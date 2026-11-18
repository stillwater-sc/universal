#pragma once
// lns_rounding_oracle.hpp : lns results rounded by the lns rule in the log domain (#2125)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>
#include <cstdint>
#include <limits>

#include <universal/behavior/arithmetic.hpp>
#include <universal/number/lns/lns_fwd.hpp>
#include <universal/number/efloat/efloat.hpp>
#include <universal/number/efloat/mathlib.hpp>
#include <universal/verification/exact_reference.hpp>

/*
   The exact reference of an lns (exact_reference.hpp). An lns value is (-1)^s * 2^(k * 2^-rbits)
   for an exponent k held as an (nbits-1)-bit two's complement integer; its most negative value
   kmin is the zero encoding (1 in the sign bit makes it NaN). lns rounds in the log domain: the
   result's k is the integer nearest to z = 2^rbits * log2|X|, ties to even. This file reads k off
   the encoding with its own decoder and computes z as each operator needs it:

   - * and /: z = ka +- kb exactly, an integer.
   - sqrt: z = k / 2, and a tie (an odd k) goes to even.
   - + and -: |X| = 2^La +- 2^Lb, so with La >= Lb and d = La - Lb,
         z = ka + 2^rbits * log2(1 +- 2^-d),
     which is transcendental. It is evaluated in efloat at W bits with a rigorous error bound M
     (below), and decided when no tie point n + 1/2 lies within M of it. Otherwise W doubles, up to
     the cap of 2048 bits; a result still within M of a tie there is undecided. An undecided result
     has no reference: it is counted (undecided()) and reported, neither pass nor fail (#2125,
     USER 2026-11-17). Equal magnitudes are exact: a + a doubles (z = ka + 2^rbits), a - a is zero.
   - exp2 and exp: z = 2^rbits * x and 2^rbits * x * log2(e), with x = +-2^(kx 2^-rbits); again
     in efloat with Ziv's retry.

   Then the range, per Behavior. Saturating: k >= kmax gives maxpos (magnitude), k <= kmin gives
   zero. Wrapping: k is reduced modulo 2^(nbits-1) into the exponent field and the sign bit is set
   as it is, as lns's own operators do. A NaN operand gives NaN, so does x / 0 and the square root
   of a negative value.

   The error bound of the + and - evaluation, in units of 2^-W of z: u = 2^-d, s = 1 +- u and
   log2(s) are each rounded once at W bits, and z = ka + 2^rbits * log2(s) once more. The
   relative error of u carries into log2(s) as at most 1.5 * 2^-W * (u / (1 - u) + 1); with
   d >= 2^-rbits, u / (1 - u) <= 1.45 * 2^rbits + 1. The rounding of log2(s), at most 2^-W of
   |log2(s)| <= rbits + 2, and of z, 2^-W of |z|, add to it. So, with a factor of 2 to spare,
       M = 2^-W * (2^rbits * (8 * 2^rbits + 4 * rbits + 32) + 4 * |z|).
*/

namespace sw {
namespace universal {

namespace lns_oracle {

using E = efloat<72>;   // 2304 bits: the 2048-bit cap of the Ziv retry, and guard bits

inline constexpr unsigned zivCap = 2048;

// results left undecided at the Ziv cap, since the last reset
inline unsigned long long& undecidedCount() {
	static unsigned long long count = 0;
	return count;
}

template<typename Lns>
constexpr int64_t kmin() { return -(int64_t(1) << (Lns::nbits - 2u)); }
template<typename Lns>
constexpr int64_t kmax() { return (int64_t(1) << (Lns::nbits - 2u)) - 1; }

// the exponent k of a finite nonzero lns, read off its encoding
template<typename Lns>
int64_t exponentOf(const Lns& v) {
	constexpr unsigned ebits = Lns::nbits - 1u;
	int64_t k = 0;
	for (unsigned i = 0; i + 1u < ebits; ++i) if (v.at(i)) k |= (int64_t(1) << i);
	if (v.at(ebits - 1u)) k -= (int64_t(1) << (ebits - 1u));
	return k;
}

// the encoding of exponent k and sign, by the Behavior's range rule
template<typename Lns>
void encode(int64_t k, bool negative, Lns& ref) {
	constexpr unsigned ebits = Lns::nbits - 1u;
	if constexpr (Lns::behavior == Behavior::Saturating) {
		if (k >= kmax<Lns>()) k = kmax<Lns>();
		if (k <= kmin<Lns>()) { ref.setzero(); return; }
	}
	ref.clear();
	const uint64_t bits = static_cast<uint64_t>(k);   // two's complement, reduced modulo 2^ebits below
	for (unsigned i = 0; i < ebits; ++i) ref.setbit(i, ((bits >> i) & 1u) != 0);
	ref.setbit(Lns::nbits - 1u, negative);
}

// the nearest integer to z, ties to even; z is a finite efloat
inline int64_t nearestInteger(const E& z) {
	double approx = double(z);
	int64_t n = static_cast<int64_t>(std::llround(approx));
	for (int pass = 0; pass < 3; ++pass) {   // a double holds 53 bits of z: two refinements reach the integer
		const double r = double(z - E(static_cast<long long>(n)));
		const int64_t step = static_cast<int64_t>(std::llround(r));
		if (step == 0) break;
		n += step;
	}
	const E frac = z - E(static_cast<long long>(n));   // within [-1/2, 1/2]
	const E half(0.5);
	if (frac > half) ++n;
	else if (frac < -half) --n;
	else if (frac == half && (n & 1)) ++n;
	else if (frac == -half && (n & 1)) --n;
	return n;
}

// decide the nearest integer to the exact value that z(W) approximates within margin(W, z), by
// Ziv's retry; false when it stays undecided at the cap
template<typename Evaluate, typename Margin>
bool decide(Evaluate&& evaluate, Margin&& margin, unsigned startBits, int64_t& k) {
	for (unsigned W = startBits; W <= zivCap; W *= 2u) {
		const E z = evaluate(W);
		if (z.isnan() || z.isinf()) return false;
		const int64_t n = nearestInteger(z);
		const E distance = abs(z - E(static_cast<long long>(n)));
		const E limit = E(0.5) - E(margin(W, std::fabs(double(z))));
		if (distance < limit) { k = n; return true; }
	}
	++undecidedCount();
	return false;
}

}  // namespace lns_oracle

// the exact reference of an lns of at most 64 bits
template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
	requires (nbits <= 64u)
struct exact_rounding<lns<nbits, rbits, bt, xtra...>> {
	using Lns = lns<nbits, rbits, bt, xtra...>;
	using E = lns_oracle::E;

	static unsigned long long undecided() { return lns_oracle::undecidedCount(); }
	static void reset_undecided() { lns_oracle::undecidedCount() = 0; }

	static bool binary(char op, const Lns& a, const Lns& b, Lns& ref) {
		using namespace lns_oracle;
		if (op != '+' && op != '-' && op != '*' && op != '/') return false;
		if (a.isnan() || b.isnan()) { ref.setnan(); return true; }
		const bool za = a.iszero(), zb = b.iszero();
		const bool sa = a.sign(), sb = (op == '-') ? !b.sign() : b.sign();
		if (op == '*' || op == '/') {
			if (op == '/' && zb) { ref.setnan(); return true; }
			if (za || zb) { ref.setzero(); return true; }
			const int64_t ka = exponentOf(a), kb = exponentOf(b);
			encode<Lns>(op == '*' ? ka + kb : ka - kb, sa != sb, ref);
			return true;
		}
		// + and -: a + (+-b)
		if (za && zb) { ref.setzero(); return true; }
		if (zb) { ref = a; return true; }
		if (za) { ref = (op == '-') ? -b : b; return true; }
		int64_t ka = exponentOf(a), kb = exponentOf(b);
		bool negative = sa;
		if (kb > ka) { std::swap(ka, kb); negative = sb; }
		const bool effectiveAdd = (sa == sb);
		const int64_t dk = ka - kb;   // >= 0, in units of 2^-rbits
		if (dk == 0) {
			if (!effectiveAdd) { ref.setzero(); return true; }   // a - a
			encode<Lns>(ka + (int64_t(1) << rbits), negative, ref);  // a + a = 2a
			return true;
		}
		auto evaluate = [&](unsigned W) {
			E d(static_cast<long long>(-dk));
			d.set_precision(W);
			d = ldexp(d, -static_cast<int>(rbits));           // -d, exact
			const E u = exp2(d);                              // 2^-d
			E one(1.0);
			one.set_precision(W);
			const E s = effectiveAdd ? (one + u) : (one - u);
			E z = ldexp(log2(s), static_cast<int>(rbits));
			E base(static_cast<long long>(ka));
			base.set_precision(W);
			z = base + z;
			return z;
		};
		auto margin = [](unsigned W, double absz) {
			const double two_r = std::ldexp(1.0, static_cast<int>(rbits));
			return std::ldexp(two_r * (8.0 * two_r + 4.0 * rbits + 32.0) + 4.0 * absz, -static_cast<int>(W));
		};
		int64_t k{ 0 };
		if (!decide(evaluate, margin, 2u * rbits + 96u, k)) return false;
		encode<Lns>(k, negative, ref);
		return true;
	}

	static bool sqrt(const Lns& a, Lns& ref) {
		using namespace lns_oracle;
		if (a.isnan()) { ref.setnan(); return true; }
		if (a.iszero()) { ref.setzero(); return true; }
		if (a.sign()) { ref.setnan(); return true; }
		const int64_t k = exponentOf(a);
		int64_t h = k >> 1;                         // floor(k / 2)
		if ((k & 1) && (h & 1)) ++h;               // a tie: to even
		encode<Lns>(h, false, ref);
		return true;
	}

	// f is 'e' for exp, '2' for exp2
	static bool function(char f, const Lns& a, Lns& ref) {
		using namespace lns_oracle;
		if (f != 'e' && f != '2') return false;
		if (a.isnan()) { ref.setnan(); return true; }
		if (a.iszero()) { encode<Lns>(0, false, ref); return true; }   // e^0 = 2^0 = 1
		const int64_t kx = exponentOf(a);
		const bool negx = a.sign();
		// exp2 of a power of two, x = +-2^j: z = +-2^(rbits + j) is a dyadic, and can sit exactly on a
		// tie (exp2(2^-4) at rbits = 3 is z = 1/2), which no precision decides; round it exactly
		constexpr int64_t one = int64_t(1) << rbits;
		if (f == '2' && (kx % one) == 0) {
			const int64_t j = kx / one;
			const int64_t e = static_cast<int64_t>(rbits) + j;
			if (e >= static_cast<int64_t>(nbits) - 2) {   // |z| >= 2^(nbits-2), past the exponent field
				if constexpr (Lns::behavior == Behavior::Saturating) {
					if (negx) ref.setzero(); else encode<Lns>(kmax<Lns>(), false, ref);
					return true;
				}
				else {
					return false;
				}
			}
			E z(1.0);
			z = ldexp(z, static_cast<int>(e));
			if (negx) z = -z;
			encode<Lns>(nearestInteger(z), false, ref);
			return true;
		}
		auto evaluate = [&](unsigned W) {
			E t(static_cast<long long>(kx));
			t.set_precision(W);
			t = ldexp(t, -static_cast<int>(rbits));   // log2|x|, exact
			E x = exp2(t);                             // |x|
			if (f == 'e') {
				E two(2.0);
				two.set_precision(W);
				x = x / log(two);                      // |x| * log2(e)
			}
			E z = ldexp(x, static_cast<int>(rbits));
			return negx ? -z : z;
		};
		auto margin = [](unsigned W, double absz) { return std::ldexp(16.0 * absz + 16.0, -static_cast<int>(W)); };
		// |z| clearly past the exponent field (a double estimate, 4 units beyond its edge) saturates
		// or wraps; everything up to there is decided, and encode() applies the range rule exactly
		const double approx = std::ldexp(std::exp2(std::ldexp(double(kx), -static_cast<int>(rbits))) * (f == 'e' ? 1.4426950408889634 : 1.0), static_cast<int>(rbits));
		if (!(approx < std::ldexp(1.0, static_cast<int>(nbits) - 2) + 4.0)) {
			if constexpr (Lns::behavior == Behavior::Saturating) {
				if (negx) ref.setzero(); else encode<Lns>(kmax<Lns>(), false, ref);
				return true;
			}
			else {
				return false;   // wraps by an amount past the exponent field: no reference
			}
		}
		int64_t k{ 0 };
		if (!decide(evaluate, margin, 2u * rbits + 96u, k)) return false;
		encode<Lns>(k, false, ref);
		return true;
	}
};

}  // namespace universal
}  // namespace sw
