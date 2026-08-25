#pragma once
// ieee754_core.hpp: the arithmetic and bit-manipulation half of the native IEEE-754 support
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// ieee754.hpp is the umbrella: this header plus the text layer. Include THIS one
// from anything that only decodes, encodes or manipulates bits -- it pulls no
// <iostream>, <sstream> or <iomanip> (#1334).
//
//     #include <universal/native/ieee754.hpp>        // everything, as before
//     #include <universal/native/ieee754_core.hpp>   // bit manipulation only
//
// What is deliberately NOT here, and where to get it if you need it:
//
//   ieee754_float.hpp / ieee754_double.hpp   to_hex, to_binary, to_triple,
//   ieee754_longdouble.hpp                   to_base2_scientific, color_print
//                                            (the long-double dispatcher pulls
//                                            the per-compiler headers, which are
//                                            text; extract_fp_components comes
//                                            from its own header, included here)
//   ieee754_parameter_ostream.hpp            operator<< for ieee754_parameter
//   manipulators.hpp, attributes.hpp         reporting helpers
//   traits/arithmetic_traits.hpp             minmax_range, symmetry_range, ...
//   integers.hpp                             MIXED: ipow/nlz are core, but
//                                            to_binary/to_hex bring <sstream>,
//                                            so include it directly if you need
//                                            the integer helpers
//
// ieee_components() for float, double and long double IS here, via
// ieee754_components.hpp -- that is the one core function those text headers
// used to carry.
#include <cmath>    // frexpf/frexp/frexpl fraction/exponent extraction
#include <limits>
#include <tuple>

// configure the low level compiler interface for floating-point bit manipulation
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

// the database of compiler/architecture specific floating-point parameters
#include <universal/native/ieee754_parameter.hpp>
#include <universal/native/ieee754_decoder.hpp>
#include <universal/native/ieee754_type_tag.hpp>
#include <universal/native/integer_type_tag.hpp>   // type_tag(IntegralType): the block
                                                  // types name themselves through this, and
                                                  // blocktriple's type_tag calls it (#1334)

// constexpr compatible bit casts where the compiler allows, else a fallback
#include <universal/native/extract_fields.hpp>
#include <universal/native/set_fields.hpp>
#include <universal/native/nonconst_bitcast.hpp>

// sign/exponent/fraction extraction for float, double and long double
#include <universal/native/ieee754_components.hpp>
#include <universal/native/nonconstexpr/extract_fp_components.hpp>

// numeric helpers
#include <universal/native/ieee754_numeric.hpp>
