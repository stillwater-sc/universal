#pragma once
// ieee754.hpp: manipulation functions for IEEE-754 native types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <sstream>
#include <iomanip>
#include <cmath>    // for frexpf/frexp/frexpl  float/double/long double fraction/exponent extraction
#include <limits>
#include <tuple>

// configure the low level compiler interface to deal with floating-point bit manipulation
#include <universal/utility/architecture.hpp>
#include <universal/utility/compiler.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

// set up the database of compiler/architecture specific floating-point parameters
#include <universal/native/ieee754_parameter.hpp>
#include <universal/native/ieee754_decoder.hpp>
#include <universal/native/ieee754_type_tag.hpp>

// if the compiler environment allows, set up
// constexpr compatible bit casts, otherwise
// fallback to nonconstexpr bit casts.
#include <universal/native/extract_fields.hpp>
#include <universal/native/set_fields.hpp>

// functions that do not need to be constexpr
#include <universal/native/nonconst_bitcast.hpp>
#include <universal/native/ieee754_float.hpp>
#include <universal/native/ieee754_double.hpp>
#include <universal/native/ieee754_longdouble.hpp>
// above includes are a refactoring of this old include
//#include <universal/native/nonconstexpr754.hpp>

// support functions
#include <universal/native/integers.hpp>
#include <universal/native/ieee754_parameter_ostream.hpp>
#include <universal/native/manipulators.hpp>
#include <universal/native/attributes.hpp>
#include <universal/traits/arithmetic_traits.hpp>

// numeric helpers
#include <universal/native/ieee754_numeric.hpp>

// error-free operations
#include <universal/native/error_free_ops.hpp>
