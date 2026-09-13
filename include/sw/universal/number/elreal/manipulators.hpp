#pragma once
// manipulators.hpp: type identification and rendering for elreal.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 2a of the elreal headers (#1334, Phase 2 group 5b, #1455): the <iomanip> half
// -- everything that turns an elreal into a std::string. The stream operators moved to
// iostream.hpp. Self-contained.
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include <universal/number/elreal/core.hpp>
#include <universal/number/elreal/block_manipulators.hpp>   // to_binary(block) / to_hex(block)

namespace sw { namespace universal {

// type_tag: a human-readable identifier (host type).
template <typename FpType>
inline std::string type_tag(const elreal<FpType>& = {}) {
    std::string host = "double";
    if (sizeof(FpType) == sizeof(float)) host = "float";
    return std::string("elreal<") + host + ">";
}

// to_components: the block expansion v ~ sum_i (block_i), each block's value, at the
// value's current precision. A non-empty stream renders down to precision() blocks.
// nonfinite_tag: "nan" / "inf" / "-inf". With include_sign=false the inf is
// unsigned ("inf") -- for renderings like to_triple that carry the sign separately.
template <typename FpType>
inline std::string nonfinite_tag(const elreal<FpType>& v, bool include_sign = true) {
    if (v.isnan()) return "nan";
    if (!include_sign) return "inf";
    return v.sign() < 0 ? "-inf" : "inf";
}

template <typename FpType>
inline std::string to_components(const elreal<FpType>& v) {
    if (!v.isfinite()) return std::string("( ") + nonfinite_tag(v) + " )";
    std::stringstream s;
    auto blocks = v.limbs(v.precision());
    s << "( ";
    for (std::size_t i = 0; i < blocks.size(); ++i) {
        s << std::setprecision(17) << blocks[i].template value_as<double>();
        if (i + 1 < blocks.size()) s << ", ";
    }
    if (blocks.empty()) s << "0";
    s << " )";
    return s.str();
}

// to_binary: the leading (and, if multi-block, trailing) block in binary.
template <typename FpType>
inline std::string to_binary(const elreal<FpType>& v, bool nibbleMarker = false) {
    if (!v.isfinite()) return nonfinite_tag(v);
    std::stringstream s;
    auto blocks = v.limbs(v.precision());
    if (blocks.empty()) { s << "0"; return s.str(); }
    s << to_binary(blocks.front(), nibbleMarker);
    if (blocks.size() > 1) s << " ... " << to_binary(blocks.back(), nibbleMarker);
    return s.str();
}

// to_triple: (sign, scale, significand) of the value, so that
// value ~ (sign) significand * 2^scale with significand in [1,2) (0 for zero).
template <typename FpType>
inline std::string to_triple(const elreal<FpType>& v) {
    if (!v.isfinite()) return std::string("(") + (v.isneg() ? "-, " : "+, ") + nonfinite_tag(v, false) + ')';
    std::stringstream s;
    const int64_t e = v.scale();
    const double  m = v.iszero() ? 0.0
                    : std::ldexp(v.template approx<double>(2), -static_cast<int>(e));
    s << (v.isneg() ? "(-, " : "(+, ") << e << ", "
      << std::setprecision(17) << m << ')';
    return s.str();
}

}} // namespace sw::universal
