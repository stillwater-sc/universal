#pragma once
// ieee754_longdouble.hpp: manipulation functions for IEEE-754 native long double types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// compiler specific long double IEEE floating point

/*
Long double is not consistently implemented across different 
compilers and processor architectures.

The following section organizes the implementation details of
each of the compilers supported.

The x86 extended precision format is an 80-bit format first
implemented in the Intel 8087 math coprocessor and is supported
by all processors that are based on the x86 design that incorporate
a floating-point unit(FPU).This 80 - bit format uses one bit for
the sign of the significand, 15 bits for the exponent field
(i.e. the same range as the 128 - bit quadruple precision IEEE 754 format)
and 64 bits for the significand. The exponent field is biased by 16383,
meaning that 16383 has to be subtracted from the value in the
exponent field to compute the actual power of 2.
An exponent field value of 32767 (all fifteen bits 1) is reserved
so as to enable the representation of special states such as
infinity and Not a Number.If the exponent field is zero, the
value is a denormal number and the exponent of 2 is 16382.
*/
#include <universal/native/nonconstexpr/extract_fp_components.hpp>

// specialize for the different compiler environments
#include <universal/native/nonconstexpr/msvc_long_double.hpp>
#include <universal/native/nonconstexpr/clang_long_double.hpp>
#include <universal/native/nonconstexpr/gcc_long_double.hpp>
#include <universal/native/nonconstexpr/riscv_long_double.hpp>
/*
  the support for these compilers is not up to date
#include <universal/native/nonconstexpr/intelicc_long_double.hpp>
#include <universal/native/nonconstexpr/ibmxlc_long_double.hpp>
#include <universal/native/nonconstexpr/hpcc_long_double.hpp>
#include <universal/native/nonconstexpr/pgi_long_double.hpp>
#include <universal/native/nonconstexpr/sunpro_long_double.hpp>
*/
