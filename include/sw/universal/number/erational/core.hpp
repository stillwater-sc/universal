#pragma once
// core.hpp: the erational arithmetic core -- no streams
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 1 of the erational headers (#1334, Phase 2).
//
//     #include <universal/number/erational/erational.hpp>   // everything, as before
//     #include <universal/number/erational/core.hpp>        // arithmetic only
//
// WHAT "core" EXCLUDES, precisely: the stream family -- <iostream>, <sstream>,
// <iomanip>, <ostream>, <istream>. Measured: this header pulls zero of the five.
//
// It does NOT exclude <string> or <regex>: parse() is part of the type's arithmetic
// surface, as it is for integer, fixpnt and edecimal. <cstdio> backs fprintf(stderr,...),
// the Phase 0 idiom adopted so diagnostics do not require <iostream>.
//
// erational stores its numerator and denominator as edecimal, so it layers on edecimal's
// core rather than edecimal's umbrella.
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

#include <universal/number/edecimal/core.hpp>

#include <universal/number/erational/exceptions.hpp>
#include <universal/number/erational/erational_fwd.hpp>
#include <universal/number/erational/erational_impl.hpp>
#include <universal/traits/erational_traits.hpp>
#include <universal/number/erational/numeric_limits.hpp>
