#pragma once
// core.hpp: the takum arithmetic core -- no I/O, no text
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 1 of the takum headers (#1334, Phase 2). Covers BOTH takum and takum_log: the
// takum.hpp umbrella has always included both impl headers, so they share a core.
//
//     #include <universal/number/takum/takum.hpp>   // everything, as before
//     #include <universal/number/takum/core.hpp>    // arithmetic only
//
// NOTE: traits/arithmetic_traits.hpp and common/number_traits_reports.hpp are
// deliberately NOT included -- they build report strings and pull <sstream>/<iomanip>.
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

#include <universal/traits/number_traits.hpp>

#include <universal/number/takum/exceptions.hpp>
#include <universal/number/takum/takum_fwd.hpp>
#include <universal/number/takum/takum_impl.hpp>
#include <universal/number/takum/takum_log_impl.hpp>
#include <universal/number/takum/takum_traits.hpp>
#include <universal/number/takum/numeric_limits.hpp>
