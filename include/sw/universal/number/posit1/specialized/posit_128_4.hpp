#pragma once
// posit_128_4.hpp: specialized 128-bit posit using fast compute specialized for posit<128,4>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// DO NOT USE DIRECTLY!
// the compile guards in this file are only valid in the context of the specialization logic
// configured in the main <universal/posit/posit>

#ifndef POSIT_FAST_POSIT_128_4
#define POSIT_FAST_POSIT_128_4 0
#endif

#include <universal/utility/directives.hpp>

	// guard for the fact that we don't have a specialization yet
#if POSIT_FAST_POSIT_128_4
#undef POSIT_FAST_POSIT_128_4
#define POSIT_FAST_POSIT_128_4 0
UNIVERSAL_COMPILER_MESSAGE("Fast specialization of posit<128,4> requested but ignored as fast implemention is TBD")
#endif

namespace sw { namespace universal {

	// test the fast specialization variable to indicate that we are running a special template specialization
#if POSIT_FAST_POSIT_128_4
UNIVERSAL_COMPILER_MESSAGE("Fast specialization of posit<128,4>")

// fast specialized posit<128,4>
// TODO

#endif // POSIT_FAST_POSIT_128_4

}} // namespace sw::universal
