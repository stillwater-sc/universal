#pragma once
// mathlib.hpp: definition of mathematical functions for the brain floats
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

//#include <universal/number/bfloat/math/classify.hpp>
//#include <universal/number/bfloat/math/complex.hpp>
//#include <universal/number/bfloat/math/error_and_gamma.hpp>
//#include <universal/number/bfloat/math/exponent.hpp>
//#include <universal/number/bfloat/math/fractional.hpp>
//#include <universal/number/bfloat/math/hyperbolic.hpp>
//#include <universal/number/bfloat/math/hypot.hpp>
//#include <universal/number/bfloat/math/logarithm.hpp>
//#include <universal/number/bfloat/math/minmax.hpp>
#include <universal/number/bfloat/math/next.hpp>
#include <universal/number/bfloat/math/pow.hpp>
//#include <universal/number/bfloat/math/sqrt.hpp>
//#include <universal/number/bfloat/math/trigonometry.hpp>
//#include <universal/number/bfloat/math/truncate.hpp>

namespace sw {
    namespace universal {
        //////////////////////////////////////////////////////////////////////////

/* TODO
        // calculate the integer power a ^ b
        // exponentiation by squaring is the standard method for modular exponentiation of large numbers in asymmetric cryptography
        bfloat16 ipow(bfloat16 a, bfloat16 b) {
            // precondition
            if (!a.isinteger() || !b.isinteger()) return bfloat16(0);

            // TODO: using uint64_t as ipow constrains dynamic range
            uint64_t result(1);
            uint64_t base = uint64_t(a);
            uint64_t exp = uint64_t(b);
            for (;;) {
                if (exp & 0x1) result *= base;
                exp >>= 1;
                if (exp == 0) break;
                base *= base;
            }
            return bfloat16(result);
        }
*/

        // clang <complex> implementation is calling these functions so we need implementations for bfloat

        // copysign returns a value with the magnitude of a, and the sign of b
        inline bfloat16 copysign(bfloat16 a, bfloat16 b) {
            bfloat16 c(a);
            if (a.sign() == b.sign()) return c;
            return -c;
        }

    }
}
