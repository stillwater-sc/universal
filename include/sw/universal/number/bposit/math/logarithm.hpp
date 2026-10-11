#pragma once
// logarithm.hpp: logarithms for the bounded posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Phase 1 (#1653): evaluated in double and rounded once, NOT correctly rounded; see shim.hpp.
//
// The logarithm of 0 is a pole and of a negative value a domain error; both are NaR.
// Every other result is finite and inside the range of every b-posit.
#include <cmath>
#include <universal/number/bposit/math/shim.hpp>

namespace sw { namespace universal {

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> log(const bposit<nbits, rs, es, bt>& x) { return bposit<nbits, rs, es, bt>(std::log(double(x))); }

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> log2(const bposit<nbits, rs, es, bt>& x) { return bposit<nbits, rs, es, bt>(std::log2(double(x))); }

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> log10(const bposit<nbits, rs, es, bt>& x) { return bposit<nbits, rs, es, bt>(std::log10(double(x))); }

// log(1 + x), accurate for small x; NaR at x = -1 and below
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> log1p(const bposit<nbits, rs, es, bt>& x) { return bposit<nbits, rs, es, bt>(std::log1p(double(x))); }

}} // namespace sw::universal
