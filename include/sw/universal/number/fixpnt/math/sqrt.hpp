#pragma once
#include <cerrno>
#include <cstdint>
#include <limits>        // std::numeric_limits
// sqrt.hpp: sqrt functions for fixed-points
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/native/ieee754.hpp>
#include <universal/number/fixpnt/numeric_limits.hpp>
#include <universal/number/fixpnt/math/sqrt_tables.hpp>

namespace sw { namespace universal {

	/*
	- Consider the function argument, x, in floating-point form, with a base
	(or radix) B, exponent e, and a fraction, f , such that 1/B <= f < 1.
	Then we have x = f Be. The number of bits in the exponent and
	fraction, and the value of the base, depends on the particular floating
	point arithmetic system chosen.

	- Use properties of the elementary function to range reduce the argument
	x to a small fixed interval.

	- Use a small polynomial approximation to produce an initial estimate,
	y0, of the function on the small interval. Such an estimate may
	be good to perhaps 5 to 10 bits.

	- Apply Newton iteration to refine the result. This takes the form yk =
	yk_1/2 + (f /2)/yk_1. In base 2, the divisions by two can be done by
	exponent adjustments in floating-point computation, or by bit shifting
	in fixed-point computation.

	Convergence of the Newton method is quadratic, so the number of
	correct bits doubles with each iteration. Thus, a starting point correct
	to 7 bits will produce iterates accurate to 14, 28, 56, ... bits. Since the
	number of iterations is very small, and known in advance, the loop is
	written as straight-line code.

	- Having computed the function value for the range-reduced argument,
	make whatever adjustments are necessary to produce the function value
	for the original argument; this step may involve a sign adjustment,
	and possibly a single multiplication and/or addition.
	*/


	namespace fixpnt_sqrt_detail {

		// a little-endian unsigned integer of W bits, just what the integer square root needs
		template<unsigned W>
		struct wide {
			static constexpr unsigned N = (W + 31u) / 32u;
			std::uint32_t w[N];
			void clear() { for (unsigned i = 0; i < N; ++i) w[i] = 0; }
			bool test(unsigned i) const { return (i < 32u * N) && ((w[i / 32u] >> (i % 32u)) & 1u); }
			void set(unsigned i) { if (i < 32u * N) w[i / 32u] |= (std::uint32_t(1) << (i % 32u)); }
			bool iszero() const { for (unsigned i = 0; i < N; ++i) if (w[i]) return false; return true; }
			int compare(const wide& o) const {
				for (unsigned i = N; i-- > 0;) { if (w[i] != o.w[i]) return (w[i] < o.w[i]) ? -1 : 1; }
				return 0;
			}
			void add(const wide& o) {
				std::uint64_t c = 0;
				for (unsigned i = 0; i < N; ++i) { c += std::uint64_t(w[i]) + o.w[i]; w[i] = static_cast<std::uint32_t>(c); c >>= 32; }
			}
			void sub(const wide& o) {                       // *this >= o
				std::int64_t b = 0;
				for (unsigned i = 0; i < N; ++i) {
					std::int64_t d = std::int64_t(w[i]) - o.w[i] - b;
					b = (d < 0) ? 1 : 0;
					w[i] = static_cast<std::uint32_t>(d + (b << 32));
				}
			}
			void shr(unsigned s) {                          // s < 32
				for (unsigned i = 0; i < N; ++i) {
					std::uint32_t hi = (i + 1 < N) ? w[i + 1] : 0u;
					w[i] = (w[i] >> s) | ((s != 0u) ? (hi << (32u - s)) : 0u);
				}
			}
		};

