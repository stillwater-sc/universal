#pragma once
// core.hpp: the ereal arithmetic core -- no I/O, no text
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 1 of the ereal headers (#1334, Phase 2 group 5b, #1455). Storage,
// arithmetic, comparison, numeric_limits and traits -- everything a compute kernel
// needs and nothing that turns an ereal into a stream.
//
//     #include <universal/number/ereal/ereal.hpp>   // everything, as before
//     #include <universal/number/ereal/core.hpp>    // arithmetic only
//
// ereal.hpp includes this plus attributes.hpp, manipulators.hpp, iostream.hpp and the
// mathlib, so existing code is unaffected.
//
// parse() and to_string() stay here: parse() backs assign(const std::string&) and the
// string constructor, and to_string() concatenates a std::string. Both are members.
// Only the two stream operators move out. Both also call pown() and to_digits() calls
// ldexp(), so those two mathlib functions come with the core -- see below and
// ereal_impl.hpp.
//
// NOTE: traits/arithmetic_traits.hpp and common/number_traits_reports.hpp are
// deliberately NOT included -- they build report strings and pull <sstream>/<iomanip>.
// The umbrella provides them. So is the mathlib: math/functions/complex.hpp reaches
// <complex>, which pulls <sstream> in libstdc++.
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

#include <universal/traits/number_traits.hpp>

#include <universal/number/ereal/exceptions.hpp>
#include <universal/number/ereal/ereal_fwd.hpp>
#include <universal/number/ereal/ereal_impl.hpp>
#include <universal/traits/ereal_traits.hpp>
#include <universal/number/ereal/numeric_limits.hpp>
// ldexp/frexp and the rest of the <cmath> numeric manipulations: to_digits() scales
// through ldexp. Pure computation on an ereal, so it is core, though it lives in the
// math tree beside the functions that build on it.
#include <universal/number/ereal/math/functions/numerics.hpp>
