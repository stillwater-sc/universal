#pragma once
// mathlib.hpp: definition of mathematical functions specialized for fixpnt arithmetic types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/number/fixpnt/math/classify.hpp>
#include <universal/number/fixpnt/math/complex.hpp>
#include <universal/number/fixpnt/math/error_and_gamma.hpp>
#include <universal/number/fixpnt/math/exponent.hpp>
#include <universal/number/fixpnt/math/fractional.hpp>
#include <universal/number/fixpnt/math/hyperbolic.hpp>
#include <universal/number/fixpnt/math/hypot.hpp>
#include <universal/number/fixpnt/math/logarithm.hpp>
#include <universal/number/fixpnt/math/minmax.hpp>
#include <universal/number/fixpnt/math/next.hpp>
#include <universal/number/fixpnt/math/pow.hpp>
#include <universal/number/fixpnt/math/sqrt.hpp>
#include <universal/number/fixpnt/math/trigonometry.hpp>
#include <universal/number/fixpnt/math/truncate.hpp>

namespace sw {
    namespace universal {
        //////////////////////////////////////////////////////////////////////////

        // calculate the integer power a ^ b
        // exponentiation by squaring is the standard method for modular exponentiation of large numbers in asymmetric cryptography
        template<unsigned nbits, unsigned es, bool isSaturating, typename bt>
        fixpnt<nbits, es, isSaturating, bt> ipow(const fixpnt<nbits, es, isSaturating, bt>& a, const fixpnt<nbits, es, isSaturating, bt>& b) {
            // precondition
            if (!a.isinteger() || !b.isinteger()) return fixpnt<nbits, es, isSaturating, bt>(0);

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
            return fixpnt<nbits, es, isSaturating, bt>(result);
        }

        // clang <complex> implementation is calling isinf functions so we need implementations

        // already defined in math/classify.hpp
		// isnan, isinf, isfinite, isnormal, fpclassify

        // copysign returns a value with the magnitude of a, and the sign of b
        template<unsigned nbits, unsigned es, bool isSaturating, typename bt>
        inline fixpnt<nbits, es, isSaturating, bt> copysign(const fixpnt<nbits, es, isSaturating, bt>& a, const fixpnt<nbits, es, isSaturating, bt>& b) {
            fixpnt<nbits, es, isSaturating, bt> c(a);
            if (a.sign() == b.sign()) return c;
            return -c;
        }
    }
}