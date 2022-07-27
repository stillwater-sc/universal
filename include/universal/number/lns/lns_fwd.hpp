#pragma once
// lns_fwd.hpp :  forward declarations of the logarithmic number system environment
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstddef>  // for size_t
#include <universal/behavior/arithmetic.hpp> // for ArithmeticBehavior

namespace sw { namespace universal {

// core lns types
template<size_t nbits, size_t rbits, ArithmeticBehavior behavior, typename bt> class lns;
template<size_t nbits, size_t rbits, ArithmeticBehavior behavior, typename bt> lns<nbits, rbits, behavior, bt> abs(const lns<nbits, rbits, behavior, bt>&);
template<size_t nbits, size_t rbits, ArithmeticBehavior behavior, typename bt> lns<nbits, rbits, behavior, bt> sqrt(const lns<nbits, rbits, behavior, bt>&);

}} // namespace sw::universal

