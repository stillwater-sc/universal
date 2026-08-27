#pragma once
// core.hpp: the rational arithmetic core -- no streams
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 1 of the rational headers (#1334, Phase 2).
//
//     #include <universal/number/rational/rational.hpp>   // everything, as before
//     #include <universal/number/rational/core.hpp>       // arithmetic only
//
// WHAT "core" EXCLUDES, precisely: the stream family -- <iostream>, <sstream>,
// <iomanip>, <ostream>, <istream>. Measured: this header pulls zero of the five.
//
// rational_impl.hpp had included internal/blockbinary/manipulators.hpp, which pulls four
// of those five. Its only consumer was to_binary(rational<>), which is a manipulator, so
// that include moved to manipulators.hpp with the function.
//
// NOTE: traits/arithmetic_traits.hpp and common/number_traits_reports.hpp are
// deliberately NOT included -- they build report strings and pull <sstream>/<iomanip>.
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

#include <universal/traits/number_traits.hpp>

#include <universal/number/rational/exceptions.hpp>
#include <universal/number/rational/rational_fwd.hpp>
#include <universal/number/rational/rational_impl.hpp>
#include <universal/traits/rational_traits.hpp>
#include <universal/number/rational/numeric_limits.hpp>
