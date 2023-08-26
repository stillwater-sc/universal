#pragma once
// dbns_fwd.hpp :  forward declarations of the double base number system environment
//
// Copyright (C) 2022-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstddef>  // for unsigned
#include <universal/behavior/arithmetic.hpp> // for definition of ArithmeticBehavior used in the variadic template continuation

namespace sw { namespace universal {

// core dbns types and functions
template<unsigned nbits, unsigned fbbits, typename bt, auto... xtra> class dbns;
template<unsigned nbits, unsigned fbbits, typename bt, auto... xtra> constexpr dbns<nbits, fbbits, bt, xtra...> abs(const dbns<nbits, fbbits, bt, xtra...>&);
template<unsigned nbits, unsigned fbbits, typename bt, auto... xtra> dbns<nbits, fbbits, bt, xtra...> sqrt(const dbns<nbits, fbbits, bt, xtra...>&);

}} // namespace sw::universal

