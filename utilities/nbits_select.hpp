// nbits_select.hpp: select nbits value at run-time
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#pragma once

#include <type_traits>
#include <stdexcept>

#include <boost/variant.hpp>

template <std::size_t ES>
using nbits_tag = std::integral_constant<std::size_t, ES>;

/// All possible variants of nbits valunbits.
using nbits_variant = boost::variant<nbits_tag<4>, nbits_tag<8>, nbits_tag<16>>;

struct undefined_nbits_variant
  : std::runtime_error
{
    undefined_nbits_variant() : std::runtime_error("Undefined nbits-variant") {}
};

/// Return the according variant or throw an exception.
//  Inline for not needing cpp file.
inline nbits_variant nbits_select(size_t nbits)
{
    switch (nbits) {
        case 4: return nbits_tag<4>{};
        case 8: return nbits_tag<8>{};
        case 16: return nbits_tag<16>{};
        default: throw undefined_nbits_variant{}; 
    }
    
}
