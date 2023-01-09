#pragma once
// mdlns_fwd.hpp :  forward declarations of the logarithmic number system environment
//
// Copyright (C) 2022-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstddef>  // for unsigned
#include <universal/behavior/arithmetic.hpp> // for definition of ArithmeticBehavior used in the variadic template continuation

namespace sw { namespace universal {

// core mdlns types and functions
template<unsigned nbits, unsigned rbits, unsigned firstBase, unsigned secondBase, typename bt, auto...x> class mdlns;
template<unsigned nbits, unsigned rbits, unsigned firstBase, unsigned secondBase, typename bt, auto...x> constexpr mdlns<nbits, rbits, firstBase, secondBase, bt, x...> abs(const mdlns<nbits, rbits, firstBase, secondBase, bt, x...>&);
template<unsigned nbits, unsigned rbits, unsigned firstBase, unsigned secondBase, typename bt, auto...x> mdlns<nbits, rbits, firstBase, secondBase, bt, x...> sqrt(const mdlns<nbits, rbits, firstBase, secondBase, bt, x...>&);

}} // namespace sw::universal

