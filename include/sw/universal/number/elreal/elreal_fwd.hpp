// elreal_fwd.hpp: forward declarations for the elreal (McCleeary LFPERA) types.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

namespace sw { namespace universal {

// Phase 1: the block<FpType> primitive.
// Phase 2 (issue #926) will add ZBCL<FpType>, the lazy stream of blocks.
// Phase 3+ adds arithmetic, math suite, real floating-point conversion.
template <typename FpType>
struct block;

}} // namespace sw::universal
