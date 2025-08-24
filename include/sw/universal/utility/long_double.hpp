#pragma once
// long_double.hpp: compiler specialization for long double support
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// Some compilers, in particular MSVC, does not support a long double type.
// For floating-point bit twiddling, we need to know the backing
// store of the type. This compiler check yields the define LONG_DOUBLE_SUPPORT
// with the answer to that question.

#if defined(__clang__)
/* Clang/LLVM. ---------------------------------------------- */
#define LONG_DOUBLE_SUPPORT 1

#elif defined(__ICC) || defined(__INTEL_COMPILER)
/* Intel ICC/ICPC. ------------------------------------------ */
#define LONG_DOUBLE_SUPPORT 1

#elif defined(__GNUC__) || defined(__GNUG__)
/* GNU GCC/G++. --------------------------------------------- */
#if defined(__riscv)
#define LONG_DOUBLE_SUPPORT 0
#else
#define LONG_DOUBLE_SUPPORT 1
#endif

#elif defined(__HP_cc) || defined(__HP_aCC)
/* Hewlett-Packard C/aC++. ---------------------------------- */
#define LONG_DOUBLE_SUPPORT 1

#elif defined(__IBMC__) || defined(__IBMCPP__)
/* IBM XL C/C++. -------------------------------------------- */
#define LONG_DOUBLE_SUPPORT 1

#elif defined(_MSC_VER)
/* Microsoft Visual Studio. --------------------------------- */
#define LONG_DOUBLE_SUPPORT 0

#elif defined(__PGI)
/* Portland Group PGCC/PGCPP. ------------------------------- */
#define LONG_DOUBLE_SUPPORT 1

#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
/* Oracle Solaris Studio. ----------------------------------- */
#define LONG_DOUBLE_SUPPORT 1

#endif
