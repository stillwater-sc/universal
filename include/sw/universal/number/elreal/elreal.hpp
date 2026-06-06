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
// CANONICAL LFPERA arithmetic: streaming (online, pull-driven) infSum / multiply
// / divide (#1061). LFPERA is online by design; these are the faithful
// realization. infsum and mul_online are complete; div_online is complete for
// single-block and sparse multi-block divisors (general dense multi-block div is
// the remaining open item). The eager multiply.hpp/divide.hpp/sum.hpp above are
// DEPRECATED Phase-5/6 scaffolding kept only until the math suite migrates onto
// these; see docs/design/elreal-online-convergence.md.
#include <universal/number/elreal/infsum.hpp>
#include <universal/number/elreal/online_multiply.hpp>
#include <universal/number/elreal/online_divide.hpp>
// DEPRECATED eager scaffolding (depth-budgeted batch). Superseded by the
// streaming ops above; retained only while the math suite still calls them.
#include <universal/number/elreal/negate.hpp>
#include <universal/number/elreal/multiply.hpp>
#include <universal/number/elreal/divide.hpp>
// Phase 7 (#931): math suite
#include <universal/number/elreal/math/sqrt.hpp>
#include <universal/number/elreal/math/hypot.hpp>
#include <universal/number/elreal/math/constants.hpp>
#include <universal/number/elreal/math/exponent.hpp>
#include <universal/number/elreal/math/hyperbolic.hpp>
#include <universal/number/elreal/math/trigonometry.hpp>
