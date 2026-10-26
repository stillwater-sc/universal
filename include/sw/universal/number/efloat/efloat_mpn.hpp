#pragma once
// efloat_mpn.hpp: deprecated forwarding header (#1823)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The limb kernels moved to <universal/internal/limbs/limb_kernels.hpp>, namespace
// sw::universal::limb_kernels, in #1823. This header and the namespace alias keep code that
// includes it, or names efloat_mpn, compiling for one release; include the new header instead.
#include <universal/internal/limbs/limb_kernels.hpp>

namespace sw { namespace universal {

namespace efloat_mpn = limb_kernels;

}}  // namespace sw::universal
