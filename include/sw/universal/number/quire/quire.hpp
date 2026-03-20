#pragma once
// quire.hpp: umbrella header for the generalized quire (Kulisch super-accumulator)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The generalized quire is parameterized on the number type it accumulates for,
// with quire_traits<NumberType> defining the accumulator geometry.
//
// This umbrella header assembles the public quire surface. Exact product generation
// remains number-system specific and is provided by companion headers such as posit/fdp.hpp
// or cfloat/fdp.hpp, which feed unrounded blocktriple products into the common quire type.
//
// Usage:
//   #include <universal/number/cfloat/cfloat.hpp>
//   #include <universal/number/quire/quire.hpp>
//
//   using Scalar = cfloat<32, 8, uint32_t, true, false, false>;
//   sw::universal::quire<Scalar> q;
//   q += blocktriple_product;   // accumulate an unrounded product
//   Scalar result = q.convert_to<Scalar>();
//
////////////////////////////////////////////////////////////////////////////////////////
///  COMPILATION DIRECTIVES TO DIFFERENT COMPILERS
#include <universal/utility/compiler.hpp>
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// required std libraries 
#include <iostream>
#include <iomanip>
#include <stdexcept>

////////////////////////////////////////////////////////////////////////////////////////
///  BEHAVIORAL COMPILATION SWITCHES

////////////////////////////////////////////////////////////////////////////////////////
// enable/disable the ability to use literals in binary logic and arithmetic operators
#if !defined(QUIRE_ENABLE_LITERALS)
// default is to enable them
#	define QUIRE_ENABLE_LITERALS 1
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable throwing specific exceptions for integer arithmetic errors
// left to application to enable
#if !defined(QUIRE_THROW_ARITHMETIC_EXCEPTION)
// default is to use std::cerr as a signalling error
#	define QUIRE_THROW_ARITHMETIC_EXCEPTION 0
#endif

///////////////////////////////////////////////////////////////////////////////////////
// bring in the trait functions
#include <universal/traits/number_traits.hpp>
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/common/number_traits_reports.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// useful internal functions to work with quires
#include <universal/utility/boolean_logic_operators.hpp>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/blocktriple/blocktriple.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
#include <universal/number/quire/exceptions.hpp>
#include <universal/traits/quire_traits.hpp>
#include <universal/number/quire/quire_fwd.hpp>
#include <universal/number/quire/quire_impl.hpp>
#include <universal/number/quire/numeric_limits.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// useful external functions to work with quires
#include <universal/number/quire/manipulators.hpp>
#include <universal/number/quire/attributes.hpp>
