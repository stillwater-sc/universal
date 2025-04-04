#pragma once
// twoprod.hpp: definition of the twoProd function
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

    // NOTE proof for rounding toward zero in "Error-Free Transformation in Rounding Mode toward Zero"
    // WARNING proven only for rounding to nearest and toward zero

    // twoProd error free transform for a multiplication

    /// <summary>
    /// twoProd generates the product and the remainder (error) of a floating-point multiplication operator
    /// </summary>
    /// <typeparam name="Real"></typeparam>
    /// <param name="a">input operand</param>
    /// <param name="b">input operand</param>
    /// <param name="product">output resulting product rounded in parameterized floating-point precision</param>
    /// <param name="error">output error in parameterized floating-point precision</param>
    template <typename Real>
    inline void twoProd(const Real& a, const Real& b, Real& product, Real& error) {
        product = a * b;
        error = fma(a, b, -product);
    }

}} // namespace sw::universal
