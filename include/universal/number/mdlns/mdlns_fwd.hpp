#pragma once
// mdlns_fwd.hpp :  forward declarations of the logarithmic number system environment
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstddef>  // for unsigned
#include <universal/behavior/arithmetic.hpp> // for ArithmeticBehavior

namespace sw { namespace universal {

// core mdlns types and functions
template<unsigned nbits, unsigned rbits, typename bt, auto...x> class mdlns;
template<unsigned nbits, unsigned rbits, typename bt, auto...x> constexpr mdlns<nbits, rbits, bt, x...> abs(const mdlns<nbits, rbits, bt, x...>&);
template<unsigned nbits, unsigned rbits, typename bt, auto...x> mdlns<nbits, rbits, bt, x...> sqrt(const mdlns<nbits, rbits, bt, x...>&);

}} // namespace sw::universal

