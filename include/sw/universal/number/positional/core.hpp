#pragma once
// core.hpp: the positional arithmetic core -- no I/O, no text
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 1 of the positional headers (#1334, Phase 2 group 5c, #1467). Storage,
// arithmetic, comparison, numeric_limits and traits -- everything a compute kernel
// needs and nothing that turns a positional into a stream.
//
//     #include <universal/number/positional/positional.hpp>   // everything, as before
//     #include <universal/number/positional/core.hpp>         // arithmetic only
//
// positional.hpp includes this plus manipulators.hpp, iostream.hpp, attributes.hpp, the
// mathlib and the oi/di/hi aliases, so existing code is unaffected.
//
// to_string() stays here: it hands back blockdigit's to_string(), which concatenates.
// The divide- and modulo-by-zero diagnostics use fprintf(stderr, ...).
//
// NOTE: traits/arithmetic_traits.hpp and common/number_traits_reports.hpp are
// deliberately NOT included -- they build report strings and pull <sstream>/<iomanip>.
// The umbrella provides them.
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/directives.hpp>

#include <universal/traits/number_traits.hpp>

#include <universal/number/positional/exceptions.hpp>
#include <universal/number/positional/positional_fwd.hpp>
#include <universal/number/positional/positional_impl.hpp>
#include <universal/traits/positional_traits.hpp>
#include <universal/number/positional/numeric_limits.hpp>
