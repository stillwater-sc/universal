#pragma once
// core.hpp: the sorn arithmetic core -- no I/O, no text
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 1 of the sorn headers (#1334, Phase 2 group 5c, #1467): the SORN lattice, the
// value type and its arithmetic, numeric_limits and traits.
//
//     #include <universal/number/sorn/sorn.hpp>   // everything, as before
//     #include <universal/number/sorn/core.hpp>   // arithmetic only
//
// sorn.hpp includes this plus sorn_type_tag.hpp, manipulators.hpp and iostream.hpp, so
// existing code is unaffected. getInt(), getConfig() and getDT() -- the members that
// produce text -- are declared in their classes and defined in manipulators.hpp.
#include <universal/utility/directives.hpp>
#include <universal/utility/bit_cast.hpp>

#include <universal/number/sorn/exceptions.hpp>
#include <universal/number/sorn/sorn_fwd.hpp>
#include <universal/number/sorn/sorn_impl.hpp>
#include <universal/number/sorn/sorn_traits.hpp>
#include <universal/number/sorn/numeric_limits.hpp>
