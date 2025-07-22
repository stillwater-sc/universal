#pragma once
// lns_fwd.hpp :  forward declarations of the logarithmic number system environment
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstddef>  // for unsigned
#include <universal/behavior/arithmetic.hpp> // for ArithmeticBehavior

namespace sw { namespace universal {

// core lns types and functions
template<unsigned nbits, unsigned rbits, typename bt, auto...x> class lns;
template<unsigned nbits, unsigned rbits, typename bt, auto...x> constexpr lns<nbits, rbits, bt, x...> abs(const lns<nbits, rbits, bt, x...>&);
template<unsigned nbits, unsigned rbits, typename bt, auto...x> lns<nbits, rbits, bt, x...> sqrt(const lns<nbits, rbits, bt, x...>&);

}} // namespace sw::universal

