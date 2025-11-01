#pragma once
// cbrt.hpp: cube root for triple-double cascade (td_cascade) floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

    // Cube root function - stub implementation using double approximation
    // TODO: Implement high-precision Newton iteration for triple-double
    inline td_cascade cbrt(const td_cascade& a) {
        return td_cascade(std::cbrt(a[0]));
    }

}} // namespace sw::universal
