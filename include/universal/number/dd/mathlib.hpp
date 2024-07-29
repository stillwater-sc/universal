#pragma once
// mathlib.hpp: definition of mathematical functions for the brain floats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/number/dd/math/classify.hpp>
//#include <universal/number/dd/math/complex.hpp>
//#include <universal/number/dd/math/error_and_gamma.hpp>
//#include <universal/number/dd/math/exponent.hpp>
//#include <universal/number/dd/math/fractional.hpp>
//#include <universal/number/dd/math/hyperbolic.hpp>
//#include <universal/number/dd/math/hypot.hpp>
#include <universal/number/dd/math/logarithm.hpp>
//#include <universal/number/bfloat/math/minmax.hpp>
#include <universal/number/dd/math/next.hpp>
#include <universal/number/dd/math/pow.hpp>
//#include <universal/number/dd/math/sqrt.hpp>
//#include <universal/number/dd/math/trigonometry.hpp>
//#include <universal/number/dd/math/truncate.hpp>

namespace sw {
    namespace universal {
        //////////////////////////////////////////////////////////////////////////

        // clang <complex> implementation is calling these functions so we need implementations for doubledouble (dd)

        // copysign returns a value with the magnitude of a, and the sign of b
        inline dd copysign(dd a, dd b) {
            dd c(a);
            if (a.sign() == b.sign()) return c;
            return -c;
        }

    }
}
