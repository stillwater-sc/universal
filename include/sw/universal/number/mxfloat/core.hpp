#pragma once
// core.hpp: the mxfloat arithmetic core -- no streams
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 1 of the mxfloat headers (#1334, Phase 2 group 5).
//
//     #include <universal/number/mxfloat/mxfloat.hpp>   // everything, as before
//     #include <universal/number/mxfloat/core.hpp>   // arithmetic only
//
// WHAT "core" EXCLUDES, precisely: the stream family -- <iostream>, <sstream>,
// <iomanip>, <ostream>, <istream>. Measured: this header pulls zero of the five.
//
// mxfloat is a block format over e8m0 scale + microfloat elements. It layers on those types'
// core.hpp rather than their umbrellas, which is what lets this core reach zero.
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

#include <universal/number/mxfloat/exceptions.hpp>
#include <universal/number/mxfloat/mxfloat_fwd.hpp>
#include <universal/number/mxfloat/mxblock_impl.hpp>
