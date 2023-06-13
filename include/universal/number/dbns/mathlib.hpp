#pragma once
// mathlib.hpp: definition of mathematical functions for the double base number systems
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/number/dbns/math/classify.hpp>
#include <universal/number/dbns/math/complex.hpp>
#include <universal/number/dbns/math/error_and_gamma.hpp>
#include <universal/number/dbns/math/exponent.hpp>
#include <universal/number/dbns/math/fractional.hpp>
#include <universal/number/dbns/math/hyperbolic.hpp>
#include <universal/number/dbns/math/hypot.hpp>
#include <universal/number/dbns/math/logarithm.hpp>
#include <universal/number/dbns/math/minmax.hpp>
#include <universal/number/dbns/math/next.hpp>
#include <universal/number/dbns/math/pow.hpp>
#include <universal/number/dbns/math/sqrt.hpp>
#include <universal/number/dbns/math/trigonometry.hpp>
#include <universal/number/dbns/math/truncate.hpp>

namespace sw {
    namespace universal {
        //////////////////////////////////////////////////////////////////////////

        // calculate the integer power a ^ b
        // exponentiation by squaring is the standard method for modular exponentiation of large numbers in asymmetric cryptography
        template<unsigned nbits, unsigned fbbits, typename bt, auto... xtra>
        dbns<nbits, fbbits, bt, xtra...> ipow(const dbns<nbits, fbbits, bt, xtra...>& a, int b) {
            // precondition
            if (!a.isinteger()) return dbns<nbits, fbbits, bt, xtra...>(0);

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
            return dbns<nbits, fbbits, bt, xtra...>(result);
        }

        // clang <complex> implementation is calling these functions so we need implementations for posit

        // already defined in math/classify.hpp
        //template<unsigned nbits, unsigned es>
        //inline bool isnan(const posit<nbits, es>& p) { return p.isnar(); }
        //
        //template<unsigned nbits, unsigned es>
        //inline bool isinf(const posit<nbits, es>& p) { return p.isnar(); }

        // copysign returns a value with the magnitude of a, and the sign of b
        template<unsigned nbits, unsigned fbbits, typename bt, auto... xtra>
        inline dbns<nbits, fbbits, bt, xtra...> copysign(const dbns<nbits, fbbits, bt, xtra...>& a, const dbns<nbits, fbbits, bt, xtra...>& b) {
            dbns<nbits, fbbits, bt, xtra...> c(a);
            if (a.sign() == b.sign()) return c;
            return -c;
        }
    }
}
