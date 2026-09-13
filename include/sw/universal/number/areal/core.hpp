#pragma once
// core.hpp: the areal arithmetic core -- no I/O, no text
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 1 of the areal headers (#1334, Phase 2 group 5a, #1444). Storage, arithmetic,
// comparison and numeric_limits -- everything a compute kernel needs and nothing that
// turns an areal into text.
//
//     #include <universal/number/areal/areal.hpp>   // everything, as before
//     #include <universal/number/areal/core.hpp>    // arithmetic only
//
// areal.hpp includes this plus manipulators.hpp, iostream.hpp and debug.hpp, so existing
// code is unaffected.
//
// areal had the largest std::cout population of group 5a -- 51 sites -- and almost all of
// them were already behind the TRACE_CONVERSION switch. What kept <iostream> in every
// graph was that the INCLUDE sat outside the guard. Four changes:
//   - <iostream> moved inside `#if TRACE_CONVERSION`, where the code that needs it lives.
//   - constexprClassParameters() is declared in the class and defined in debug.hpp, the
//     shape blocktriple's got in #1388.
//   - operator<< and operator>> keep their friend DECLARATIONS here (operator>> reaches
//     _fraction directly); only the definitions move to iostream.hpp, which is what lets
//     this header get by with <iosfwd>.
//   - assign()'s "assign TBD" stub prints with std::printf rather than std::cout: same
//     destination, same text, no stream header.
//
// to_string() and to_binary() are free functions that format through a stringstream, so
// they are manipulators.hpp's. This is the same criterion applied everywhere in the epic
// -- a builder that opens a stream is text; one that concatenates is core -- and areal's
// to_string opens one.
//
// native/ieee754.hpp was re-pointed to native/ieee754_core.hpp.
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

#include <universal/number/areal/exceptions.hpp>
#include <universal/number/areal/areal_impl.hpp>
#include <universal/number/areal/numeric_limits.hpp>
