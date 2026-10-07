#pragma once
// poxel.hpp: umbrella header for the poxel, a posit with an uncertainty bit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Specification: docs/number-systems/poxel.md.  Epic #1631.

////////////////////////////////////////////////////////////////////////////////////////
///  COMPILATION DIRECTIVES TO DIFFERENT COMPILERS
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/directives.hpp>

#include <iostream>
#include <iomanip>

////////////////////////////////////////////////////////////////////////////////////////
///  BEHAVIORAL COMPILATION SWITCHES
// POXEL_THROW_ARITHMETIC_EXCEPTION: throw on NaR operands and division by zero, left to
// the application to define before this include; its default and its forwarding to the
// building blocks live in poxel_impl.hpp.

////////////////////////////////////////////////////////////////////////////////////////
/// generic number system traits
#include <universal/traits/number_traits.hpp>
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/common/number_traits_reports.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// the library: the arithmetic core, then the text layers (#1334)
#include <universal/number/poxel/core.hpp>
#include <universal/number/poxel/manipulators.hpp>
#include <universal/number/poxel/iostream.hpp>
#include <universal/number/poxel/attributes.hpp>
