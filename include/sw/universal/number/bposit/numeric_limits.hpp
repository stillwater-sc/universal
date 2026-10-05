#pragma once
// numeric_limits.hpp: definition of numeric_limits for the bounded posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <limits>
#include <universal/utility/decimal_digits.hpp>   // digits10, max_digits10, exponent10

namespace std {

template <unsigned nbits, unsigned rs, unsigned es, typename bt>
class numeric_limits< sw::universal::bposit<nbits, rs, es, bt> > {
public:
	using BPosit = sw::universal::bposit<nbits, rs, es, bt>;
	static constexpr bool is_specialized = true;
	static constexpr BPosit min()           { return BPosit(sw::universal::SpecificValue::minpos); }
	static constexpr BPosit max()           { return BPosit(sw::universal::SpecificValue::maxpos); }
	static constexpr BPosit lowest()        { return BPosit(sw::universal::SpecificValue::maxneg); }
	// the gap above 1.0: the regime there is 2 bits, so fbits fraction bits
	static constexpr BPosit epsilon()       { return BPosit::pow2(-static_cast<int>(BPosit::fbits)); }
	static constexpr BPosit round_error()   { return BPosit::pow2(-1); }
	static constexpr BPosit denorm_min()    { return BPosit(sw::universal::SpecificValue::minpos); }
	// no infinity: maxpos is the saturation value, as for posits
	static constexpr BPosit infinity()      { return BPosit(sw::universal::SpecificValue::maxpos); }
	static constexpr BPosit quiet_NaN()     { return BPosit(sw::universal::SpecificValue::nar); }
	static constexpr BPosit signaling_NaN() { return BPosit(sw::universal::SpecificValue::nar); }

	static constexpr int  digits         = static_cast<int>(BPosit::fbits) + 1;   // significand bits at 1.0
	static constexpr int  digits10       = sw::universal::decimal_digits10(digits);
	static constexpr int  max_digits10   = sw::universal::decimal_max_digits10(digits);
	static constexpr bool is_signed      = true;
	static constexpr bool is_integer     = false;
	static constexpr bool is_exact       = false;
	static constexpr int  radix          = 2;
	// radix^(min_exponent - 1) is minpos's binade, radix^(max_exponent - 1) maxpos's
	static constexpr int  min_exponent   = BPosit::minscale + 1;
	static constexpr int  min_exponent10 = sw::universal::decimal_min_exponent10(min_exponent);
	static constexpr int  max_exponent   = BPosit::maxscale + 1;
	static constexpr int  max_exponent10 = sw::universal::decimal_max_exponent10(max_exponent);
	static constexpr bool has_infinity      = false;
	static constexpr bool has_quiet_NaN     = true;
	static constexpr bool has_signaling_NaN = false;
	static constexpr float_denorm_style has_denorm = denorm_absent;
	static constexpr bool has_denorm_loss   = false;
	static constexpr bool is_iec559         = false;
	static constexpr bool is_bounded        = true;
	static constexpr bool is_modulo         = false;
	static constexpr bool traps             = false;
	static constexpr bool tinyness_before   = false;
	static constexpr float_round_style round_style = round_to_nearest;
};

} // namespace std
