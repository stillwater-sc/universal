#pragma once
// posit_16_2.hpp: specialized 16-bit posit using fast implementation specialized for posit<16,2>
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// DO NOT USE DIRECTLY!
// the compile guards in this file are only valid in the context of the specialization logic
// configured in the main <universal/posit/posit>

#ifndef POSIT_FAST_POSIT_16_2
#define POSIT_FAST_POSIT_16_2 0
#endif

	// guard for the fact that we don't have a specialization yet
#if POSIT_FAST_POSIT_16_2
#undef POSIT_FAST_POSIT_16_2
#define POSIT_FAST_POSIT_16_2 0
#pragma message("Fast specialization of posit<16,2> requested but ignored as fast implemention is TBD")
#endif

namespace sw { namespace universal {

// set the fast specialization variable to indicate that we are running a special template specialization
#if POSIT_FAST_POSIT_16_2
#ifdef _MSC_VER
#pragma message("Fast specialization of posit<16,2>")
//#else
//#warning("Fast specialization of posit<16,2>")
#endif

// fast specialized posit<16,2>
// TODO

#endif // POSIT_FAST_POSIT_16_2

}} // namespace sw::universal
