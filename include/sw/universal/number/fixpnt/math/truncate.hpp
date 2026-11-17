#pragma once
// truncate.hpp: truncation functions (trunc, round, floor, and ceil) for fixed-points
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

namespace detail {
	// x rounded to an integral value, from the encoding (#2111: through double, which is wrong past
	// 53 bits). The rule acts on the magnitude; a result past the range then follows the arithmetic
	// mode: Saturate clamps and Modulo wraps, as fixpnt arithmetic does.
	enum class fixpnt_integral { trunc, round, floor, ceil };
	template<fixpnt_integral mode, unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
	fixpnt<nbits, rbits, arithmetic, bt> to_integral(const fixpnt<nbits, rbits, arithmetic, bt>& x) {
		if constexpr (rbits == 0) {
			return x;
		}
		else {
			using Wide = blockbinary<nbits + 2, bt, BinaryNumberType::Signed>;
			Wide m(x.bits());
			const bool negative = m.sign();
			if (negative) m.twosComplement();
			const bool fractional = m.any(rbits - 1);   // a bit below the radix point
			const bool half       = m.test(rbits - 1);  // the magnitude's fraction is 1/2 or more
			for (unsigned i = 0; i < rbits; ++i) m.setbit(i, false);
			bool up = false;
			switch (mode) {
			case fixpnt_integral::trunc: up = false; break;
			case fixpnt_integral::round: up = half; break;                   // half away from zero
			case fixpnt_integral::floor: up = negative && fractional; break;
			case fixpnt_integral::ceil:  up = !negative && fractional; break;
			}
			if (up) {
				Wide one;
				one.clear();
				one.setbit(rbits, true);
				m += one;
			}
			if (negative) m.twosComplement();
			fixpnt<nbits, rbits, arithmetic, bt> r;
			if constexpr (arithmetic == Saturate) {
				const Wide maxposW{ blockbinary<nbits, bt, BinaryNumberType::Signed>(SpecificValue::maxpos) };
				const Wide maxnegW{ blockbinary<nbits, bt, BinaryNumberType::Signed>(SpecificValue::maxneg) };
				if (m > maxposW) return r.maxpos();
				if (m < maxnegW) return r.maxneg();
			}
			r = m;   // the low nbits
			return r;
		}
	}
}

// Truncate value by rounding toward zero, returning the nearest integral value that is not larger in magnitude than x
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> trunc(fixpnt<nbits, rbits, arithmetic, bt> x) {
	return detail::to_integral<detail::fixpnt_integral::trunc>(x);
}

// Round to nearest: returns the integral value that is nearest to x, with halfway cases rounded away from zero
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> round(fixpnt<nbits, rbits, arithmetic, bt> x) {
	return detail::to_integral<detail::fixpnt_integral::round>(x);
}

// Round x downward, returning the largest integral value that is not greater than x
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> floor(fixpnt<nbits, rbits, arithmetic, bt> x) {
	return detail::to_integral<detail::fixpnt_integral::floor>(x);
}

// Round x upward, returning the smallest integral value that is not less than x
template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
fixpnt<nbits, rbits, arithmetic, bt> ceil(fixpnt<nbits, rbits, arithmetic, bt> x) {
	return detail::to_integral<detail::fixpnt_integral::ceil>(x);
}

}} // namespace sw::universal
