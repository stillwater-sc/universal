#pragma once
// efloat_bridge.hpp: a posit to and from efloat, for the parts of the posit math library that need an
// exact or a wider intermediate (#1855)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// to_efloat is exact: the value carries the posit's whole significand as its precision. from_efloat
// rounds once, to nearest with ties to even on the posit's encoding, through posit_limbs::encode, so a
// correctly rounded or exact efloat result becomes a correctly rounded posit. Neither goes through a
// native floating-point type, so the conversions hold at any nbits and over the posit's whole range.
#include <cstddef>
#include <cstdint>
#include <universal/number/efloat/core.hpp>
#include <universal/number/posit/math/posit_limbs.hpp>

namespace sw { namespace universal {

	namespace posit_limbs {

		// an efloat that holds a posit<nbits, ...> exactly, with room to spare: 32-bit units
		template<unsigned nbits>
		using efloat_for = efloat<static_cast<unsigned>(2u * limbs(nbits) + 2u)>;

		// x, exactly; NaR is a quiet NaN
		template<unsigned nbits, unsigned es, typename bt>
		inline efloat_for<nbits> to_efloat(const posit<nbits, es, bt>& x) {
			using E = efloat_for<nbits>;
			if (x.iszero()) return E(0);
			if (x.isnar()) { E e(0); e.setnan(); return e; }
			const bool negative = x.isneg();
			limb_t w[limbs(nbits)];
			gather(negative ? -x : x, w);
			const Fields f = decode<nbits, es>(w);   // w = the nf fraction bits
			constexpr std::size_t n = limbs(nbits);   // nf + 1 < nbits significand bits
			limb_t r[n];
			limb_kernels::load_window(r, n, w, limbs(nbits), static_cast<std::int64_t>(64u * n - 1u) - static_cast<std::int64_t>(f.nf));
			set(r, static_cast<unsigned>(64u * n - 1u));   // the hidden bit
			E e(0);
			e.assign_rounded_limbs(negative, r, n, f.scale, false, static_cast<unsigned>(2u * n));
			return e;
		}

		// v rounded once to the posit; NaN becomes NaR, as does an infinity, which has no posit
		template<unsigned nbits, unsigned es, typename bt, unsigned nlimbs>
		inline posit<nbits, es, bt>& from_efloat(posit<nbits, es, bt>& p, const efloat<nlimbs>& v) {
			if (v.isnan() || v.isinf()) { p.setnar(); return p; }
			if (v.iszero()) { p.setzero(); return p; }
			// past maxpos and below minpos the posit saturates: clamp the scale so that it fits an int
			constexpr std::int64_t reach = static_cast<std::int64_t>(nbits) * (std::int64_t(1) << es) + 2;
			std::int64_t scale = v.scale();
			if (scale > reach) scale = reach;
			if (scale < -reach) scale = -reach;
			encode(p, static_cast<int>(scale), v.limb_data(), v.limb_count(), false);
			if (v.sign() < 0) p = -p;
			return p;
		}

	} // namespace posit_limbs

}} // namespace sw::universal
