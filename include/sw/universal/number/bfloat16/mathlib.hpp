#pragma once
// mathlib.hpp: definition of mathematical functions for the Google Brain floats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>
#include <universal/number/bfloat16/math/functions/classify.hpp>
//#include <universal/number/bfloat16/math/functions/complex.hpp>
#include <universal/number/bfloat16/math/functions/error_and_gamma.hpp>
#include <universal/number/bfloat16/math/functions/exponent.hpp>
#include <universal/number/bfloat16/math/functions/fma.hpp>
#include <universal/number/bfloat16/math/functions/fractional.hpp>
#include <universal/number/bfloat16/math/functions/hyperbolic.hpp>
#include <universal/number/bfloat16/math/functions/hypot.hpp>
#include <universal/number/bfloat16/math/functions/logarithm.hpp>
#include <universal/number/bfloat16/math/functions/minmax.hpp>
#include <universal/number/bfloat16/math/functions/next.hpp>
#include <universal/number/bfloat16/math/functions/pow.hpp>
#include <universal/number/bfloat16/math/functions/sqrt.hpp>
#include <universal/number/bfloat16/math/functions/trigonometry.hpp>
#include <universal/number/bfloat16/math/functions/truncate.hpp>

namespace sw {
    namespace universal {
        //////////////////////////////////////////////////////////////////////////

        // ipow, an integer power through float, is retired: pow(a, int) is the integer power (#1875)

        // clang <complex> implementation is calling these functions so we need implementations for bfloat16

        // copysign returns a value with the magnitude of a, and the sign of b
        inline bfloat16 copysign(bfloat16 a, bfloat16 b) {
            bfloat16 c(a);
            if (a.sign() == b.sign()) return c;
            return -c;
        }

    }
}
