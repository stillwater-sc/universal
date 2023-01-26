#pragma once
// lns2b_fwd.hpp :  forward declarations of the 2-base logarithmic number system environment
//
// Copyright (C) 2022-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstddef>  // for unsigned
#include <universal/behavior/arithmetic.hpp> // for definition of ArithmeticBehavior used in the variadic template continuation

namespace sw { namespace universal {

// core mdlns types and functions
template<unsigned nbits, unsigned fbbits, typename bt, auto... xtra> class lns2b;
template<unsigned nbits, unsigned fbbits, typename bt, auto... xtra> constexpr lns2b<nbits, fbbits, bt, xtra...> abs(const lns2b<nbits, fbbits, bt, xtra...>&);
template<unsigned nbits, unsigned fbbits, typename bt, auto... xtra> lns2b<nbits, fbbits, bt, xtra...> sqrt(const lns2b<nbits, fbbits, bt, xtra...>&);

}} // namespace sw::universal

