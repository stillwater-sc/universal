// elreal.hpp: umbrella header for the McCleeary LFPERA elreal number system.
//
// Phase 1 ships only the `block<FpType>` primitive (issue #925). Higher-level
// pieces (ZBCL co-list, arithmetic, math suite, real-FP conversion) arrive in
// later phases (#926-#933 under epic #923).
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

#include <universal/number/elreal/elreal_fwd.hpp>
#include <universal/number/elreal/exp_field_width.hpp>
#include <universal/number/elreal/block.hpp>
#include <universal/number/elreal/block_manipulators.hpp>
