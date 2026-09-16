#pragma once
// core.hpp: the efloat arithmetic core -- no I/O, no text
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 1 of the efloat headers (#1334, Phase 2 group 5b, #1455). Storage,
// arithmetic, comparison and traits -- everything a compute kernel needs and
// nothing that turns an efloat into a stream.
//
//     #include <universal/number/efloat/efloat.hpp>   // everything, as before
//     #include <universal/number/efloat/core.hpp>     // arithmetic only
//
// efloat.hpp includes this plus manipulators.hpp, iostream.hpp, attributes.hpp and
// the mathlib, so existing code is unaffected.
//
// parse() and to_string() stay here. parse() backs assign(const std::string&) and
// goes through utility/decimal_to_binary.hpp, which is I/O-free; to_string()
// concatenates a std::string and only TAKES a std::streamsize, which is <ios>, not a
// stream. Only the stream operators and the stringstream-built to_binary() move out.
//
// NOTE: traits/arithmetic_traits.hpp and common/number_traits_reports.hpp are
// deliberately NOT included -- they build report strings and pull <sstream>/<iomanip>.
// The umbrella provides them. So is the mathlib: math/complex.hpp reaches <complex>,
// which pulls <sstream> in libstdc++.
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/directives.hpp>

#include <universal/traits/number_traits.hpp>

#include <universal/number/efloat/exceptions.hpp>
#include <universal/number/efloat/efloat_fwd.hpp>
#include <universal/number/efloat/efloat_impl.hpp>
#include <universal/traits/efloat_traits.hpp>
