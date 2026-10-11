#pragma once
// sqrt.hpp: square and cube roots for the bounded posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Phase 1 (#1653): evaluated in double and rounded once, NOT correctly rounded; see shim.hpp.
#include <cmath>
#include <universal/number/bposit/math/shim.hpp>

namespace sw { namespace universal {

// the square root; NaR for a negative argument
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> sqrt(const bposit<nbits, rs, es, bt>& x) { return bposit<nbits, rs, es, bt>(std::sqrt(double(x))); }

// the cube root, defined for every real
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> cbrt(const bposit<nbits, rs, es, bt>& x) { return bposit<nbits, rs, es, bt>(std::cbrt(double(x))); }

}} // namespace sw::universal
