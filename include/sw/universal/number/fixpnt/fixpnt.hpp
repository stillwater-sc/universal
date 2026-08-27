// arbitrary fixed-point arithmetic type standard header
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

////////////////////////////////////////////////////////////////////////////////////////
///  BEHAVIORAL COMPILATION SWITCHES

////////////////////////////////////////////////////////////////////////////////////////
// enable/disable the ability to use literals in binary logic and arithmetic operators
#if !defined(FIXPNT_ENABLE_LITERALS)
// default is to enable them
#define FIXPNT_ENABLE_LITERALS 1
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable native SQRT instruction for this arithmetic
#if !defined(FIXPNT_NATIVE_SQRT)
// default is to enable them
#define FIXPNT_NATIVE_SQRT 1
// if disabled, the sqrt function marshals through native IEEE-754 double
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable throwing specific exceptions for integer arithmetic errors
// left to application to enable
#if !defined(FIXPNT_THROW_ARITHMETIC_EXCEPTION)
// default is to use std::cerr for signalling an error
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 0
#endif
// the fused dot product accumulator (quire, via fdp.hpp) must honor the same
// exception policy as the fixpnt it accumulates for (#1226)
#if !defined(QUIRE_THROW_ARITHMETIC_EXCEPTION)
#define QUIRE_THROW_ARITHMETIC_EXCEPTION FIXPNT_THROW_ARITHMETIC_EXCEPTION
#endif

///////////////////////////////////////////////////////////////////////////////////////
// bring in the trait functions
#include <universal/traits/number_traits.hpp>
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/common/number_traits_reports.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
// layer 1: the arithmetic core (#1334). Include core.hpp directly in a translation
// unit that only computes -- it pulls no <iostream>/<sstream>/<iomanip>.
#include <universal/number/fixpnt/core.hpp>

// useful functions to work with fixpnts
#include <universal/number/fixpnt/attributes.hpp>
#include <universal/number/fixpnt/manipulators.hpp>
// layer 2b: the <iostream> half -- operator<< / operator>> / parse
#include <universal/number/fixpnt/iostream.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// math functions
#include <universal/number/fixpnt/mathlib.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// fused dot product / quire accumulation support (quire_mul), matching posit.hpp/cfloat.hpp/lns.hpp
#include <universal/number/fixpnt/fdp.hpp>
