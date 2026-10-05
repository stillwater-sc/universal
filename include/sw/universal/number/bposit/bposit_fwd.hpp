#pragma once
// bposit_fwd.hpp: forward declarations of the bounded posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>

namespace sw { namespace universal {

// bposit<nbits, rs, es, bt>: Gustafson's bounded posit, notation <N, rS, eS>
template<unsigned nbits, unsigned rs, unsigned es, typename bt> class bposit;

// the standard configurations: rS = 6, eS = 5, dynamic range 2^-192 .. 2^192
using bposit16 = bposit<16, 6, 5, std::uint16_t>;
using bposit32 = bposit<32, 6, 5, std::uint32_t>;
using bposit64 = bposit<64, 6, 5, std::uint64_t>;

}} // namespace sw::universal
