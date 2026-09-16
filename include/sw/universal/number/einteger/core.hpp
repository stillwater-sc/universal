#pragma once
// core.hpp: the einteger arithmetic core -- no I/O, no text
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 1 of the einteger headers (#1334, Phase 2 group 5b, #1455). Storage,
// arithmetic, comparison, numeric_limits and traits -- everything a compute kernel
// needs and nothing that turns an einteger into text.
//
//     #include <universal/number/einteger/einteger.hpp>   // everything, as before
//     #include <universal/number/einteger/core.hpp>       // arithmetic only
//
// einteger.hpp includes this plus manipulators.hpp, iostream.hpp and debug.hpp, so
// existing code is unaffected.
//
// parse() and assign(const std::string&) stay here: assign is built on parse, and
// parse reads its four grammars with hand-written matchers rather than std::regex
// (see einteger_impl.hpp), so it needs <string> and <map> but no stream. Diagnostics
// in the core use fprintf(stderr, ...), the idiom Phase 0 established.
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/directives.hpp>

#include <universal/number/einteger/exceptions.hpp>
#include <universal/number/einteger/einteger_fwd.hpp>
#include <universal/number/einteger/einteger_impl.hpp>
#include <universal/number/einteger/numeric_limits.hpp>
#include <universal/traits/einteger_traits.hpp>
