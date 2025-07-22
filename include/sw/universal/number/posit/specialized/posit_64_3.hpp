#pragma once
// posit_64_3.hpp: specialized 64-bit posit using fast compute specialized for posit<64,3>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// DO NOT USE DIRECTLY!
// the compile guards in this file are only valid in the context of the specialization logic
// configured in the main <universal/posit/posit>

#ifndef POSIT_FAST_POSIT_64_3
#define POSIT_FAST_POSIT_64_3 0
#endif

	// guard for the fact that we don't have a specialization yet
#if POSIT_FAST_POSIT_64_3
#undef POSIT_FAST_POSIT_64_3
#define POSIT_FAST_POSIT_64_3 0
#ifdef _MSC_VER
#pragma message("Fast specialization of posit<64,3> requested but ignored as fast implemention is TBD")
#else
#pragma GCC warning "Fast specialization of posit<64,3> requested but ignored as fast implemention is TBD"
#endif
#endif

namespace sw { namespace universal {

	// test the fast specialization variable to indicate that we are running a special template specialization
#if POSIT_FAST_POSIT_64_3
#ifdef _MSC_VER
#pragma message("Fast specialization of posit<64,3>")
#else
#pragma GCC warning message "Fast specialization of posit<64,3>"
#endif

// fast specialized posit<64,3>
// TODO

#endif // POSIT_FAST_POSIT_64_3

}} // namespace sw::universal
