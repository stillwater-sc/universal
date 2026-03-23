#pragma once
// convert.hpp: cross-type conversion and mixed-type arithmetic for Universal numbers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// This is the single-include header for the cross-type conversion
// infrastructure. Include this after your number type headers to
// enable conversions and mixed-type arithmetic between any
// combination of Universal number types.
//
// Example:
//   #include <universal/number/posit/posit.hpp>
//   #include <universal/number/cfloat/cfloat.hpp>
//   #include <universal/number/fixpnt/fixpnt.hpp>
//   #include <universal/number/convert/convert.hpp>   // <-- enables cross-type ops
//
//   posit<32,2> p(3.14159);
//   cfloat<32,8> c = universal_cast<cfloat<32,8>>(p);
//   fixpnt<32,16> f = universal_cast<fixpnt<32,16>>(p);
//   auto sum = p + c;   // mixed-type addition, result is promoted type
//   bool eq = (p == f);  // mixed-type comparison

#include <universal/traits/cross_type_traits.hpp>
#include <universal/number/convert/universal_convert.hpp>
#include <universal/number/convert/mixed_arithmetic.hpp>
