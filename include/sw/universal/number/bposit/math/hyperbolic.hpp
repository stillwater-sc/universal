#pragma once
// hyperbolic.hpp: hyperbolic functions for the bounded posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Phase 1 (#1653): evaluated in double and rounded once, NOT correctly rounded; see shim.hpp.
//
// sinh and cosh overflow double for large arguments, and saturate.  acosh is NaR below
// 1, and atanh is NaR outside (-1, 1), with poles at +/-1.
#include <cmath>
#include <universal/number/bposit/math/shim.hpp>

namespace sw { namespace universal {

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> sinh(const bposit<nbits, rs, es, bt>& x) { return bposit_shim::finite<bposit<nbits, rs, es, bt>>(std::sinh(double(x))); }

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> cosh(const bposit<nbits, rs, es, bt>& x) { return bposit_shim::finite<bposit<nbits, rs, es, bt>>(std::cosh(double(x))); }

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> tanh(const bposit<nbits, rs, es, bt>& x) { return bposit<nbits, rs, es, bt>(std::tanh(double(x))); }

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> asinh(const bposit<nbits, rs, es, bt>& x) { return bposit<nbits, rs, es, bt>(std::asinh(double(x))); }

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> acosh(const bposit<nbits, rs, es, bt>& x) { return bposit<nbits, rs, es, bt>(std::acosh(double(x))); }

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> atanh(const bposit<nbits, rs, es, bt>& x) { return bposit<nbits, rs, es, bt>(std::atanh(double(x))); }

}} // namespace sw::universal
