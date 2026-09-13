// arbitrary configuration decimal floating-point arithmetic standard header
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
////////////////////////////////////////////////////////////////////////////////////////
///  COMPILATION DIRECTIVES TO DIFFERENT COMPILERS
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// required std libraries
#include <iostream>
#include <iomanip>

////////////////////////////////////////////////////////////////////////////////////////
///  BEHAVIORAL COMPILATION SWITCHES
///
/// DFLOAT_ENABLE_LITERALS, DFLOAT_THROW_ARITHMETIC_EXCEPTION, DFLOAT_EXCEPT and
/// DFLOAT_NATIVE_SQRT now default in dfloat_impl.hpp, beside the code they govern, so
/// that a translation unit which includes core.hpp directly gets the same defaults this
/// umbrella used to supply (#1334, #1436). Defining any of them before this header still
/// wins, as before.
///
// NOTE: blockbinary does not currently consume BLOCKBINARY_THROW_ARITHMETIC_EXCEPTION.
// dfloat's own divide-by-zero check at the dfloat layer handles the throw-vs-NaN
// branch; blockbinary's divide silently zero-fills.  If/when blockbinary gains
// a throw path we should add a DFLOAT_... -> BLOCKBINARY_... cascade here.

///////////////////////////////////////////////////////////////////////////////////////
// bring in the trait functions
#include <universal/traits/number_traits.hpp>
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/common/number_traits_reports.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
// layer 1: the arithmetic core (#1334). Include core.hpp directly in a translation
// unit that only computes -- it pulls no <iostream>/<sstream>/<iomanip>.
#include <universal/number/dfloat/core.hpp>

// useful functions to work with dfloats
// layer 2a: the string producers -- to_binary, to_native, type_tag, color_print
#include <universal/number/dfloat/manipulators.hpp>
// layer 2b: the <iostream> half -- operator<< / operator>>
#include <universal/number/dfloat/iostream.hpp>
#include <universal/number/dfloat/attributes.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// elementary math functions library
#include <universal/number/dfloat/mathlib.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// aliases for IEEE 754-2008 decimal floating-point configurations
namespace sw { namespace universal {

// BID encoding (default)
using decimal32  = dfloat<7, 6, DecimalEncoding::BID, uint32_t>;
using decimal64  = dfloat<16, 8, DecimalEncoding::BID, uint32_t>;
using decimal128 = dfloat<34, 12, DecimalEncoding::BID, uint32_t>;

// DPD encoding variants
using decimal32_dpd  = dfloat<7, 6, DecimalEncoding::DPD, uint32_t>;
using decimal64_dpd  = dfloat<16, 8, DecimalEncoding::DPD, uint32_t>;
using decimal128_dpd = dfloat<34, 12, DecimalEncoding::DPD, uint32_t>;

}}  // namespace sw::universal
