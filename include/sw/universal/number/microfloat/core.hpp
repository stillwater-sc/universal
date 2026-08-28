#pragma once
// core.hpp: the microfloat arithmetic core -- no streams
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 1 of the microfloat headers (#1334, Phase 2 group 5).
//
//     #include <universal/number/microfloat/microfloat.hpp>   // everything, as before
//     #include <universal/number/microfloat/core.hpp>   // arithmetic only
//
// WHAT "core" EXCLUDES, precisely: the stream family -- <iostream>, <sstream>,
// <iomanip>, <ostream>, <istream>. Measured: this header pulls zero of the five.
//
// microfloat is an element type for the MX and NVFP block formats, so mxfloat and nvblock
// layer on THIS header rather than on the umbrella; that is what lets their cores reach
// zero too.
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

#include <universal/number/microfloat/exceptions.hpp>
#include <universal/number/microfloat/microfloat_fwd.hpp>
#include <universal/number/microfloat/microfloat_impl.hpp>
#include <universal/number/microfloat/numeric_limits.hpp>
