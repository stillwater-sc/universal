#pragma once
// bposit.hpp: umbrella header for the bounded posit, Gustafson's b-posit <N, rS, eS>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Specification: docs/number-systems/bposit.md (#1251).  Epic #1250.

////////////////////////////////////////////////////////////////////////////////////////
///  COMPILATION DIRECTIVES TO DIFFERENT COMPILERS
#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/directives.hpp>

#include <iostream>
#include <iomanip>

////////////////////////////////////////////////////////////////////////////////////////
///  BEHAVIORAL COMPILATION SWITCHES
// BPOSIT_THROW_ARITHMETIC_EXCEPTION: throw on NaR operands and division by zero, left to
// the application to define before this include.  Its default, and its forwarding to
// blocktriple and blockbinary, lives in bposit_impl.hpp, so core.hpp alone defines it too.

////////////////////////////////////////////////////////////////////////////////////////
/// generic number system traits
#include <universal/traits/number_traits.hpp>
#include <universal/traits/arithmetic_traits.hpp>
#include <universal/common/number_traits_reports.hpp>

////////////////////////////////////////////////////////////////////////////////////////
/// the library: the arithmetic core, then the text and introspection layers (#1334)
#include <universal/number/bposit/core.hpp>
#include <universal/number/bposit/manipulators.hpp>
#include <universal/number/bposit/iostream.hpp>
#include <universal/number/bposit/attributes.hpp>
