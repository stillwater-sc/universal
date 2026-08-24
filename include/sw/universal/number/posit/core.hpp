#pragma once
// core.hpp: the posit arithmetic core -- no I/O, no introspection
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 1 of the posit headers (#1334). Storage, arithmetic, comparison,
// numeric_limits and traits -- everything a compute kernel needs and nothing
// that turns a posit into text.
//
//     #include <universal/number/posit/posit.hpp>   // everything, as before
//     #include <universal/number/posit/core.hpp>    // arithmetic only
//
// posit.hpp includes this header plus posit_io.hpp and posit_debug.hpp, so
// existing code is unaffected. Reach for core.hpp in a translation unit that
// only computes: it does not pull <iostream>, <sstream> or <iomanip>.
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

#include <universal/number/posit/exceptions.hpp>
#include <universal/number/posit/posit_fwd.hpp>
#include <universal/number/posit/posit_scale_helpers.hpp>
#include <universal/number/posit/posit_impl.hpp>
#include <universal/traits/posit_traits.hpp>
#include <universal/number/posit/numeric_limits.hpp>
