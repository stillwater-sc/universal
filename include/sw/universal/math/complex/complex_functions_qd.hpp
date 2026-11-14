#pragma once
// complex_functions_qd.hpp: complex<qd> computes its transcendentals in qd arithmetic
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Marking qd high precision sends abs, arg and the complex transcendentals of
// <universal/math/complex.hpp> down the path that computes in qd itself (#2026), which keeps
// the ~64 decimal digit precision of qd arithmetic and the C99 Annex G special values.

#include <universal/number/qd/qd.hpp>
#include <universal/math/complex/complex_impl.hpp>

// is_high_precision<qd> is registered by number/qd/mathlib.hpp, which qd.hpp includes (#2060).
