#pragma once
// specializations.hpp: configure the fast takum specializations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// Follows the posit specialization convention (archive/.../posit1/specializations.hpp):
// TAKUM_FAST_SPECIALIZATION turns on every fast implementation, and each one has its
// own TAKUM_FAST_TAKUM_`nbits` macro for fine-grained control.  Define either before
// including takum.hpp, and identically in every translation unit of a program.
#ifdef TAKUM_FAST_SPECIALIZATION
#define TAKUM_FAST_TAKUM_16 1
#endif

#ifndef TAKUM_FAST_TAKUM_16
#define TAKUM_FAST_TAKUM_16 0
#endif
#if TAKUM_FAST_TAKUM_16
#include <universal/number/takum/specialized/takum_16_3.hpp>
#endif
