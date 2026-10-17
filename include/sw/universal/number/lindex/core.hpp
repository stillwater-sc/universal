#pragma once
// core.hpp: the lindex core -- no I/O, no text
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 1 of the lindex headers (#1334): storage, conversion, comparison, numeric_limits
// and traits -- everything a compute kernel needs and nothing that turns a lindex into text.
//
//     #include <universal/number/lindex/lindex.hpp>   // everything
//     #include <universal/number/lindex/core.hpp>     // no I/O
//
// The working precision of conversion is dd_cascade, whose core and exp/log are I/O-free.
//
// NOTE: traits/arithmetic_traits.hpp and common/number_traits_reports.hpp are
// deliberately NOT included -- they build report strings and pull <sstream>/<iomanip>.
// The umbrella provides them.
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/directives.hpp>

#include <universal/traits/number_traits.hpp>

#include <universal/number/lindex/exceptions.hpp>
#include <universal/number/lindex/lindex_fwd.hpp>
#include <universal/number/lindex/lindex_impl.hpp>
#include <universal/traits/lindex_traits.hpp>
#include <universal/number/lindex/numeric_limits.hpp>
