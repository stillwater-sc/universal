#pragma once
// core.hpp: the zfpblock arithmetic core -- no streams
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 1 of the zfpblock headers (#1334, Phase 2 group 5).
//
//     #include <universal/number/zfpblock/zfpblock.hpp>   // everything, as before
//     #include <universal/number/zfpblock/core.hpp>       // arithmetic only
//
// WHAT "core" EXCLUDES, precisely: the stream family -- <iostream>, <sstream>,
// <iomanip>, <ostream>, <istream>. Measured: this header pulls zero of the five.
//
// zfpblock needed the least work of the block formats: its text already lived in
// manipulators.hpp and it defines no stream operator of its own. The only change to the
// impl was removing an <iomanip> that nothing in the file used.
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

#include <universal/number/zfpblock/exceptions.hpp>
#include <universal/number/zfpblock/zfpblock_fwd.hpp>
#include <universal/number/zfpblock/zfpblock_impl.hpp>
#include <universal/traits/zfpblock_traits.hpp>
