// exceptions.hpp: exception types for the McCleeary LFPERA elreal number system.
//
// elreal is an experimental, header-only research type. Its streaming
// operations already surface non-convergence bugs by throwing std::runtime_error
// (see threeAdd.hpp). Phase 5 (#929) adds one caller-facing exception: a series
// handed to sum() that does not Cauchy-stabilise within the depth budget.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

#include <stdexcept>
#include <string>

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

}} // namespace sw::universal
