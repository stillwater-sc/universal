// elreal_fwd.hpp: forward declarations for the elreal (McCleeary LFPERA) types.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

namespace sw { namespace universal {

// Phase 1: the block<FpType> primitive (#925).
// Phase 2: ZBCL<FpType>, the lazy stream of blocks (#926).
// Phase 3+: arithmetic, math suite, real floating-point conversion (#927-#933).
template <typename FpType>
struct block;

template <typename FpType>
class ZBCL;

// The elreal class facade (plug-in arithmetic number system over ZBCL).
template <typename FpType>
class elreal;

template <typename FpType> elreal<FpType> abs(const elreal<FpType>&);
template <typename FpType> elreal<FpType> fabs(const elreal<FpType>&);

// Standard host aliases.
using elreal64 = elreal<double>;
using elreal32 = elreal<float>;

}} // namespace sw::universal
