// flexible configuration unum arithmetic type standard header
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once
////////////////////////////////////////////////////////////////////////////////////////
///  BEHAVIORAL COMPILATION SWITCHES

////////////////////////////////////////////////////////////////////////////////////////
// enable/disable the ability to use literals in binary logic and arithmetic operators
#if !defined(UNUM_ENABLE_LITERALS)
// default is to enable them
#define UNUM_ENABLE_LITERALS 1
#endif

////////////////////////////////////////////////////////////////////////////////////////
// enable throwing specific exceptions for integer arithmetic errors
// left to application to enable
#if !defined(UNUM_THROW_ARITHMETIC_EXCEPTION)
// default is to use std::cerr for signalling an error
#define UNUM_THROW_ARITHMETIC_EXCEPTION 0
#endif

////////////////////////////////////////////////////////////////////////////////////////
/// INCLUDE FILES that make up the library
#include <universal/number/unum/unum_impl.hpp>
#include <universal/number/unum/numeric_limits.hpp>
#include <universal/number/unum/exceptions.hpp>

///////////////////////////////////////////////////////////////////////////////////////
/// math functions
#include <universal/number/unum/math_functions.hpp>
