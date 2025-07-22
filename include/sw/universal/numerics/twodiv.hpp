#pragma once
// twodiv.hpp: definition of the twoDiv function
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

    /// <summary>
    /// twoDiv generates the ratio and remaining error of a floating-point division operator
    /// </summary>
    /// <typeparam name="Real"></typeparam>
    /// <param name="a">input operand</param>
    /// <param name="b">input operand</param>
    /// <param name="ratio">output ratio of the input operands in requested floating-point precision</param>
    /// <param name="error">output remaining error of the division in requested floating-point precision</param>
    template <typename Real>
    inline void twoDiv(const Real& a, const Real& b, Real& ratio, Real& error) {
        ratio = a / b;
        error = -fma(b, ratio, -a);
    }

}} // namespace sw::universal
