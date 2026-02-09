// zfpblock arithmetic type standard header
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
////////////////////////////////////////////////////////////////////////////////////////
///  COMPILATION DIRECTIVES TO DIFFERENT COMPILERS

// compiler specific configuration for long double support
#include <universal/utility/long_double.hpp>
// compiler specific configuration for C++20 bit_cast
#include <universal/utility/bit_cast.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// required std libraries
#include <iostream>
#include <iomanip>

////////////////////////////////////////////////////////////////////////////////////////
///  BEHAVIORAL COMPILATION SWITCHES

////////////////////////////////////////////////////////////////////////////////////////
// enable throwing specific exceptions for zfpblock codec errors
// left to application to enable
#if !defined(ZFPBLOCK_THROW_ARITHMETIC_EXCEPTION)
// default is to use std::cerr for signalling an error
#define ZFPBLOCK_THROW_ARITHMETIC_EXCEPTION 0
#endif

///////////////////////////////////////////////////////////////////////////////////////
// bring in the trait functions
#include <universal/traits/number_traits.hpp>
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/common/number_traits_reports.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library

#include <universal/number/zfpblock/exceptions.hpp>
#include <universal/number/zfpblock/zfpblock_impl.hpp>
#include <universal/traits/zfpblock_traits.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// useful functions to work with zfpblocks
#include <universal/number/zfpblock/manipulators.hpp>
#include <universal/number/zfpblock/attributes.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// compressed array container wrapping the zfpblock codec
#include <universal/number/zfpblock/zfparray.hpp>
