#pragma once
// core.hpp: the elreal arithmetic core -- no I/O, no text
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 1 of the elreal headers (#1334, Phase 2 group 5b, #1455): the block, the ZBCL
// machinery, the streaming and eager operators, the elreal class facade,
// numeric_limits and traits -- everything a compute kernel needs and nothing that
// turns an elreal into text.
//
//     #include <universal/number/elreal/elreal.hpp>   // everything, as before
//     #include <universal/number/elreal/core.hpp>     // arithmetic only
//
// elreal.hpp includes this plus the text layers (block_manipulators.hpp,
// manipulators.hpp, iostream.hpp), attributes.hpp, the mathlib and round.hpp, so
// existing code is unaffected. The mathlib and round.hpp are I/O-free too, and can be
// added to a core translation unit as they are.
//
// elreal's own code was already free of streams. What put all five I/O-family headers
// into every elreal translation unit was the types it is built from, reached by their
// umbrellas: block.hpp took integer.hpp for the exponent, round.hpp took dd.hpp,
// qd.hpp and cfloat.hpp. All four have cores now, and elreal points at those.
#include <universal/number/elreal/elreal_fwd.hpp>
#include <universal/number/elreal/block.hpp>
#include <universal/number/elreal/zbcl.hpp>
#include <universal/number/elreal/zbcl_helpers.hpp>
#include <universal/number/elreal/block_eft.hpp>
#include <universal/number/elreal/threeAdd.hpp>
#include <universal/number/elreal/exceptions.hpp>
#include <universal/number/elreal/series.hpp>
#include <universal/number/elreal/sum.hpp>
// CANONICAL LFPERA arithmetic: streaming infSum / multiply / divide (#1061)
#include <universal/number/elreal/infsum.hpp>
#include <universal/number/elreal/online_multiply.hpp>
#include <universal/number/elreal/online_divide.hpp>
// DEPRECATED eager scaffolding, retained while the math suite still calls it
#include <universal/number/elreal/negate.hpp>
#include <universal/number/elreal/multiply.hpp>
#include <universal/number/elreal/divide.hpp>
// the elreal class facade
#include <universal/number/elreal/elreal_impl.hpp>
#include <universal/number/elreal/numeric_limits.hpp>
#include <universal/traits/elreal_traits.hpp>
