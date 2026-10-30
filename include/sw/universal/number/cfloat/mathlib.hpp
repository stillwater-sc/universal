#pragma once
// mathlib.hpp: definition of mathematical functions for the classic cfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <cstdint>       // the fixed-width integer types
#include <universal/number/cfloat/math/functions/classify.hpp>
#include <universal/number/cfloat/math/functions/complex.hpp>
#include <universal/number/cfloat/math/functions/error_and_gamma.hpp>
#include <universal/number/cfloat/math/functions/exponent.hpp>
#include <universal/number/cfloat/math/functions/fractional.hpp>
#include <universal/number/cfloat/math/functions/hyperbolic.hpp>
#include <universal/number/cfloat/math/functions/hypot.hpp>
#include <universal/number/cfloat/math/functions/logarithm.hpp>
#include <universal/number/cfloat/math/functions/minmax.hpp>
#include <universal/number/cfloat/math/functions/next.hpp>
#include <universal/number/cfloat/math/functions/pow.hpp>
#include <universal/number/cfloat/math/functions/sqrt.hpp>
#include <universal/number/cfloat/math/functions/trigonometry.hpp>
#include <universal/number/cfloat/math/functions/truncate.hpp>

namespace sw {
    namespace universal {
        //////////////////////////////////////////////////////////////////////////

        // ipow, an integer power through uint64_t, is retired: pow(a, int) is the integer power (#1875)

        // clang <complex> implementation is calling these functions so we need implementations for posit

        // copysign returns a value with the magnitude of a, and the sign of b
        template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
        inline cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> copysign(const cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>& a, const cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>& b) {
            cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating> c(a);
            if (a.sign() == b.sign()) return c;
            return -c;
        }
    }
}
