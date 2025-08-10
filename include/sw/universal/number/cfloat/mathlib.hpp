#pragma once
// mathlib.hpp: definition of mathematical functions for the classic cfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/number/cfloat/math/classify.hpp>
#include <universal/number/cfloat/math/complex.hpp>
#include <universal/number/cfloat/math/error_and_gamma.hpp>
#include <universal/number/cfloat/math/exponent.hpp>
#include <universal/number/cfloat/math/fractional.hpp>
#include <universal/number/cfloat/math/hyperbolic.hpp>
#include <universal/number/cfloat/math/hypot.hpp>
#include <universal/number/cfloat/math/logarithm.hpp>
#include <universal/number/cfloat/math/minmax.hpp>
#include <universal/number/cfloat/math/next.hpp>
#include <universal/number/cfloat/math/pow.hpp>
#include <universal/number/cfloat/math/sqrt.hpp>
#include <universal/number/cfloat/math/trigonometry.hpp>
#include <universal/number/cfloat/math/truncate.hpp>

namespace sw {
    namespace universal {
        //////////////////////////////////////////////////////////////////////////

        // calculate the integer power a ^ b
        // exponentiation by squaring is the standard method for modular exponentiation of large numbers in asymmetric cryptography
        template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
        cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> ipow(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& a, const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& b) {
            // precondition
            if (!a.isinteger() || !b.isinteger()) return cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>(0);

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
            return cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>(result);
        }

        // clang <complex> implementation is calling these functions so we need implementations for posit

        // copysign returns a value with the magnitude of a, and the sign of b
        template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
        inline cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> copysign(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& a, const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>& b) {
            cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> c(a);
            if (a.sign() == b.sign()) return c;
            return -c;
        }
    }
}