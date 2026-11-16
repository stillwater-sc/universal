#pragma once
// integer_cast.hpp: the rule every Universal real type follows when it is cast to a native integer
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>
#include <limits>
#include <type_traits>

namespace sw { namespace universal {

	// The integer-cast rule (#2073, amended by #2104), used by posit, bposit, takum, takum_log,
	// cfloat and microfloat (other types do not include this header):
	//   - a finite value truncates toward zero, exactly at every width;
	//   - a value past the range, infinity included, saturates to +/-max(): a signed type to
	//     [-max(), max()], an unsigned type to [0, max() - 1];
	//   - NaN and NaR give the integer NaR, a pattern no finite value maps to: min() for a signed
	//     type (the 100...0 pattern of a posit NaR), max() for an unsigned type.
	// Before #2104 a NaN gave min(), the same as a large negative value, and an unsigned NaN gave
	// 0, the same as zero.

	// the integer NaR: the cast of a NaN or a NaR
	template<typename Integer>
	constexpr Integer integer_nar() noexcept {
		static_assert(std::is_integral_v<Integer>, "integer_nar: an integer type");
		if constexpr (std::is_signed_v<Integer>) return std::numeric_limits<Integer>::min();
		else return std::numeric_limits<Integer>::max();
	}

	// the largest magnitude a finite value casts to
	template<typename Integer>
	constexpr std::uint64_t integer_cast_max_magnitude() noexcept {
		static_assert(std::is_integral_v<Integer> && sizeof(Integer) <= 8, "integer_cast: an integer type of at most 64 bits");
		constexpr std::uint64_t top = static_cast<std::uint64_t>(std::numeric_limits<Integer>::max());
		if constexpr (std::is_signed_v<Integer>) return top;
		else return top - 1u;
	}

	// a finite value whose truncated magnitude is `magnitude`; `overflow` says it does not fit in
	// 64 bits.  Anything past the range saturates.
	template<typename Integer>
	constexpr Integer integer_from_magnitude(bool negative, bool overflow, std::uint64_t magnitude) noexcept {
		constexpr std::uint64_t cap = integer_cast_max_magnitude<Integer>();
		if (overflow || magnitude > cap) magnitude = cap;
		if (negative) {
			if constexpr (std::is_signed_v<Integer>) return static_cast<Integer>(-static_cast<Integer>(magnitude));
			else return Integer(0);
		}
		return static_cast<Integer>(magnitude);
	}

	// a value past the range, infinity included
	template<typename Integer>
	constexpr Integer integer_saturate(bool negative) noexcept {
		return integer_from_magnitude<Integer>(negative, true, 0);
	}

	// the rule applied to a native floating-point value: microfloat goes through its exact float
	template<typename Integer, typename Real>
	constexpr Integer integer_from_native(Real v) noexcept {
		static_assert(std::is_floating_point_v<Real>, "integer_from_native: a native floating-point type");
		if (v != v) return integer_nar<Integer>();
		const bool negative = v < Real(0);
		const Real a = negative ? -v : v;
		constexpr Real two64 = Real(18446744073709551616.0);   // 2^64, exact in every binary format
		if (!(a < two64)) return integer_saturate<Integer>(negative);   // infinity included
		return integer_from_magnitude<Integer>(negative, false, static_cast<std::uint64_t>(a));
	}

}} // namespace sw::universal
