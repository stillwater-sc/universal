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
//
// Higher-level pieces (math suite, real-FP conversion) arrive in later phases
// (#930-#933 under epic #923).
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

#include <universal/number/elreal/elreal_fwd.hpp>
#include <universal/number/elreal/block.hpp>
#include <universal/number/elreal/block_manipulators.hpp>
#include <universal/number/elreal/zbcl.hpp>
#include <universal/number/elreal/zbcl_helpers.hpp>
#include <universal/number/elreal/block_eft.hpp>
#include <universal/number/elreal/threeAdd.hpp>
#include <universal/number/elreal/exceptions.hpp>
#include <universal/number/elreal/series.hpp>
#include <universal/number/elreal/sum.hpp>
