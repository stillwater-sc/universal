#pragma once
// microfloat_fwd.hpp: forward declarations for microfloat types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// forward reference
template<unsigned nbits, unsigned es, bool hasInf, bool hasNaN, bool isSaturating>
class microfloat;

// type aliases for OCP Microscaling (MX) element types
using e2m1 = microfloat<4, 2, false, false, true>;
using e2m3 = microfloat<6, 2, false, false, true>;
using e3m2 = microfloat<6, 3, false, false, true>;
using e4m3 = microfloat<8, 4, false, true,  true>;
using e5m2 = microfloat<8, 5, true,  true,  false>;

}}  // namespace sw::universal
