#pragma once
// core.hpp: the unum2 arithmetic core -- no I/O, no text
//
// Copyright (C) 2017-2026 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// Layer 1 of the unum2 headers (#1334, Phase 2 group 5c, #1467): the lattice, the SORN
// value type and the operation matrices -- everything a compute kernel needs and
// nothing that turns a unum2 or a lattice into text.
//
//     #include <universal/number/unum2/unum2.hpp>   // everything, as before
//     #include <universal/number/unum2/core.hpp>    // arithmetic only
//
// unum2.hpp includes this plus manipulators.hpp and iostream.hpp, so existing code is
// unaffected. lattice::get_exact() and lattice::print(), and unum2's operator<<, are
// declared in their classes and defined in those two text headers.
#include <universal/number/unum2/common.hpp>
#include <universal/number/unum2/unum2_fwd.hpp>
#include <universal/number/unum2/unum2_impl.hpp>
#include <universal/number/unum2/op_matrix.hpp>
#include <universal/number/unum2/lattice.hpp>
