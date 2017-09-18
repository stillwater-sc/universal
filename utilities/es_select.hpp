// es_select.hpp: select es value at run-time
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#pragma once

#include <type_traits>
#include <stdexcept>

#include <boost/variant.hpp>

template <std::size_t ES>
using es_tag = std::integral_constant<std::size_t, ES>;

/// All possible variants of es values.
using es_variant = boost::variant<es_tag<1>, es_tag<2>, es_tag<4>>;

struct undefined_es_variant
  : std::runtime_error
{
    undefined_es_variant() : std::runtime_error("Undefined es-variant") {}
};

/// Return the according variant or throw an exception.
//  Inline for not needing cpp file.
inline es_variant es_select(size_t es)
{
    switch (es) {
        case 1: return es_tag<1>{};
        case 2: return es_tag<2>{};
        case 4: return es_tag<4>{};
        default: throw undefined_es_variant{}; 
    }
    
}
