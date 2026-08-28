#pragma once
// core.hpp: the quire arithmetic core -- no streams
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 1 of the quire headers (#1334, Phase 2 group 5).
//
//     #include <universal/number/quire/quire.hpp>   // everything, as before
//     #include <universal/number/quire/core.hpp>   // arithmetic only
//
// WHAT "core" EXCLUDES, precisely: the stream family -- <iostream>, <sstream>,
// <iomanip>, <ostream>, <istream>. Measured: this header pulls zero of the five.
//
// quire is the exact dot-product accumulator; it is generic over the NumberType it
// accumulates, so it layers on internal/blockbinary and internal/blocktriple -- both
// already at 0 I/O from Phase N -- rather than on any one number system.
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/blocktriple/blocktriple.hpp>
#include <universal/traits/quire_traits.hpp>   // quire_traits<>, used by quire_impl.hpp
#include <universal/number/quire/exceptions.hpp>
#include <universal/number/quire/quire_fwd.hpp>
#include <universal/number/quire/quire_impl.hpp>
