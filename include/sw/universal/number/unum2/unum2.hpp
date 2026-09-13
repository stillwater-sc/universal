// The Unum2 number system
//
// Copyright (C) 2017-2026 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT

#pragma once

// layer 1: the arithmetic core (#1334). Include core.hpp directly in a translation unit
// that only computes -- it pulls no <iostream>/<sstream>/<iomanip>.
#include <universal/number/unum2/core.hpp>
// layer 2: the text -- lattice::get_exact(), then lattice::print() and operator<<
#include <universal/number/unum2/manipulators.hpp>
#include <universal/number/unum2/iostream.hpp>
