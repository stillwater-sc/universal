#pragma once
// mathlib.hpp: definition of mathematical functions for the logarithmic number systems
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/number/lns/math/classify.hpp>
#include <universal/number/lns/math/complex.hpp>
#include <universal/number/lns/math/error_and_gamma.hpp>
#include <universal/number/lns/math/exponent.hpp>
#include <universal/number/lns/math/fractional.hpp>
#include <universal/number/lns/math/hyperbolic.hpp>
#include <universal/number/lns/math/hypot.hpp>
#include <universal/number/lns/math/logarithm.hpp>
#include <universal/number/lns/math/minmax.hpp>
#include <universal/number/lns/math/next.hpp>
#include <universal/number/lns/math/pow.hpp>
#include <universal/number/lns/math/sqrt.hpp>
#include <universal/number/lns/math/trigonometry.hpp>
#include <universal/number/lns/math/truncate.hpp>

namespace sw {
    namespace universal {
        //////////////////////////////////////////////////////////////////////////

        // calculate the integer power a ^ b
        // exponentiation by squaring is the standard method for modular exponentiation of large numbers in asymmetric cryptography
        template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
        lns<nbits, rbits, bt, xtra...> ipow(const lns<nbits, rbits, bt, xtra...>& a, const lns<nbits, rbits, bt, xtra...>& b) {
            // precondition
            if (!a.isinteger() || !b.isinteger()) return lns<nbits, rbits, bt, xtra...>(0);

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
            return lns<nbits, rbits, bt, xtra...>(result);
        }

        // clang <complex> implementation is calling these functions so we need implementations for posit

        // already defined in math/classify.hpp
        //template<unsigned nbits, unsigned es>
        //inline bool isnan(const posit<nbits, es>& p) { return p.isnar(); }
        //
        //template<unsigned nbits, unsigned es>
        //inline bool isinf(const posit<nbits, es>& p) { return p.isnar(); }

        // copysign returns a value with the magnitude of a, and the sign of b
        template<unsigned nbits, unsigned rbits, typename bt, auto... xtra>
        inline lns<nbits, rbits, bt, xtra...> copysign(const lns<nbits, rbits, bt, xtra...>& a, const lns<nbits, rbits, bt, xtra...>& b) {
            lns<nbits, rbits, bt, xtra...> c(a);
            if (a.sign() == b.sign()) return c;
            return -c;
        }
    }
}