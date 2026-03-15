#pragma once
// unum_fwd.hpp: forward declarations for the unum Type I number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>
#include <string>

namespace sw { namespace universal {

// unum Type I: variable-precision floating-point with uncertainty bit
template<unsigned esizesize, unsigned fsizesize, typename bt = std::uint8_t> class unum;

// free function helpers
template<unsigned esizesize, unsigned fsizesize, typename bt>
unum<esizesize, fsizesize, bt> abs(const unum<esizesize, fsizesize, bt>&);

}} // namespace sw::universal
