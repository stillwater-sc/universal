#pragma once
// core.hpp: the dfloat arithmetic core -- no I/O, no text
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 1 of the dfloat headers (#1334, Phase 2 group 5a, #1444). Storage, arithmetic,
// comparison, numeric_limits and traits -- everything a compute kernel needs and
// nothing that turns a dfloat into text.
//
//     #include <universal/number/dfloat/dfloat.hpp>   // everything, as before
//     #include <universal/number/dfloat/core.hpp>     // arithmetic only
//
// dfloat.hpp includes this plus manipulators.hpp, iostream.hpp and attributes.hpp, so
// existing code is unaffected.
//
// WHAT STAYS HERE, and why. str(), sig_to_string() and assign(const std::string&) build
// and consume std::string by concatenation and never open a stream -- that is the layer
// contract, the same one that keeps to_string() in every other core. parse() stays too:
// unlike cfloat's and bfloat16's, dfloat's parse() is a pure character-grammar validator
// that ends in value.assign(number), with no istringstream anywhere, so it is arithmetic
// surface rather than text.
//
// Reaching zero I/O-family headers took two substrate re-points, not just an include
// shuffle:
//   - internal/blockbinary/manipulators.hpp -> internal/blockbinary/to_decimal.hpp.
//     str() renders the significand with to_decimal(), which concatenates, but it shared
//     a header with type_tag/to_binary/to_hex, which stream: 50,808 lines and four of
//     the five. to_decimal.hpp is 33,308 and zero.
//   - native/ieee754.hpp -> native/ieee754_core.hpp, the stream-free half.
//
// NOTE: traits/arithmetic_traits.hpp and common/number_traits_reports.hpp are
// deliberately NOT included -- they build report strings and pull <sstream>/<iomanip>.
// The umbrella provides them.
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

#include <universal/traits/number_traits.hpp>

#include <universal/number/dfloat/exceptions.hpp>
#include <universal/number/dfloat/dfloat_fwd.hpp>
#include <universal/number/dfloat/dfloat_impl.hpp>
#include <universal/traits/dfloat_traits.hpp>
#include <universal/number/dfloat/numeric_limits.hpp>
