#pragma once
// mathlib.hpp: elementary functions for the oneparam number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

/*
https://en.wikipedia.org/wiki/Elementary_function

In mathematics, an elementary function is a function of one variable which is a finite sum, 
product, and/or comoneparamion of the rational functions (P(x)/Q(x) for polynomials P and Q), 
sin, cos, exp, and their inverse functions (including arcsin, log, x^(1/n)).

Elementary functions were introduced by Joseph Liouville in a series of papers from 1833 to 1841. 
An algebraic treatment of elementary functions was started by Joseph Fels Ritt in the 1930s.
*/
#include <universal/number/oneparam/math/classify.hpp>
#include <universal/number/oneparam/math/complex.hpp>
#include <universal/number/oneparam/math/error_and_gamma.hpp>
#include <universal/number/oneparam/math/exponent.hpp>
#include <universal/number/oneparam/math/fractional.hpp>
#include <universal/number/oneparam/math/hyperbolic.hpp>
#include <universal/number/oneparam/math/hypot.hpp>
#include <universal/number/oneparam/math/logarithm.hpp>
#include <universal/number/oneparam/math/minmax.hpp>
#include <universal/number/oneparam/math/next.hpp>
#include <universal/number/oneparam/math/pow.hpp>
#include <universal/number/oneparam/math/sqrt.hpp>
#include <universal/number/oneparam/math/trigonometry.hpp>
#include <universal/number/oneparam/math/truncate.hpp>

namespace sw { namespace universal {
    //////////////////////////////////////////////////////////////////////////

    // calculate the integer power a ^ b
    // exponentiation by squaring is the standard method for modular exponentiation of large numbers in asymmetric cryptography
    template<unsigned nbits, typename BlockType>
    oneparam<nbits, BlockType> ipow(const oneparam<nbits, BlockType>& a, const oneparam<nbits, BlockType>& b) {
        // precondition
        if (!a.isinteger() || !b.isinteger()) return oneparam<nbits,  BlockType>(0);

        // TODO: using uint64_t as ipow constraints dynamic range
        uint64_t result(1);
        uint64_t base = uint64_t(a); 
        uint64_t exp = uint64_t(b);
        for (;;) {
            if (exp & 0x1) result *= base;
            exp >>= 1;
            if (exp == 0) break;
            base *= base;
        }
        return oneparam<nbits, BlockType>(result);
    }

    // clang <complex> implementation is calling these functions so we need implementations for oneparam

    // already defined in math/classify.hpp
    //template<unsigned nbits, unsigned es>
    //inline bool isnan(const oneparam<nbits, es>& p) { return p.isnar(); }
    //
    //template<unsigned nbits, unsigned es>
    //inline bool isinf(const oneparam<nbits, es>& p) { return p.isnar(); }

    // copysign returns a value with the magnitude of a, and the sign of b
    template<unsigned nbits, typename BlockType>
    inline oneparam<nbits, BlockType> copysign(const oneparam<nbits, BlockType>& a, const oneparam<nbits, BlockType>& b) {
        oneparam<nbits, BlockType> c(a);
        if (a.sign() == b.sign()) return c;
        return -c;
    }
}}
