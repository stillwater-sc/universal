#pragma once
// mathlib.hpp: elementary functions for the posit number system
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

/*
https://en.wikipedia.org/wiki/Elementary_function

In mathematics, an elementary function is a function of one variable which is a finite sum,
product, and/or composition of the rational functions (P(x)/Q(x) for polynomials P and Q),
sin, cos, exp, and their inverse functions (including arcsin, log, x^(1/n)).

Elementary functions were introduced by Joseph Liouville in a series of papers from 1833 to 1841.
An algebraic treatment of elementary functions was started by Joseph Fels Ritt in the 1930s.
*/

#include <universal/number/posit/math/classify.hpp>
#include <universal/number/posit/math/complex.hpp>
#include <universal/number/posit/math/error_and_gamma.hpp>
#include <universal/number/posit/math/exponent.hpp>
#include <universal/number/posit/math/fractional.hpp>
#include <universal/number/posit/math/hyperbolic.hpp>
#include <universal/number/posit/math/hypot.hpp>
#include <universal/number/posit/math/logarithm.hpp>
#include <universal/number/posit/math/minmax.hpp>
#include <universal/number/posit/math/next.hpp>
#include <universal/number/posit/math/pow.hpp>
#include <universal/number/posit/math/sqrt.hpp>
#include <universal/number/posit/math/trigonometry.hpp>
#include <universal/number/posit/math/truncate.hpp>

namespace sw { namespace universal {
    //////////////////////////////////////////////////////////////////////////

    // calculate the integer power a ^ b
    // exponentiation by squaring is the standard method for modular exponentiation of large numbers in asymmetric cryptography
    template<unsigned nbits, unsigned es, typename bt>
    posit<nbits, es, bt> ipow(const posit<nbits, es, bt>& a, const posit<nbits, es, bt>& b) {
        // precondition
        if (!a.isinteger() || !b.isinteger()) return posit<nbits, es, bt>(0);

        uint64_t result(1);
        uint64_t base = static_cast<uint64_t>(static_cast<long long>(a));
        uint64_t exp = static_cast<uint64_t>(static_cast<long long>(b));
        for (;;) {
            if (exp & 0x1) result *= base;
            exp >>= 1;
            if (exp == 0) break;
            base *= base;
        }
        return posit<nbits,es, bt>(result);
    }

    // clang <complex> implementation is calling these functions so we need implementations for posit

    // already defined in math/classify.hpp
    //template<unsigned nbits, unsigned es>
    //inline bool isnan(const posit<nbits, es>& p) { return p.isnar(); }
    //
    //template<unsigned nbits, unsigned es>
    //inline bool isinf(const posit<nbits, es>& p) { return p.isnar(); }

    // copysign returns a value with the magnitude of a, and the sign of b
    template<unsigned nbits, unsigned es, typename bt>
    inline posit<nbits, es, bt> copysign(const posit<nbits, es, bt>& a, const posit<nbits, es, bt>& b) {
        posit<nbits, es, bt> c(a);
        if (a.sign() == b.sign()) return c;
        return -c;
    }

}}
