#pragma once
// architecture.hpp: determine the target processor architecture
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// Identify the underlying processor architecture, X86-64, POWER, ARM, or RISC-V

#undef UNIVERSAL_ARCH_X86_64
#undef UNIVERSAL_ARCH_POWER
#undef UNIVERSAL_ARCH_ARM
#undef UNIVERSAL_ARCH_RISCV

#if defined(__x86_64) || defined(__x86_64__) || \
    defined(__amd64__) || defined(__amd64) || \
    defined(_M_X64)
/* x86-64 --------------------------------------------------- */
#define UNIVERSAL_ARCH_X86_64 1

#elif defined(__powerpc) || defined(__powerpc__) || \
      defined(__POWERPC__) || defined(__ppc__) || \
      defined(_M_PPC) || defined(_ARCH_PPC)
/* IBM POWER ------------------------------------------------ */
#define UNIVERSAL_ARCH_POWER 1

#elif defined(_ARM_ARCH) || defined(__arm__) || \
      defined(__arm64) || defined(__arm64__) || \
      defined(_M_ARM) || defined(_M_ARM64) || \
      defined(__aarch64__) || defined(__ARM64_ARCH_8__)
/* ARM ------------------------------------------------------ */
#define UNIVERSAL_ARCH_ARM 1

#elif defined(__riscv)
/* RISC-V --------------------------------------------------- */
#define UNIVERSAL_ARCH_RISCV 1

#endif
