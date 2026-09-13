#pragma once
// core.hpp: the unum Type I arithmetic core -- no I/O, no text
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 1 of the unum headers (#1334, Phase 2 group 5c, #1467): the unum and ubound
// value types, their arithmetic, numeric_limits and the math functions -- everything a
// compute kernel needs and nothing that turns a unum into text.
//
//     #include <universal/number/unum/unum.hpp>   // everything, as before
//     #include <universal/number/unum/core.hpp>   // arithmetic only
//
// unum.hpp includes this plus manipulators.hpp and iostream.hpp, so existing code is
// unaffected. parse() reads a decimal literal through an istringstream, and nothing in
// the core calls it -- only operator>> does -- so it lives in manipulators.hpp.
#include <universal/number/unum/exceptions.hpp>
#include <universal/number/unum/unum_fwd.hpp>
#include <universal/number/unum/unum_impl.hpp>
#include <universal/number/unum/numeric_limits.hpp>
#include <universal/number/unum/math_functions.hpp>
#include <universal/number/unum/ubound.hpp>
