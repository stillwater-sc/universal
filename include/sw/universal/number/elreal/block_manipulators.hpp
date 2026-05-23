// block_manipulators.hpp: pretty-printers for elreal block<FpType>.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

#include <cmath>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <string>
#include <typeinfo>

#include <universal/number/elreal/block.hpp>

namespace sw { namespace universal {

template <typename FpType>
inline std::string type_tag(const block<FpType>& = {}) {
    std::stringstream s;
    s << "block<" << typeid(FpType).name() << '>';
    return s.str();
}

// to_binary: human-readable rendering as
//   <sign> 2^<scale> * <fraction>  [ ^ <exp_offset> ]
template <typename FpType>
inline std::string to_binary(const block<FpType>& b, bool /*nibbleMarker*/ = false) {
    std::stringstream s;
    if (b.is_zero_block()) {
        s << "block<>{0}";
        if (b.exp_offset != 0) s << " ^ " << b.exp_offset;
        return s.str();
    }
    s << (b.sign() < 0 ? '-' : '+');
    s << " 2^" << b.scale_of_v();
    double abs_frac = std::fabs(static_cast<double>(b.v) / std::ldexp(1.0, b.scale_of_v()));
    s << " * " << std::scientific << std::setprecision(6) << abs_frac;
    if (b.exp_offset != 0) {
        s << " ^ " << b.exp_offset;
    }
    return s.str();
}

// to_hex: render the native host FpType bit pattern in hex, else fall back to
// the human-readable form. Bit-level hex for Universal wrapper types is the
// responsibility of the wrapper's own to_hex (we do not delegate to avoid
// cross-header dependency churn).
template <typename FpType>
inline std::string to_hex(const block<FpType>& b,
                          bool /*nibbleMarker*/ = false,
                          bool hexPrefix        = true) {
    std::stringstream s;
    if constexpr (std::is_same_v<FpType, float>) {
        std::uint32_t bits = 0;
        std::memcpy(&bits, &b.v, sizeof(bits));
        if (hexPrefix) s << "0x";
        s << std::hex << std::setw(8) << std::setfill('0') << bits;
    } else if constexpr (std::is_same_v<FpType, double>) {
        std::uint64_t bits = 0;
        std::memcpy(&bits, &b.v, sizeof(bits));
        if (hexPrefix) s << "0x";
        s << std::hex << std::setw(16) << std::setfill('0') << bits;
    } else {
        // Universal wrapper types: defer to the human-readable form for now.
        return to_binary(b);
    }
    if (b.exp_offset != 0) {
        s << std::dec << " ^ " << b.exp_offset;
    }
    return s.str();
}

// color_print: ASCII-only render. Phase 1 reuses to_binary; richer colouring
// (per-bit highlighting of sign/exponent/significand fields) can come later.
template <typename FpType>
inline std::string color_print(const block<FpType>& b, bool nibbleMarker = false) {
    return to_binary(b, nibbleMarker);
}

}} // namespace sw::universal
