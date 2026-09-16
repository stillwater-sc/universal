#pragma once
// core.hpp: the dfixpnt arithmetic core -- no I/O, no text
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 1 of the dfixpnt headers (#1334). Storage, arithmetic, comparison,
// numeric_limits and traits -- everything a compute kernel needs and nothing that turns
// a dfixpnt into a stream.
//
//     #include <universal/number/dfixpnt/dfixpnt.hpp>   // everything, as before
//     #include <universal/number/dfixpnt/core.hpp>      // arithmetic only
//
// dfixpnt.hpp includes this plus manipulators.hpp, iostream.hpp, attributes.hpp and the
// mathlib, so existing code is unaffected. assign(const std::string&) and to_string()
// stay here: both concatenate, neither streams. The storage is a blockdecimal, whose own
// core is I/O-free since #1473.
//
// NOTE: traits/arithmetic_traits.hpp and common/number_traits_reports.hpp are
// deliberately NOT included -- they build report strings and pull <sstream>/<iomanip>.
// The umbrella provides them.
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/directives.hpp>

#include <universal/traits/number_traits.hpp>

#include <universal/number/dfixpnt/exceptions.hpp>
#include <universal/number/dfixpnt/dfixpnt_fwd.hpp>
#include <universal/number/dfixpnt/dfixpnt_impl.hpp>
#include <universal/traits/dfixpnt_traits.hpp>
#include <universal/number/dfixpnt/numeric_limits.hpp>
