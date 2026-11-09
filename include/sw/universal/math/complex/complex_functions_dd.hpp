#pragma once
// complex_functions_dd.hpp: complex<dd> computes its transcendentals in dd arithmetic
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Marking dd high precision sends abs, arg and the complex transcendentals of
// <universal/math/complex.hpp> down the path that computes in dd itself (#2026), which keeps
// the ~32 decimal digit precision of dd arithmetic and the C99 Annex G special values.

#include <universal/number/dd/dd.hpp>
#include <universal/math/complex/complex_impl.hpp>

namespace sw { namespace universal {

// Mark dd as a high-precision type
template<>
struct is_high_precision<dd> : std::true_type {};

}} // namespace sw::universal
