// elreal.hpp: umbrella header for the McCleeary LFPERA elreal number system.
//
// Currently exports:
//   Phase 1 (#925): block<FpType>, block manipulators.
//   Phase 2 (#926): ZBCL<FpType> lazy co-list and its empty / from_native /
//                   to_double_approx helpers.
//   Phase 3 (#927): block-level EFTs (block_two_sum / _mult / _div + RN
//                   variants).
//   Phase 4 (#928): threeAdd + add() lazy ZBCL combinator.
//   Phase 5 (#929): infinite summation -- series<FpType> co-list of ZBCL terms
//                   and sum() (dissertation 4.2.3).
//   Phase 6 (#930): negate / mul / div (dissertation 4.2.4 / 4.2.5 / 4.2.6).
//
// Higher-level pieces (math suite, real-FP conversion) arrive in later phases
// (#931-#933 under epic #923).
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

// layer 1: the arithmetic core (#1334). Include core.hpp directly in a translation
// unit that only computes -- it pulls no <iostream>/<sstream>/<iomanip>. It carries
// the block and ZBCL machinery (Phases 1-6), the streaming and eager operators, and
// the elreal class facade (#1079).
#include <universal/number/elreal/core.hpp>

// layer 2: the text. block_manipulators.hpp renders a block; manipulators.hpp an
// elreal; iostream.hpp holds operator<< / operator>>.
#include <universal/number/elreal/block_manipulators.hpp>
#include <universal/number/elreal/attributes.hpp>
#include <universal/number/elreal/manipulators.hpp>
#include <universal/number/elreal/iostream.hpp>
// Phase 7 (#931) math suite, lifted to class elreal by the facade. mathlib.hpp
// pulls in the ZBCL-level math/*.hpp (sqrt/hypot/exp/log/trig/hyperbolic/
// constants) and wraps each at the elreal class level (#1079 Phase 4).
#include <universal/number/elreal/mathlib.hpp>

// Phase 8 (#932): real floating-point conversion -- round_to<double|float|dd|qd>.
#include <universal/number/elreal/round.hpp>
