#pragma once
// core.hpp: the hfloat arithmetic core -- no I/O, no text
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 1 of the hfloat headers (#1334, Phase 2 group 5a, #1444). Storage, arithmetic,
// comparison, numeric_limits and traits -- everything a compute kernel needs and
// nothing that turns an hfloat into text.
//
//     #include <universal/number/hfloat/hfloat.hpp>   // everything, as before
//     #include <universal/number/hfloat/core.hpp>     // arithmetic only
//
// hfloat.hpp includes this plus manipulators.hpp, iostream.hpp and attributes.hpp, so
// existing code is unaffected.
//
// WHAT STAYS HERE, and why:
//   - str() is core surface: operator<< formats through it. It used to build its result
//     with a stringstream, which was the single reason this core would have needed
//     <sstream>; it now reaches the same formatter through snprintf("%.*g"). See the
//     note on str() in hfloat_impl.hpp.
//   - parse() converts through utility/decimal_to_binary.hpp, which is already at zero
//     I/O-family headers, so it is arithmetic surface rather than text -- the same call
//     dfloat's parse() got (#1446), and the opposite of cfloat's and bfloat16's, whose
//     parse() runs an istringstream.
//
// native/ieee754.hpp was re-pointed to native/ieee754_core.hpp, its stream-free half.
//
// NOTE: traits/arithmetic_traits.hpp and common/number_traits_reports.hpp are
// deliberately NOT included -- they build report strings and pull <sstream>/<iomanip>.
// The umbrella provides them.
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

#include <universal/traits/number_traits.hpp>

#include <universal/number/hfloat/exceptions.hpp>
#include <universal/number/hfloat/hfloat_fwd.hpp>
#include <universal/number/hfloat/hfloat_impl.hpp>
#include <universal/traits/hfloat_traits.hpp>
#include <universal/number/hfloat/numeric_limits.hpp>
