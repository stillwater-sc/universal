#pragma once
// takum_fwd.hpp :  forward declarations of the takum number system environment
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstddef>  // for unsigned
#include <universal/behavior/arithmetic.hpp> // for ArithmeticBehavior

namespace sw { namespace universal {

// core takum types and functions
template<unsigned nbits, unsigned es, typename bt> class takum;
template<unsigned nbits, unsigned es, typename bt> takum<nbits, es, bt> abs(const takum<nbits, es, bt>&);
template<unsigned nbits, unsigned es, typename bt> takum<nbits, es, bt> sqrt(const takum<nbits, es, bt>&);

}} // namespace sw::universal

