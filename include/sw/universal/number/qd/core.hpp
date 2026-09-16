#pragma once
// core.hpp: the quad-double arithmetic core -- no streams
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 1 of the qd headers (#1334, Phase 2).
//
//     #include <universal/number/qd/qd.hpp>     // everything, as before
//     #include <universal/number/qd/core.hpp>   // arithmetic only
//
// WHAT "core" EXCLUDES, precisely: the stream family -- <iostream>, <sstream>,
// <iomanip>, <ostream>, <istream>. Measured: this header pulls zero of the five.
//
// Same division as dd (#1426): to_string() and parse() stay HERE, not in the text layers.
// parse() is what assign(const std::string&) calls, and to_string() concatenates a
// std::string without ever opening a stream -- it only takes std::streamsize and
// std::ios_base flags, which is what <ios> is for. The free to_quad/to_triple/to_binary/
// to_native/to_components builders, which do use <sstream>, are in manipulators.hpp.
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/directives.hpp>

#include <universal/number/qd/exceptions.hpp>
#include <universal/number/qd/qd_fwd.hpp>
#include <universal/number/qd/qd_impl.hpp>
#include <universal/traits/qd_traits.hpp>
#include <universal/number/qd/numeric_limits.hpp>
// copysign/frexp/ldexp: ulp() in qd_impl.hpp calls ldexp(qd, int), which the impl
// declares but this header defines. Without it a core-only translation unit that
// calls ulp() fails to LINK -- -fsyntax-only is clean, and -Wall only warns
// "used but never defined" (#1334). Pure computation, no I/O; it follows
// numeric_limits.hpp because ldexp static_asserts on numeric_limits<qd>::radix.
#include <universal/number/qd/math/functions/numerics.hpp>
