#pragma once
// core.hpp: the bfloat16 arithmetic core -- no I/O, no text
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 1 of the bfloat16 headers (#1334, Phase 2 group 5a, #1444). Storage,
// arithmetic, comparison, numeric_limits and traits -- everything a compute kernel
// needs and nothing that turns a bfloat16 into text.
//
//     #include <universal/number/bfloat16/bfloat16.hpp>   // everything, as before
//     #include <universal/number/bfloat16/core.hpp>       // arithmetic only
//
// bfloat16.hpp includes this plus manipulators.hpp, iostream.hpp and attributes.hpp,
// so existing code is unaffected.
//
// assign(const std::string&) stays here: it walks a "0b...." pattern with plain
// std::string operations and reports malformed input through fprintf(stderr, ...),
// so it needs no stream. The istringstream-based parse() -- used only by operator>>
// -- is the text layer's, declared in bfloat16_fwd.hpp and defined in
// manipulators.hpp, exactly as cfloat and posit do it.
//
// NOTE: traits/arithmetic_traits.hpp and common/number_traits_reports.hpp are
// deliberately NOT included -- they build report strings and pull <sstream>/<iomanip>.
// The umbrella provides them.
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

#include <universal/traits/number_traits.hpp>

#include <universal/number/bfloat16/exceptions.hpp>
#include <universal/number/bfloat16/bfloat16_fwd.hpp>
#include <universal/number/bfloat16/bfloat16_impl.hpp>
#include <universal/traits/bfloat16_traits.hpp>
#include <universal/number/bfloat16/numeric_limits.hpp>
