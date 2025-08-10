#pragma once
// posit_8_1.hpp: specialized 8-bit posit using fast implementation specialized for posit<8,1>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// DO NOT USE DIRECTLY!
// the compile guards in this file are only valid in the context of the specialization logic
// configured in the main <universal/posit/posit>

#ifndef POSIT_FAST_POSIT_8_1
#define POSIT_FAST_POSIT_8_1 0
#endif

	// guard for the fact that we don't have a specialization yet
#if POSIT_FAST_POSIT_8_1
#undef POSIT_FAST_POSIT_8_1
#define POSIT_FAST_POSIT_8_1 0
#ifdef _MSC_VER
#pragma message("Fast specialization of posit<8,1> requested but ignored as fast implemention is TBD")
#else
#pragma GCC warning "Fast specialization of posit<8,1> requested but ignored as fast implemention is TBD"
#endif
#endif

namespace sw { namespace universal {

// set the fast specialization variable to indicate that we are running a special template specialization
#if POSIT_FAST_POSIT_8_1
#ifdef _MSC_VER
#pragma message("Fast specialization of posit<8,1>")
//#else  some compile time message that indicates that we are using a specialization for non MS compilers
//#warning("Fast specialization of posit<8,1>")
#endif

// fast specialized posit<8,1>
// TODO

#endif // POSIT_FAST_POSIT_8_1

}} // namespace sw::universal
