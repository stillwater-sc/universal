#pragma once
// numeric_limits.hpp: definition of numeric_limits for the poxel
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <limits>
#include <universal/utility/decimal_digits.hpp>   // digits10, max_digits10, exponent10

namespace std {

template <unsigned nbits, unsigned es, typename bt>
class numeric_limits< sw::universal::poxel<nbits, es, bt> > {
public:
	using Poxel = sw::universal::poxel<nbits, es, bt>;
	static constexpr bool is_specialized = true;
	static constexpr Poxel min()           { return Poxel(sw::universal::SpecificValue::minpos); }
	static constexpr Poxel max()           { return Poxel(sw::universal::SpecificValue::maxpos); }
	static constexpr Poxel lowest()        { return Poxel(sw::universal::SpecificValue::maxneg); }
	// the lattice gap above 1.0, as an exact tile: 2^-fbits
	static constexpr Poxel epsilon()       { Poxel p; p.setbits(Poxel::encode(false, -static_cast<int>(Poxel::fbits), 0ull, 0u, false)); return p; }
	static constexpr Poxel round_error()   { Poxel p; p.setbits(Poxel::encode(false, -1, 0ull, 0u, false)); return p; }
	static constexpr Poxel denorm_min()    { return Poxel(sw::universal::SpecificValue::minpos); }
	// the open tile (maxpos, inf) stands in for infinity
	static constexpr Poxel infinity()      { return Poxel(sw::universal::SpecificValue::infpos); }
	static constexpr Poxel quiet_NaN()     { return Poxel(sw::universal::SpecificValue::nar); }
	static constexpr Poxel signaling_NaN() { return Poxel(sw::universal::SpecificValue::nar); }

	static constexpr int  digits         = static_cast<int>(Poxel::fbits) + 1;
	static constexpr int  digits10       = sw::universal::decimal_digits10(digits);
	static constexpr int  max_digits10   = sw::universal::decimal_max_digits10(digits);
	static constexpr bool is_signed      = true;
	static constexpr bool is_integer     = false;
	static constexpr bool is_exact       = false;
	static constexpr int  radix          = 2;
	static constexpr int  min_exponent   = Poxel::minscale + 1;
	static constexpr int  min_exponent10 = sw::universal::decimal_min_exponent10(min_exponent);
	static constexpr int  max_exponent   = Poxel::maxscale + 1;
	static constexpr int  max_exponent10 = sw::universal::decimal_max_exponent10(max_exponent);
	static constexpr bool has_infinity      = false;   // the end tiles are open intervals, not infinities
	static constexpr bool has_quiet_NaN     = true;
	static constexpr bool has_signaling_NaN = false;
	static constexpr float_denorm_style has_denorm = denorm_absent;
	static constexpr bool has_denorm_loss   = false;
	static constexpr bool is_iec559         = false;
	static constexpr bool is_bounded        = true;
	static constexpr bool is_modulo         = false;
	static constexpr bool traps             = false;
	static constexpr bool tinyness_before   = false;
	static constexpr float_round_style round_style = round_toward_neg_infinity;   // a value is stored as its tile's lower end
};

} // namespace std
