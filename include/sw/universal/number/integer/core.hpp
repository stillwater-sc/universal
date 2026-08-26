#pragma once
// core.hpp: the integer arithmetic core -- no I/O, no text
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 1 of the integer headers (#1334, Phase N). Storage, arithmetic, comparison,
// numeric_limits and traits -- everything a compute kernel needs and nothing that
// turns an integer into text.
//
//     #include <universal/number/integer/integer.hpp>   // everything, as before
//     #include <universal/number/integer/core.hpp>      // arithmetic only
//
// integer.hpp includes this header plus manipulators.hpp and iostream.hpp, so
// existing code is unaffected.
//
// This header matters beyond integer itself: utility/decimal_to_binary.hpp needs
// arbitrary-precision integers to convert a decimal literal, and it is reached from
// posit, cfloat, fixpnt, dd and qd. Pointing it at the integer umbrella put all five
// I/O-family headers into every one of those cores. It points here instead.
//
// String construction and parsing DO stay in the core: integer(const std::string&),
// assign() and parse() are part of the type's arithmetic surface, and <string> is
// inside the epic's derived line budget. What is excluded is the stream family --
// <iostream>, <sstream>, <iomanip>, <ostream>, <istream>. Diagnostics in the core
// use fprintf(stderr, ...), the idiom Phase 0 established.
//
// NOTE: traits/arithmetic_traits.hpp and common/number_traits_reports.hpp are
// deliberately NOT included here -- they build report strings and pull <sstream>
// and <iomanip>. The umbrella provides them.
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

#include <universal/traits/number_traits.hpp>

#include <universal/number/integer/exceptions.hpp>
#include <universal/number/integer/integer_fwd.hpp>
#include <universal/number/integer/integer_impl.hpp>
#include <universal/traits/integer_traits.hpp>
#include <universal/number/integer/numeric_limits.hpp>
