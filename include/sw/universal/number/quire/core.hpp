#pragma once
// core.hpp: the quire arithmetic core -- no streams
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Layer 1 of the quire headers (#1334, Phase 2 group 5).
//
//     #include <universal/number/quire/quire.hpp>   // everything, as before
//     #include <universal/number/quire/core.hpp>   // arithmetic only
//
// WHAT "core" EXCLUDES, precisely: the stream family -- <iostream>, <sstream>,
// <iomanip>, <ostream>, <istream>. Measured: this header pulls zero of the five.
//
// quire is the exact dot-product accumulator; it is generic over the NumberType it
// accumulates, so it layers on internal/blockbinary and internal/blocktriple -- both
// already at 0 I/O from Phase N -- rather than on any one number system.
////////////////////////////////////////////////////////////////////////////////////////
///  BEHAVIORAL COMPILATION SWITCHES
///
/// These live HERE, not only in the umbrella, because core.hpp is a public entry point:
/// quire_impl.hpp tests them in #if directives, so a translation unit that includes only
/// core.hpp would otherwise leave them undefined. That still evaluates to 0 -- the
/// intended default -- but it is a diagnostic under -Wundef and it makes the core's
/// configuration depend on an include the caller was told they do not need (#1334).
/// The umbrella's own #if !defined guards make this a no-op when both are included.

// enable/disable the ability to use literals in binary logic and arithmetic operators
#if !defined(QUIRE_ENABLE_LITERALS)
// default is to enable them
#	define QUIRE_ENABLE_LITERALS 1
#endif

// enable throwing specific exceptions for arithmetic errors
// left to application to enable
#if !defined(QUIRE_THROW_ARITHMETIC_EXCEPTION)
// default is to use a stderr diagnostic as the signalling error
#	define QUIRE_THROW_ARITHMETIC_EXCEPTION 0
#endif

#include <universal/utility/architecture.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/utility/long_double.hpp>

#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/blocktriple/blocktriple.hpp>
#include <universal/traits/quire_traits.hpp>   // quire_traits<>, used by quire_impl.hpp
#include <universal/number/quire/exceptions.hpp>
#include <universal/number/quire/quire_fwd.hpp>
#include <universal/number/quire/quire_impl.hpp>