		// floor(sqrt(m)) and m - floor(sqrt(m))^2, by the restoring digit-by-digit method:
		// exact, two bits of m per step
		template<unsigned W>
		void isqrt(wide<W> m, wide<W>& root, wide<W>& rem) {
			root.clear();
			int top = static_cast<int>(32u * wide<W>::N) - 1;
			while (top >= 0 && !m.test(static_cast<unsigned>(top))) --top;
			if (top < 0) { rem = m; return; }
			wide<W> bit;
			bit.clear();
			bit.set(static_cast<unsigned>(top) & ~1u);      // the highest power of four <= m
			while (!bit.iszero()) {
				wide<W> trial = root;
				trial.add(bit);
				if (m.compare(trial) >= 0) {
					m.sub(trial);
					root.shr(1);
					root.add(bit);
				}
				else {
					root.shr(1);
				}
				bit.shr(2);
			}
			rem = m;
		}

	}  // namespace fixpnt_sqrt_detail

	// sqrt for fixpnt, correctly rounded (#1725).
	//
	// A fixpnt value is N 2^-rbits for an integer N, so its root is sqrt(N 2^rbits) 2^-rbits:
	// the result is the integer R nearest sqrt(M) for M = N 2^rbits, computed exactly by an
	// integer square root.  With s = floor(sqrt(M)) and the remainder M - s^2, the root rounds
	// up exactly when the remainder exceeds s, since (s + 1/2)^2 = s^2 + s + 1/4 and M is an
	// integer: a tie cannot occur.  Only a format without an integer bit (rbits >= nbits - 1)
	// has a root it cannot hold, sqrt of a value in [maxpos^2, maxpos], which saturates.
	//
	// This used to be two different functions.  fixpnt.hpp defaulted FIXPNT_NATIVE_SQRT to 1
	// and selected a Babylonian iteration that stopped on an epsilon test, not at the correctly
	// rounded root, and threw on a negative argument whatever FIXPNT_THROW_ARITHMETIC_EXCEPTION
	// said; math/sqrt.hpp defaulted the switch to 0 and returned fixpnt(std::sqrt(double(a))),
	// so a translation unit that included the core and the mathlib without the umbrella got a
	// root through double.  The switch is gone.
	//
	// fixpnt has no NaN: a negative argument throws fixpnt_negative_sqrt_arg under
	// FIXPNT_THROW_ARITHMETIC_EXCEPTION, and otherwise sets errno = EDOM and returns 0.
	template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
	inline fixpnt<nbits, rbits, arithmetic, bt> sqrt(const fixpnt<nbits, rbits, arithmetic, bt>& a) {
		using Fixed = fixpnt<nbits, rbits, arithmetic, bt>;
		if (a.iszero()) return a;
		if (a.isneg()) {
#if FIXPNT_THROW_ARITHMETIC_EXCEPTION
			throw fixpnt_negative_sqrt_arg();
#else
			errno = EDOM;
			return Fixed(0);
#endif
		}
		constexpr unsigned W = nbits + rbits + 2u;
		using Wide = fixpnt_sqrt_detail::wide<W>;
		Wide m, root, rem;
		m.clear();
		for (unsigned i = 0; i + 1u < nbits; ++i) if (a.at(i)) m.set(i + rbits);   // M = N 2^rbits, N > 0
		fixpnt_sqrt_detail::isqrt(m, root, rem);
		if (rem.compare(root) > 0) {                    // round to nearest: up when rem > s
			Wide one;
			one.clear();
			one.set(0);
			root.add(one);
		}
		// the largest raw value is 2^(nbits - 1) - 1: anything at or above 2^(nbits - 1) saturates
		bool overflow = false;
		for (unsigned i = nbits - 1u; i < 32u * Wide::N; ++i) if (root.test(i)) { overflow = true; break; }
		Fixed r(0);
		if (overflow) return r.maxpos();
		for (unsigned i = 0; i + 1u < nbits; ++i) r.setbit(i, root.test(i));
		return r;
	}

	// reciprocal sqrt
	template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
	inline fixpnt<nbits, rbits, arithmetic, bt> rsqrt(const fixpnt<nbits, rbits, arithmetic, bt>& f) {
		fixpnt<nbits, rbits, arithmetic, bt> rsqrt = sqrt(f);
		return rsqrt.reciprocate();
	}

	///////////////////////////////////////////////////////////////////
	// specialized sqrt configurations

}} // namespace sw::universal
