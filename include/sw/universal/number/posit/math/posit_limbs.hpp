#pragma once
// posit_limbs.hpp: a posit's encoding on 64-bit limbs, decoded to its scale and fraction and
// encoded back with one rounding, at any nbits; shared by the posit math library (#1846, #1855)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstddef>
#include <cstdint>
#include <universal/internal/limbs/limb_kernels.hpp>

namespace sw { namespace universal {

	namespace posit_limbs {

		using limb_kernels::limb_t;

		constexpr std::size_t limbs(unsigned bits) noexcept { return (bits + 63u) / 64u; }
		inline bool test(const limb_t* w, unsigned i) noexcept { return ((w[i / 64u] >> (i % 64u)) & 1u) != 0u; }
		inline void set(limb_t* w, unsigned i) noexcept { w[i / 64u] |= limb_t(1) << (i % 64u); }
		inline void clear(limb_t* w, unsigned i) noexcept { w[i / 64u] &= ~(limb_t(1) << (i % 64u)); }

		// the encoding of a posit, gathered from its blocks
		template<unsigned nbits, unsigned es, typename bt>
		inline void gather(const posit<nbits, es, bt>& a, limb_t* w) noexcept {
			constexpr unsigned bitsInBlock = sizeof(bt) * 8u;
			const auto bits = a.bits();
			for (std::size_t i = 0; i < limbs(nbits); ++i) w[i] = 0u;
			for (unsigned i = 0; i < bits.nrBlocks; ++i) {
				const unsigned pos = i * bitsInBlock;
				w[pos / 64u] |= limb_t(bits.block(i)) << (pos % 64u);
			}
		}

		// the posit with encoding w[0, limbs(nbits)), bits at and above nbits ignored
		template<unsigned nbits, unsigned es, typename bt>
		inline posit<nbits, es, bt>& scatter(posit<nbits, es, bt>& p, const limb_t* w) noexcept {
			constexpr unsigned bitsInBlock = sizeof(bt) * 8u;
			blockbinary<nbits, bt, BinaryNumberType::Signed> bits;
			bits.clear();
			for (unsigned i = 0; i < bits.nrBlocks; ++i) {
				const unsigned pos = i * bitsInBlock;
				bits.setblock(i, static_cast<bt>(w[pos / 64u] >> (pos % 64u)));
			}
			return p.setbits(bits);
		}

		// a positive posit's scale, and its nf fraction bits, which are w[0, nf) on return: the
		// regime and exponent bits above them are cleared. Exponent bits cut off by the regime
		// are zero.
		struct Fields { int scale; unsigned nf; };
		template<unsigned nbits, unsigned es>
		inline Fields decode(limb_t* w) noexcept {
			const unsigned top = nbits - 2u;   // the first bit after the sign
			const bool positiveRegime = test(w, top);
			unsigned run = 1;
			while (run < nbits - 1u && test(w, top - run) == positiveRegime) ++run;
			const int k = positiveRegime ? static_cast<int>(run) - 1 : -static_cast<int>(run);
			const int below = static_cast<int>(nbits) - 2 - static_cast<int>(run);   // bits after the terminator
			int exponent = 0;
			for (int i = 1; i <= static_cast<int>(es); ++i) {
				const int j = below - i;
				exponent = (exponent << 1) | ((j >= 0 && test(w, static_cast<unsigned>(j))) ? 1 : 0);
			}
			const unsigned nf = (below > static_cast<int>(es)) ? static_cast<unsigned>(below) - es : 0u;
			for (std::size_t i = nf / 64u; i < limbs(nbits); ++i) {
				w[i] = (i == nf / 64u) ? (w[i] & ((limb_t(1) << (nf % 64u)) - 1u)) : 0u;
			}
			return { k * (1 << es) + exponent, nf };
		}

		// the scale and fraction length of a positive posit's encoding w, which is left as it is
		template<unsigned nbits, unsigned es>
		inline Fields fields(const limb_t* w) noexcept {
			limb_t t[limbs(nbits)];
			for (std::size_t i = 0; i < limbs(nbits); ++i) t[i] = w[i];
			return decode<nbits, es>(t);
		}

		// p = sig * 2^(scale - (64 n - 1)), sig = s[0, n) in [2^(64 n - 1), 2^(64 n)), with sticky
		// standing for nonzero bits below sig: rounded once to nearest, ties to even, and never to
		// zero or NaR. The sign is applied by the caller.
		template<unsigned nbits, unsigned es, typename bt>
		inline posit<nbits, es, bt>& encode(posit<nbits, es, bt>& p, int scale, const limb_t* sig, std::size_t n, bool sticky) noexcept {
			const int k = scale >> es;   // floor(scale / 2^es)
			const unsigned run = (k >= 0) ? static_cast<unsigned>(k) + 1u : static_cast<unsigned>(-k);
			if (run >= nbits - 1u) {   // the regime fills the posit: past maxpos or below minpos
				return (k >= 0) ? p.maxpos() : p.minpos();
			}
			// the unrounded body, T bits: the regime and its terminator, es exponent bits, and sig
			// below its hidden bit. Only the top nbits + 1 bits of sig can reach the rounding; the
			// rest joins the sticky bit.
			constexpr std::size_t S = limbs(nbits + 1u) + 1u;   // the significand window
			constexpr unsigned sbits = static_cast<unsigned>(64u * S);
			limb_t window[S];
			const bool lost = limb_kernels::load_window(window, S, sig, n, static_cast<std::int64_t>(sbits) - static_cast<std::int64_t>(64u * n));
			sticky = sticky || lost;
			constexpr std::size_t B = limbs(sbits + es + nbits);   // T < sbits + es + nbits
			limb_t body[B]{};
			for (std::size_t i = 0; i < S; ++i) body[i] = window[i];
			clear(body, sbits - 1u);
			const unsigned exponent = static_cast<unsigned>(scale - k * (1 << es));
			for (unsigned i = 0; i < es; ++i) {
				if ((exponent >> i) & 1u) set(body, sbits - 1u + i);
			}
			const unsigned terminator = sbits - 1u + es;
			if (k >= 0) { for (unsigned i = 1; i <= run; ++i) set(body, terminator + i); }
			else set(body, terminator);
			const unsigned T = terminator + run + 1u;

			// the top nbits - 1 bits, and the guard bit below them
			constexpr std::size_t L = limbs(nbits);
			limb_t r[L];
			const bool dropped = limb_kernels::load_window(r, L, body, B, static_cast<std::int64_t>(nbits) - static_cast<std::int64_t>(T));
			const bool guard = (r[0] & 1u) != 0u;
			limb_kernels::rshift(r, r, L, 1u);
			if (guard && (dropped || sticky || (r[0] & 1u) != 0u)) limb_kernels::add_1(r, L, 1u);
			return scatter(p, r);
		}

		// the posit's negation, on its encoding: two's complement of w[0, limbs(nbits)) at nbits bits
		template<unsigned nbits>
		inline void negate(limb_t* w) noexcept {
			constexpr std::size_t L = limbs(nbits);
			for (std::size_t i = 0; i < L; ++i) w[i] = ~w[i];
			limb_kernels::add_1(w, L, 1u);
			if constexpr (nbits % 64u != 0u) w[L - 1] &= (limb_t(1) << (nbits % 64u)) - 1u;
		}

	} // namespace posit_limbs

}} // namespace sw::universal
