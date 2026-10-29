#pragma once
// truncate.hpp: truncation functions (trunc, round, floor, and ceil) for posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// All four are exact and computed on the encoding, at any nbits (#1855). A posit x = (-1)^s 1.f 2^scale
// with nf fraction bits is an integer when scale >= nf; for 0 <= scale < nf its integer part is x with
// the low nf - scale fraction bits cleared, a shorter fraction at the same regime and exponent, so it
// is a posit; for scale < 0, |x| < 1. floor, ceil and round then step the integer part by one in
// magnitude, which is exact: either it stays in x's binade, where an ulp is at most 1, or it carries
// to the power of two 2^(scale+1), which every posit in range represents. No posit arithmetic is
// involved: the step is an addition on the encoding too.
#include <cstddef>
#include <universal/number/posit/math/posit_limbs.hpp>

namespace sw { namespace universal {

	namespace posit_limbs {

		// |x| without its fraction, for a nonzero, non-NaR x, and what the cleared bits held: any
		// nonzero bit, and the half bit (weight 1/2); up is |x| with its fraction cleared plus one
		template<unsigned nbits, unsigned es, typename bt>
		struct IntegerPart {
			posit<nbits, es, bt> magnitude;
			posit<nbits, es, bt> up;
			bool negative;
			bool fraction;   // the cleared bits are nonzero: x is not an integer
			bool half;       // the bit of weight 1/2 is set
		};

		// The step to the next integer is on the encoding as well: adding 2^u, u the position of the
		// bit of weight 1, carries out of the fraction exactly when the result is the power of two
		// 2^(scale+1), and the encoding with that carry is 2^(scale+1)'s, since a posit of fewer bits
		// padded with zeros is the same value.
		template<unsigned nbits, unsigned es, typename bt>
		inline IntegerPart<nbits, es, bt> integer_part(const posit<nbits, es, bt>& x, bool wantUp) noexcept {
			using Posit = posit<nbits, es, bt>;
			IntegerPart<nbits, es, bt> ip{ x.isneg() ? -x : x, Posit(1), x.isneg(), false, false };
			limb_t w[limbs(nbits)];
			gather(ip.magnitude, w);
			const Fields f = fields<nbits, es>(w);
			if (f.scale < 0) {   // |x| < 1: the integers around it are 0 and 1
				ip.magnitude = Posit(0);
				ip.fraction = true;
				ip.half = (f.scale == -1);
				return ip;
			}
			if (f.scale >= static_cast<int>(f.nf)) return ip;   // already an integer; up is not used
			const unsigned unit = f.nf - static_cast<unsigned>(f.scale);   // in [1, nf]
			ip.half = test(w, unit - 1u);
			for (unsigned i = 0; i < unit; ++i) {
				if (test(w, i)) { ip.fraction = true; clear(w, i); }
			}
			scatter(ip.magnitude, w);
			if (wantUp) {
				limb_t one[limbs(nbits)]{};
				set(one, unit);
				limb_kernels::add_n(w, w, one, limbs(nbits));
				scatter(ip.up, w);
			}
			return ip;
		}

	} // namespace posit_limbs

	// Truncate value by rounding toward zero, returning the nearest integral value that is not larger in magnitude than x
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits,es,bt> trunc(posit<nbits,es,bt> x) {
		if (x.iszero() || x.isnar()) return x;
		const auto ip = posit_limbs::integer_part(x, false);
		return ip.negative ? -ip.magnitude : ip.magnitude;
	}

	// Round to nearest: returns the integral value that is nearest to x, with halfway cases rounded away from zero
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits,es,bt> round(posit<nbits,es,bt> x) {
		if (x.iszero() || x.isnar()) return x;
		const auto ip = posit_limbs::integer_part(x, true);
		const posit<nbits, es, bt>& m = ip.half ? ip.up : ip.magnitude;
		return ip.negative ? -m : m;
	}

	// Round x downward, returning the largest integral value that is not greater than x
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits,es,bt> floor(posit<nbits,es,bt> x) {
		if (x.iszero() || x.isnar()) return x;
		const auto ip = posit_limbs::integer_part(x, x.isneg());
		if (!ip.negative) return ip.magnitude;
		return -(ip.fraction ? ip.up : ip.magnitude);
	}

	// Round x upward, returning the smallest integral value that is greater than x
	template<unsigned nbits, unsigned es, typename bt>
	posit<nbits,es,bt> ceil(posit<nbits,es,bt> x) {
		if (x.iszero() || x.isnar()) return x;
		const auto ip = posit_limbs::integer_part(x, !x.isneg());
		if (ip.negative) return -ip.magnitude;
		return ip.fraction ? ip.up : ip.magnitude;
	}

}} // namespace sw::universal
