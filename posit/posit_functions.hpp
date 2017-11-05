#pragma once

// posit_functions.hpp: simple math functions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "posit.hpp"

/// Magnitude of a posit (same with sign bit turned off).
template<size_t nbits, size_t es> 
posit<nbits, es> abs(const posit<nbits, es>& p)
{
    return posit<nbits, es>(false, p.get_regime(), p.get_exponent(), p.get_fraction());
}