#pragma once
// core.hpp: the nvblock arithmetic core -- no streams
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 1 of the nvblock headers (#1334, Phase 2 group 5).
//
//     #include <universal/number/nvblock/nvblock.hpp>   // everything, as before
//     #include <universal/number/nvblock/core.hpp>   // arithmetic only
//
// WHAT "core" EXCLUDES, precisely: the stream family -- <iostream>, <sstream>,
// <iomanip>, <ostream>, <istream>. Measured: this header pulls zero of the five.
//
// nvblock is a block format over microfloat elements with an e4m3 scale. It layers on those types'
// core.hpp rather than their umbrellas, which is what lets this core reach zero.
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

#include <universal/number/nvblock/exceptions.hpp>
#include <universal/number/nvblock/nvblock_fwd.hpp>
#include <universal/number/nvblock/nvblock_impl.hpp>
