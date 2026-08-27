#pragma once
// core.hpp: the double-double arithmetic core -- no streams
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 1 of the dd headers (#1334, Phase 2).
//
//     #include <universal/number/dd/dd.hpp>     // everything, as before
//     #include <universal/number/dd/core.hpp>   // arithmetic only
//
// WHAT "core" EXCLUDES, precisely: the stream family -- <iostream>, <sstream>,
// <iomanip>, <ostream>, <istream>. Measured: this header pulls zero of the five.
//
// It does NOT exclude <string> or <ios>. to_string() and parse() are part of the type's
// arithmetic surface -- parse() is what assign(const std::string&) calls -- and to_string()
// builds its result by string concatenation, never through a stream; it takes
// std::streamsize and std::ios_base flags, which is what <ios> is for. <cstdio> backs
// fprintf, the Phase 0 idiom, used by both the diagnostics and the (default-off)
// bTraceDecimal* trace prints.
//
// The traces are why <iostream> could not simply be dropped: a discarded
// `if constexpr` branch still requires std::cout to be DECLARED, so <iosfwd> is not
// enough and the branch has to stop naming std::cout at all.
//
// native/manipulators_core.hpp rather than the full native/manipulators.hpp: dd calls
// scale(), which is bit manipulation, not text.
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

#include <universal/number/dd/exceptions.hpp>
#include <universal/number/dd/dd_fwd.hpp>
#include <universal/number/dd/dd_impl.hpp>
#include <universal/traits/dd_traits.hpp>
#include <universal/number/dd/numeric_limits.hpp>
