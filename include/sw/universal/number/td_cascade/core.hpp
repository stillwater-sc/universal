#pragma once
// core.hpp: the td_cascade arithmetic core -- no streams
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 1 of the td_cascade headers (#1334, Phase 2).
//
//     #include <universal/number/td_cascade/td_cascade.hpp>   // everything, as before
//     #include <universal/number/td_cascade/core.hpp>   // arithmetic only
//
// WHAT "core" EXCLUDES, precisely: the stream family -- <iostream>, <sstream>,
// <iomanip>, <ostream>, <istream>. Measured: this header pulls zero of the five.
//
// Same division as dd (#1426) and qd (#1427): to_string() and parse() stay HERE.
// parse() is what assign(const std::string&) calls, and to_string() concatenates a
// std::string without ever opening a stream -- it only takes std::streamsize and
// std::ios_base flags, which is what <ios> is for.
//
// The substrate, internal/floatcascade/floatcascade.hpp, is layered the same way: its
// to_tuple()/to_scientific() builders and its operator<< are separate headers now, which
// is what lets this core reach zero.
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

#include <universal/number/td_cascade/exceptions.hpp>
#include <universal/number/td_cascade/td_cascade_fwd.hpp>
#include <universal/number/td_cascade/td_cascade_impl.hpp>
#include <universal/number/td_cascade/numeric_limits.hpp>
