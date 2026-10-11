#pragma once
// truncate.hpp: truncation functions (floor, ceil, trunc, round) for the bounded posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The integral result of a b-posit is itself a b-posit: it lies in the argument's
// binade or a lower one, or is a power of two, so these round nothing when the
// argument converts exactly to double (see shim.hpp).
#include <cmath>

namespace sw { namespace universal {

// the largest integral value not greater than x
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> floor(const bposit<nbits, rs, es, bt>& x) { return bposit<nbits, rs, es, bt>(std::floor(double(x))); }

// the smallest integral value not less than x
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> ceil(const bposit<nbits, rs, es, bt>& x) { return bposit<nbits, rs, es, bt>(std::ceil(double(x))); }

// the nearest integral value not larger in magnitude than x
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> trunc(const bposit<nbits, rs, es, bt>& x) { return bposit<nbits, rs, es, bt>(std::trunc(double(x))); }

// the nearest integral value, halfway cases away from zero
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
bposit<nbits, rs, es, bt> round(const bposit<nbits, rs, es, bt>& x) { return bposit<nbits, rs, es, bt>(std::round(double(x))); }

}} // namespace sw::universal
