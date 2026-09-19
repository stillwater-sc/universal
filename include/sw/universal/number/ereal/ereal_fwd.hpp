#pragma once
// ereal_fwd.hpp :  forward declarations of ereal 
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// core ereal types
// NOTE: maxLimbs must be <= max_safe_limbs of the limb type -- 19 for the default double
// (Shewchuk's expansion arithmetic requires normal limbs; larger values cause underflow).
// The defaults (8 limbs of double) are given on the definition in ereal_impl.hpp.
template<unsigned maxLimbs, typename FpType> class ereal;
template<unsigned maxLimbs, typename FpType> ereal<maxLimbs, FpType> abs(const ereal<maxLimbs, FpType>&);
// fabs is still defined in the math library for double limbs only (#1567)
template<unsigned maxLimbs> ereal<maxLimbs, double> fabs(const ereal<maxLimbs, double>&);

}} // namespace sw::universal

