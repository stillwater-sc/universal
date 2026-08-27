#pragma once
// core.hpp: the lns arithmetic core -- no I/O, no text
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 1 of the lns headers (#1334, Phase 2). Storage, arithmetic, comparison,
// numeric_limits and traits -- everything a compute kernel needs and nothing that
// turns an lns into text.
//
//     #include <universal/number/lns/lns.hpp>    // everything, as before
//     #include <universal/number/lns/core.hpp>   // arithmetic only
//
// lns.hpp includes this plus manipulators.hpp, iostream.hpp and debug.hpp, so existing
// code is unaffected.
//
// NOTE: traits/arithmetic_traits.hpp and common/number_traits_reports.hpp are
// deliberately NOT included -- they build report strings and pull <sstream>/<iomanip>.
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

#include <universal/traits/number_traits.hpp>

#include <universal/number/lns/exceptions.hpp>
#include <universal/number/lns/lns_fwd.hpp>
#include <universal/number/lns/lns_impl.hpp>
#include <universal/number/lns/lns_traits.hpp>
#include <universal/number/lns/numeric_limits.hpp>
