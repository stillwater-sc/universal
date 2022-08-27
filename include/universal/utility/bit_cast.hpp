#pragma once
// bit_cast.hpp: C++20 <bit> compiler directive
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// BIT_CAST_SUPPORT is compiler env dependent and drives the algorithm selection of ieee-754 decode
#if defined(__clang__)
/* Clang/LLVM. ---------------------------------------------- */

#ifndef BIT_CAST_SUPPORT
#define BIT_CAST_SUPPORT 0
#define CONSTEXPRESSION
#endif

#ifndef CONSTEXPRESSION
#define CONSTEXPRESSION
#endif

#elif defined(__ICC) || defined(__INTEL_COMPILER)
/* Intel ICC/ICPC. ------------------------------------------ */


#elif defined(__GNUC__) || defined(__GNUG__)
/* GNU GCC/G++. --------------------------------------------- */

#ifndef BIT_CAST_SUPPORT
#define BIT_CAST_SUPPORT 0
#define CONSTEXPRESSION
#endif

#ifndef CONSTEXPRESSION
#define CONSTEXPRESSION
#endif

#elif defined(__HP_cc) || defined(__HP_aCC)
/* Hewlett-Packard C/aC++. ---------------------------------- */

#elif defined(__IBMC__) || defined(__IBMCPP__)
/* IBM XL C/C++. -------------------------------------------- */

#elif defined(_MSC_VER)
/* Microsoft Visual Studio. --------------------------------- */

#ifndef BIT_CAST_SUPPORT
#define BIT_CAST_SUPPORT 1
#include <bit>
#ifndef CONSTEXPRESSION
#define CONSTEXPRESSION constexpr
#endif
#endif

// if you are not controlling BIT_CAST_SUPPORT
// you have the option to indepdently control CONSTEXPRESSION
// default is to turn it off
#ifndef CONSTEXPRESSION
#define CONSTEXPRESSION
#endif

#elif defined(__PGI)
/* Portland Group PGCC/PGCPP. ------------------------------- */

#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
/* Oracle Solaris Studio. ----------------------------------- */

#endif

