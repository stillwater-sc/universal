#pragma once
// fdp.hpp: fused dot product and quire accumulation support for posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// This header now delegates to fdp_generalized.hpp, which provides
// blocktriple-based quire_mul and quire_resolve for posit types
// using the generalized quire (quire_impl.hpp).

#include <universal/number/posit/fdp_generalized.hpp>
