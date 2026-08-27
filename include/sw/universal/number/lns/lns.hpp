// arbitrary logarithmic number arithmetic type standard header
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
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
#if !defined(LNS_ENABLE_LITERALS)
// default is to enable them
#define LNS_ENABLE_LITERALS 1
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable throwing specific exceptions for logarithmic number system arithmetic errors
// left to application to enable
#if !defined(LNS_THROW_ARITHMETIC_EXCEPTION)
// default is to use std::cerr for signalling an error
#define LNS_THROW_ARITHMETIC_EXCEPTION 0
#endif
#if !defined(BITBLOCK_THROW_ARITHMETIC_EXCEPTION)
#define BITBLOCK_THROW_ARITHMETIC_EXCEPTION LNS_THROW_ARITHMETIC_EXCEPTION
#endif
// the fused dot product accumulator (quire, via fdp.hpp) must honor the same
// exception policy as the lns it accumulates for (#1226)
#if !defined(QUIRE_THROW_ARITHMETIC_EXCEPTION)
#define QUIRE_THROW_ARITHMETIC_EXCEPTION LNS_THROW_ARITHMETIC_EXCEPTION
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
#include <universal/number/lns/core.hpp>

// useful functions to work with logarithmic numbers
// layer 2a: the string producers
#include <universal/number/lns/manipulators.hpp>
// layer 2b: the <iostream> half -- operator<< / operator>>
#include <universal/number/lns/iostream.hpp>
// layer 3: introspection -- debugConstexprParameters, the statistics printer
#include <universal/number/lns/debug.hpp>
#include <universal/number/lns/attributes.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// math functions
#include <universal/number/lns/mathlib.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// fused dot product / quire accumulation support (quire_mul), matching posit.hpp
#include <universal/number/lns/fdp.hpp>
