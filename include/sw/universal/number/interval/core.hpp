#pragma once
// core.hpp: the interval arithmetic core -- no streams
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 1 of the interval headers (#1334, Phase 2 group 5).
//
//     #include <universal/number/interval/interval.hpp>   // everything, as before
//     #include <universal/number/interval/core.hpp>   // arithmetic only
//
// WHAT "core" EXCLUDES, precisely: the stream family -- <iostream>, <sstream>,
// <iomanip>, <ostream>, <istream>. Measured: this header pulls zero of the five.
//
// interval is interval arithmetic over an arbitrary Scalar, so it pulls in no concrete number
// system of its own.
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

#include <universal/number/interval/exceptions.hpp>
#include <universal/number/interval/interval_fwd.hpp>
#include <universal/number/interval/interval_impl.hpp>
#include <universal/number/interval/interval_traits.hpp>
#include <universal/number/interval/numeric_limits.hpp>
