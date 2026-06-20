#pragma once
// numeric_limits.hpp: std::numeric_limits specialisation for elreal.
//
// elreal is a LAZY, unbounded-exponent real that is exact over the finite reals
// but carries an IEEE-style non-finite classification (#1079 Phase 5): it has
// quiet NaN and +-Inf states, so has_infinity / has_quiet_NaN are true and the
// factories return the real states. It has no fixed digit count (precision is
// pull-driven at runtime), so the precision-dependent fields (digits/epsilon) are
// reported against the COMPILE-TIME nominal default precision (kElrealDefaultPrecision
// blocks); the runtime precision can differ -- query an object's precision() for
// its actual pull depth. is_iec559 stays false (not a fixed-width IEEE format) and
// signalling NaN is not distinguished (snan folds to qnan).
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>
#include <limits>

#include <universal/number/elreal/elreal_fwd.hpp>
#include <universal/number/elreal/elreal_impl.hpp>

namespace std {

template <typename FpType>
class numeric_limits< sw::universal::elreal<FpType> > {
public:
    using ElrealType = sw::universal::elreal<FpType>;
    static constexpr bool is_specialized = true;

    // value range follows the host FpType (values materialise to host blocks); the
    // EXPONENT is effectively unbounded (integer<256>), hence is_bounded = false.
    static ElrealType min()    { return ElrealType(std::numeric_limits<FpType>::min()); }
    static ElrealType max()    { return ElrealType(std::numeric_limits<FpType>::max()); }
    static ElrealType lowest() { return ElrealType(std::numeric_limits<FpType>::lowest()); }
    // smallest increment from 1.0 at the nominal default precision (~2^-digits).
    static ElrealType epsilon()     { return ElrealType(std::ldexp(1.0, -digits)); }
    static ElrealType round_error() { return ElrealType(0.5); }
    // has_denorm = denorm_absent, so the C++20 contract requires denorm_min() to
    // return the minimum positive NORMALISED value, i.e. min().
    static ElrealType denorm_min()  { return min(); }
    // non-finite states (route through operator=(double), which classifies them).
    static ElrealType infinity()      { return ElrealType(std::numeric_limits<FpType>::infinity()); }
    static ElrealType quiet_NaN()     { return ElrealType(std::numeric_limits<FpType>::quiet_NaN()); }
    static ElrealType signaling_NaN() { return ElrealType(std::numeric_limits<FpType>::quiet_NaN()); }

    // precision-dependent: reported against the nominal default precision (blocks).
    static constexpr int  digits        = static_cast<int>(sw::universal::kElrealDefaultPrecision)
                                          * std::numeric_limits<FpType>::digits;
    static constexpr int  digits10      = static_cast<int>(digits * 0.30103);
    static constexpr int  max_digits10  = digits10 + 2;
    static constexpr bool is_signed     = true;
    static constexpr bool is_integer    = false;
    static constexpr bool is_exact      = false;
    static constexpr int  radix         = 2;

    static constexpr int  min_exponent   = std::numeric_limits<FpType>::min_exponent;
    static constexpr int  min_exponent10 = std::numeric_limits<FpType>::min_exponent10;
    static constexpr int  max_exponent   = std::numeric_limits<FpType>::max_exponent;
    static constexpr int  max_exponent10 = std::numeric_limits<FpType>::max_exponent10;

    static constexpr bool has_infinity              = true;
    static constexpr bool has_quiet_NaN             = true;
    static constexpr bool has_signaling_NaN         = false;   // snan folds to qnan
    static constexpr float_denorm_style has_denorm  = denorm_absent;
    static constexpr bool has_denorm_loss           = false;

    static constexpr bool is_iec559        = false;
    static constexpr bool is_bounded       = false;   // unbounded exponent
    static constexpr bool is_modulo        = false;
    static constexpr bool traps            = false;
    static constexpr bool tinyness_before  = false;
    static constexpr float_round_style round_style = round_toward_zero;
};

} // namespace std
