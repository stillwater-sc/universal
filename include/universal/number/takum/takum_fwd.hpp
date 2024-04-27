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
template<unsigned nbits, typename bt> class takum;
template<unsigned nbits, typename bt> takum<nbits, bt> abs(const takum<nbits, bt>&);
template<unsigned nbits, typename bt> takum<nbits, bt> sqrt(const takum<nbits, bt>&);

}} // namespace sw::universal

