#pragma once
// takum_fwd.hpp: forward declarations of the takum number system environment
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstddef>  // for unsigned
#include <universal/behavior/arithmetic.hpp> // for ArithmeticBehavior

namespace sw { namespace universal {

// core takum types and functions
template<unsigned nbits, unsigned rbits, typename bt> class takum;
template<unsigned nbits, unsigned rbits, typename bt> constexpr takum<nbits, rbits, bt> abs(const takum<nbits, rbits, bt>&);
template<unsigned nbits, unsigned rbits, typename bt> takum<nbits, rbits, bt> sqrt(const takum<nbits, rbits, bt>&);

}} // namespace sw::universal

