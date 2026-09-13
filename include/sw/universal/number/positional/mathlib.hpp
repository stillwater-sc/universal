#pragma once
// mathlib.hpp: definition of the standard mathematical functions for positional integer type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The math function headers name positional without including it; this header brings
// the core, and iostream.hpp because sqrt() streams a negative argument to std::cerr
// (#1334). The mathlib was not self-contained on main.
#include <universal/number/positional/core.hpp>
#include <universal/number/positional/iostream.hpp>

#include <universal/number/positional/math/logarithm.hpp>
#include <universal/number/positional/math/minmax.hpp>
#include <universal/number/positional/math/pow.hpp>
#include <universal/number/positional/math/sqrt.hpp>
