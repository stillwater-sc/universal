#pragma once
// core.hpp: the dbns arithmetic core -- no I/O, no text
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 1 of the dbns headers (#1334, Phase 2 group 5a, #1444). Storage, arithmetic,
// comparison, numeric_limits and traits -- everything a compute kernel needs and
// nothing that turns a dbns into text.
//
//     #include <universal/number/dbns/dbns.hpp>   // everything, as before
//     #include <universal/number/dbns/core.hpp>   // arithmetic only
//
// dbns.hpp includes this plus manipulators.hpp, iostream.hpp, debug.hpp and
// attributes.hpp, so existing code is unaffected.
//
// Four things had to move for this header to reach zero I/O-family headers, and only
// the first is an include shuffle:
//   - to_binary() formats through a stringstream -> manipulators.hpp.
//   - operator<< and operator>> were in-class friend DEFINITIONS, which pin <iostream>
//     into every consumer -- the trap blockdigit and floatcascade had. They are now
//     namespace-scope templates in iostream.hpp; they need no private access.
//   - debugConstexprParameters() is declared here and defined in debug.hpp, the shape
//     blocktriple's constexprClassParameters() got in #1388.
//   - convert_ieee754's (a,b) search traced behind `if constexpr (bDebug)`. A discarded
//     if-constexpr branch still requires std::cout to be DECLARED, so that is now the
//     DBNS_TRACE_CONVERSION preprocessor switch. bDebug was a local constant no caller
//     could reach, so this also makes the tracing usable for the first time.
//
// assign(const std::string&) stays here -- it is a stub that clears, and opens no
// stream. native/ieee754.hpp was re-pointed to native/ieee754_core.hpp.
//
// NOTE: traits/arithmetic_traits.hpp and common/number_traits_reports.hpp are
// deliberately NOT included -- they build report strings and pull <sstream>/<iomanip>.
// The umbrella provides them.
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

#include <universal/traits/number_traits.hpp>

#include <universal/number/dbns/exceptions.hpp>
#include <universal/number/dbns/dbns_fwd.hpp>
#include <universal/number/dbns/dbns_impl.hpp>
#include <universal/number/dbns/dbns_traits.hpp>
#include <universal/number/dbns/numeric_limits.hpp>
