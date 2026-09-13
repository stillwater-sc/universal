#pragma once
// core.hpp: the bisection arithmetic core -- no I/O, no text
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// Layer 1 of the bisection headers (#1334, Phase 2 group 5c, #1467): the encoding, its
// arithmetic, numeric_limits and the pre-built generator/refinement pairs -- everything
// a compute kernel needs and nothing that turns a bisection value into text.
//
//     #include <universal/number/bisection/bisection.hpp>   // everything, as before
//     #include <universal/number/bisection/core.hpp>        // arithmetic only
//
// bisection.hpp includes this plus manipulators.hpp, iostream.hpp and attributes.hpp,
// so existing code is unaffected.
#include <universal/utility/directives.hpp>

#include <universal/number/bisection/exceptions.hpp>
#include <universal/number/bisection/bisection_fwd.hpp>
#include <universal/number/bisection/bisection_impl.hpp>
#include <universal/number/bisection/numeric_limits.hpp>
#include <universal/number/bisection/generators.hpp>
