// exceptions.hpp: exception types for the McCleeary LFPERA elreal number system.
//
// elreal is an experimental, header-only research type. Its streaming
// operations already surface non-convergence bugs by throwing std::runtime_error
// (see threeAdd.hpp). Phase 5 (#929) adds elreal_sum_budget_exceeded (a series
// handed to sum() that does not Cauchy-stabilise within the depth budget).
// Phase 6 (#930) adds elreal_divide_by_zero for div() with a zero divisor.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

#include <stdexcept>
#include <string>

// Arithmetic-exception guard, consistent with the other Universal number
// systems. When 0 (default), div() by zero returns the empty co-list (the
// undefined/zero result) instead of throwing; define it to 1 to opt in to
// throwing elreal_divide_by_zero.
#if !defined(ELREAL_THROW_ARITHMETIC_EXCEPTION)
#define ELREAL_THROW_ARITHMETIC_EXCEPTION 0
#endif

namespace sw { namespace universal {

// Base class for elreal exceptions, so callers can catch the whole family.
struct elreal_exception : public std::runtime_error {
    explicit elreal_exception(const std::string& what) : std::runtime_error(what) {}
};

// Thrown by sum() when the depth budget is exhausted before the series'
// partial-sum sequence shows convergence (term magnitudes not trending toward
// zero). The dissertation defines summation only for Cauchy series; this guards
// against ill-formed (divergent) input. A convergent-but-slow series does NOT
// trigger this: it is truncated at the budget and the partial sum is returned.
struct elreal_sum_budget_exceeded : public elreal_exception {
    explicit elreal_sum_budget_exceeded(const std::string& what) : elreal_exception(what) {}
};

// Thrown by div() when the divisor is the empty co-list (zero), and only when
// ELREAL_THROW_ARITHMETIC_EXCEPTION is enabled.
struct elreal_divide_by_zero : public elreal_exception {
    explicit elreal_divide_by_zero(const std::string& what = "elreal: division by zero")
        : elreal_exception(what) {}
};

}} // namespace sw::universal
