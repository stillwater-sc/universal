#pragma once
// numeric_limits.hpp: definition of numeric_limits for generalized quire types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/utility/decimal_digits.hpp>   // exact digit/exponent counts (#1597, #1601, #1602, #1603, #1604)

namespace std {

// The quire is a sign-magnitude fixed-point accumulator: qbits magnitude bits, the lowest of
// them 2^-radix_point, and the non-finite states NaN, +inf and -inf (#2019). It accumulates
// exactly; a carry past the top bit wraps (is_modulo), and an operand outside it is dropped or,
// with QUIRE_THROW_ARITHMETIC_EXCEPTION, throws. These functions build a quire at run time (its
// bit setters are not constexpr), so unlike the native limits they are not constexpr (#2058).
template<typename NumberType, unsigned capacity, typename LimbType>
class numeric_limits< sw::universal::quire<NumberType, capacity, LimbType> > {
public:
	using QuireType = sw::universal::quire<NumberType, capacity, LimbType>;
	static constexpr bool is_specialized = true;
	static QuireType min() {            // the smallest positive value: bit 0, 2^-radix_point
		return QuireType(sw::universal::SpecificValue::minpos);
	}
	static QuireType max() {            // every magnitude bit set
		return QuireType(sw::universal::SpecificValue::maxpos);
	}
	static QuireType lowest() {         // -max()
		return QuireType(sw::universal::SpecificValue::maxneg);
	}
	static QuireType epsilon() {        // the step after 1.0 is the unit in the last place, as for fixpnt
		return min();
	}
	static QuireType round_error() {    // accumulation is exact
		return QuireType(sw::universal::SpecificValue::zero);
	}
	static QuireType denorm_min() {     // no subnormals: min(), as has_denorm == denorm_absent asks
		return min();
	}
	static QuireType infinity() {
		return QuireType(sw::universal::SpecificValue::infpos);
	}
	static QuireType quiet_NaN() {
		return QuireType(sw::universal::SpecificValue::qnan);
	}
	static QuireType signaling_NaN() {  // the quire has one NaN state
		return QuireType(sw::universal::SpecificValue::snan);
	}

	static constexpr int digits       = static_cast<int>(QuireType::qbits);
	// is_exact is true for a quire -- holding an exact sum is its entire purpose -- so it
	// takes the integer pair, and max_digits10 is 0 as it is for the native integers (#1601)
	static constexpr int digits10     = sw::universal::decimal_digits10_integer(digits);
	static constexpr int max_digits10 = 0;
	static constexpr bool is_signed   = true;
	static constexpr bool is_integer  = false;
	static constexpr bool is_exact    = true;
	static constexpr int radix        = 2;

	// fixed point: no exponent field
	static constexpr int min_exponent   = 0;
	static constexpr int min_exponent10 = 0;
	static constexpr int max_exponent   = 0;
	static constexpr int max_exponent10 = 0;
	static constexpr bool has_infinity  = true;
	static constexpr bool has_quiet_NaN = true;
	static constexpr bool has_signaling_NaN = false;
	static constexpr float_denorm_style has_denorm = denorm_absent;
	static constexpr bool has_denorm_loss = false;

	static constexpr bool is_iec559  = false;
	static constexpr bool is_bounded = true;
	static constexpr bool is_modulo  = true;   // a carry past the top magnitude bit wraps
	static constexpr bool traps      = false;
	static constexpr bool tinyness_before = false;
	static constexpr float_round_style round_style = round_toward_zero;   // bits below 2^-radix_point are dropped
};

}
