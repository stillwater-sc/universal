#pragma once
// core.hpp: the cfloat arithmetic core -- no I/O, no text
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 1 of the cfloat headers (#1334, Phase 2). Storage, arithmetic, comparison,
// numeric_limits and traits -- everything a compute kernel needs and nothing that
// turns a cfloat into text.
//
//     #include <universal/number/cfloat/cfloat.hpp>   // everything, as before
//     #include <universal/number/cfloat/core.hpp>     // arithmetic only
//
// cfloat.hpp includes this plus manipulators.hpp, iostream.hpp and debug.hpp, so
// existing code is unaffected.
//
// The string constructor and assign() stay here: they parse "0b..." with plain
// std::string operations, no streams. The regex/istringstream parse() -- used only by
// operator>> -- is the text layer's, declared in cfloat_fwd.hpp and defined in
// manipulators.hpp, exactly as posit does it.
//
// NOTE: traits/arithmetic_traits.hpp and common/number_traits_reports.hpp are
// deliberately NOT included -- they build report strings and pull <sstream>/<iomanip>.
// The umbrella provides them.
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

#include <universal/traits/number_traits.hpp>

#include <universal/number/cfloat/exceptions.hpp>
#include <universal/number/cfloat/cfloat_fwd.hpp>
#include <universal/number/cfloat/cfloat_impl.hpp>
#include <universal/traits/cfloat_traits.hpp>
#include <universal/number/cfloat/numeric_limits.hpp>
