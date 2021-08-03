// bitcast.hpp: compiler specialization for C++20 <bit> bit_cast functionality
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#if defined(__clang__)
/* Clang/LLVM. ---------------------------------------------- */
#define BIT_CAST_SUPPORT 0

#elif defined(__ICC) || defined(__INTEL_COMPILER)
/* Intel ICC/ICPC. ------------------------------------------ */
#define BIT_CAST_SUPPORT 0

#elif defined(__GNUC__) || defined(__GNUG__)
/* GNU GCC/G++. --------------------------------------------- */
#define BIT_CAST_SUPPORT 0

#elif defined(__HP_cc) || defined(__HP_aCC)
/* Hewlett-Packard C/aC++. ---------------------------------- */
#define BIT_CAST_SUPPORT 0

#elif defined(__IBMC__) || defined(__IBMCPP__)
/* IBM XL C/C++. -------------------------------------------- */
#define BIT_CAST_SUPPORT 0

#elif defined(_MSC_VER)
/* Microsoft Visual Studio. --------------------------------- */
#define BIT_CAST_SUPPORT 1

#elif defined(__PGI)
/* Portland Group PGCC/PGCPP. ------------------------------- */
#define BIT_CAST_SUPPORT 0

#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
/* Oracle Solaris Studio. ----------------------------------- */
#define BIT_CAST_SUPPORT 0

#endif
