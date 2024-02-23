#pragma once
// mathlib.hpp: elementary functions for the takum number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

/*
https://en.wikipedia.org/wiki/Elementary_function

In mathematics, an elementary function is a function of one variable which is a finite sum, 
product, and/or comtakumion of the rational functions (P(x)/Q(x) for polynomials P and Q), 
sin, cos, exp, and their inverse functions (including arcsin, log, x^(1/n)).

Elementary functions were introduced by Joseph Liouville in a series of papers from 1833 to 1841. 
An algebraic treatment of elementary functions was started by Joseph Fels Ritt in the 1930s.
*/
#include <universal/number/takum/math/classify.hpp>
#include <universal/number/takum/math/complex.hpp>
#include <universal/number/takum/math/error_and_gamma.hpp>
#include <universal/number/takum/math/exponent.hpp>
#include <universal/number/takum/math/fractional.hpp>
#include <universal/number/takum/math/hyperbolic.hpp>
#include <universal/number/takum/math/hypot.hpp>
#include <universal/number/takum/math/logarithm.hpp>
#include <universal/number/takum/math/minmax.hpp>
#include <universal/number/takum/math/next.hpp>
#include <universal/number/takum/math/pow.hpp>
#include <universal/number/takum/math/sqrt.hpp>
#include <universal/number/takum/math/trigonometry.hpp>
#include <universal/number/takum/math/truncate.hpp>

namespace sw { namespace universal {
    //////////////////////////////////////////////////////////////////////////

    // calculate the integer power a ^ b
    // exponentiation by squaring is the standard method for modular exponentiation of large numbers in asymmetric cryptography
    template<unsigned nbits, unsigned es, typename BlockType>
    takum<nbits, es, BlockType> ipow(const takum<nbits, es, BlockType>& a, const takum<nbits, es, BlockType>& b) {
        // precondition
        if (!a.isinteger() || !b.isinteger()) return takum<nbits, es, BlockType>(0);

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
        return takum<nbits, BlockType>(result);
    }

    // clang <complex> implementation is calling these functions so we need implementations for takum

    // already defined in math/classify.hpp
    //template<unsigned nbits, unsigned es>
    //inline bool isnan(const takum<nbits, es>& p) { return p.isnar(); }
    //
    //template<unsigned nbits, unsigned es>
    //inline bool isinf(const takum<nbits, es>& p) { return p.isnar(); }

    // copysign returns a value with the magnitude of a, and the sign of b
    template<unsigned nbits, unsigned es, typename BlockType>
    inline takum<nbits, es, BlockType> copysign(const takum<nbits, es, BlockType>& a, const takum<nbits, es, BlockType>& b) {
        takum<nbits, es, BlockType> c(a);
        if (a.sign() == b.sign()) return c;
        return -c;
    }

}}
