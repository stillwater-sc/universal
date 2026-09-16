#pragma once
// long_double.hpp: LONG_DOUBLE_SUPPORT, which now lives in directives.hpp
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// This header carried a second, hand-maintained definition of LONG_DOUBLE_SUPPORT, keyed on the
// compiler. directives.hpp derives the same macro from the long double format the compiler
// reports, and neither definition could see the other: the value a translation unit got depended
// on which header it included first, and the two disagreed on every RISC-V build (#1534, #1399).
//
// The definition is now in directives.hpp alone. This header remains so that its includers keep
// compiling, and goes away once the call sites stop asking through the macro.
#include <universal/utility/directives.hpp>
