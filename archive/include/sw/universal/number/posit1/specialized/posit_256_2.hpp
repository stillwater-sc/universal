#pragma once
// posit_256_2.hpp: specialized 256-bit posit using fast implementation specialized for posit<256,2>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// DO NOT USE DIRECTLY!
// the compile guards in this file are only valid in the context of the specialization logic
// configured in the main <universal/posit/posit>

#ifndef POSIT_FAST_POSIT_256_2
#define POSIT_FAST_POSIT_256_2 0
#endif

#include <universal/utility/directives.hpp>

	// guard for the fact that we don't have a specialization yet
#if POSIT_FAST_POSIT_256_2
#undef POSIT_FAST_POSIT_256_2
#define POSIT_FAST_POSIT_256_2 0
UNIVERSAL_COMPILER_MESSAGE("Fast specialization of posit<256,2> requested but ignored as fast implemention is TBD")
#endif

namespace sw { namespace universal {

// set the fast specialization variable to indicate that we are running a special template specialization
#if POSIT_FAST_POSIT_256_2
UNIVERSAL_COMPILER_MESSAGE("Fast specialization of posit<256,2>")

// fast specialized posit<256,2>
// TODO

#endif // POSIT_FAST_POSIT_256_2

}} // namespace sw::universal
