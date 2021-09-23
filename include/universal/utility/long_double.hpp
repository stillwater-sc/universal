// long_double.hpp: compiler specialization for long double support
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// compilation guard fir long double support to enable ARM and RISC-V embedded environments


#if defined(__clang__)
/* Clang/LLVM. ---------------------------------------------- */
#define LONG_DOUBLE_SUPPORT 1

#elif defined(__ICC) || defined(__INTEL_COMPILER)
/* Intel ICC/ICPC. ------------------------------------------ */
#define LONG_DOUBLE_SUPPORT 1

#elif defined(__GNUC__) || defined(__GNUG__)
/* GNU GCC/G++. --------------------------------------------- */
#define LONG_DOUBLE_SUPPORT 1

#elif defined(__HP_cc) || defined(__HP_aCC)
/* Hewlett-Packard C/aC++. ---------------------------------- */
#define LONG_DOUBLE_SUPPORT 1

#elif defined(__IBMC__) || defined(__IBMCPP__)
/* IBM XL C/C++. -------------------------------------------- */
#define LONG_DOUBLE_SUPPORT 1

#elif defined(_MSC_VER)
/* Microsoft Visual Studio. --------------------------------- */
#define LONG_DOUBLE_SUPPORT 0

#elif defined(_ARM)
/* ARM ------------------------------------------------------ */
#define LONG_DOUBLE_SUPPORT 0

#elif defined(_RISCV)
/* RISC-V GNU GCC/G++ --------------------------------------- */
#define LONG_DOUBLE_SUPPORT 0

#elif defined(__PGI)
/* Portland Group PGCC/PGCPP. ------------------------------- */
#define LONG_DOUBLE_SUPPORT 1

#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
/* Oracle Solaris Studio. ----------------------------------- */
#define LONG_DOUBLE_SUPPORT 1

#endif
