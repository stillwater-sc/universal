#pragma once
// manipulators.hpp: type identification, rendering, and stream I/O for elreal.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include <universal/number/elreal/elreal_fwd.hpp>
#include <universal/number/elreal/elreal_impl.hpp>
#include <universal/number/elreal/block_manipulators.hpp>   // to_binary(block) / to_hex(block)
#include <universal/traits/elreal_traits.hpp>

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
template <typename FpType>
inline std::string to_components(const elreal<FpType>& v) {
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
    std::stringstream s;
    auto blocks = v.limbs(v.precision());
    if (blocks.empty()) { s << "0"; return s.str(); }
    s << to_binary(blocks.front(), nibbleMarker);
    if (blocks.size() > 1) s << " ... " << to_binary(blocks.back(), nibbleMarker);
    return s.str();
}

// to_triple: (sign, scale, significand) of the value.
template <typename FpType>
inline std::string to_triple(const elreal<FpType>& v) {
    std::stringstream s;
    s << (v.isneg() ? "(-, " : "(+, ") << v.scale() << ", "
      << std::setprecision(17) << static_cast<double>(v) << ')';
    return s.str();
}

// stream output: the value at its current precision as a host-double approximation.
// (A full high-precision decimal printer is tracked as later manipulators work.)
template <typename FpType>
inline std::ostream& operator<<(std::ostream& ostr, const elreal<FpType>& v) {
    return ostr << static_cast<double>(v);
}

// stream input: parse a host-double literal into an elreal (exact for values a
// double represents exactly; otherwise the nearest double).
template <typename FpType>
inline std::istream& operator>>(std::istream& istr, elreal<FpType>& v) {
    double d{};
    istr >> d;
    if (!istr.fail()) v = d;
    return istr;
}

}} // namespace sw::universal
