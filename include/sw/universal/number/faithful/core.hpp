#pragma once
// core.hpp: the faithful arithmetic core -- no streams
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 1 of the faithful headers (#1334, Phase 2 group 5).
//
//     #include <universal/number/faithful/faithful.hpp>   // everything, as before
//     #include <universal/number/faithful/core.hpp>   // arithmetic only
//
// WHAT "core" EXCLUDES, precisely: the stream family -- <iostream>, <sstream>,
// <iomanip>, <ostream>, <istream>. Measured: this header pulls zero of the five.
//
// faithful is a value-plus-error pair over an arbitrary FPType, so it pulls in no concrete
// number system of its own.
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

#include <universal/number/faithful/exceptions.hpp>
#include <universal/number/faithful/faithful_impl.hpp>
#include <universal/number/faithful/faithful_traits.hpp>
#include <universal/number/faithful/numeric_limits.hpp>
