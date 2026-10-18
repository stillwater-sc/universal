#pragma once
// lindex_fwd.hpp: forward declarations of the level-index number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>

namespace sw { namespace universal {

// lindex<nbits, lbits, bt>: level-index (Clenshaw and Olver 1984), see docs/level-index-design.md
template<unsigned nbits, unsigned lbits, typename bt> class lindex;
template<unsigned nbits, unsigned lbits, typename bt> constexpr lindex<nbits, lbits, bt> abs(const lindex<nbits, lbits, bt>&) noexcept;

}} // namespace sw::universal
