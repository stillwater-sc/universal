#pragma once
// cbrt.hpp: cube root for quad-double cascade (qd_cascade) floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

    // Cube root function - stub implementation using double approximation
    // TODO: Implement high-precision Newton iteration for quad-double
    inline qd_cascade cbrt(const qd_cascade& a) {
        return qd_cascade(std::cbrt(a[0]));
    }

}} // namespace sw::universal
