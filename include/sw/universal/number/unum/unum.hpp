// unum Type I flexible configuration number system standard header
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

////////////////////////////////////////////////////////////////////////////////////////
///  BEHAVIORAL COMPILATION SWITCHES

////////////////////////////////////////////////////////////////////////////////////////
// UNUM_ENABLE_LITERALS and UNUM_THROW_ARITHMETIC_EXCEPTION now default in unum_impl.hpp,
// so that a translation unit which includes core.hpp directly gets the same defaults this
// umbrella used to supply (#1334, #1436). Defining either before this header still wins.

// layer 1: the arithmetic core (#1334). Include core.hpp directly in a translation unit
// that only computes -- it pulls no <iostream>/<sstream>/<iomanip>.
#include <universal/number/unum/core.hpp>

// The core reads IEEE-754 fields through native/ieee754_core.hpp. The full native
// support -- to_binary/to_hex/color_print on float and double -- used to arrive here
// through unum_impl.hpp, so the umbrella keeps providing it.
#include <universal/native/ieee754.hpp>

// layer 2a: the string producers and parse(); layer 2b: operator<< / operator>>
#include <universal/number/unum/manipulators.hpp>
#include <universal/number/unum/iostream.hpp>
