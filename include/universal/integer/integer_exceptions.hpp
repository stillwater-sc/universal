#pragma once
// integer_exceptions.hpp: definition of integer exceptions
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <exception>

#if defined(__clang__)
/* Clang/LLVM. ---------------------------------------------- */


#elif defined(__ICC) || defined(__INTEL_COMPILER)
/* Intel ICC/ICPC. ------------------------------------------ */


#elif defined(__GNUC__) || defined(__GNUG__)
/* GNU GCC/G++. --------------------------------------------- */


#elif defined(__HP_cc) || defined(__HP_aCC)
/* Hewlett-Packard C/aC++. ---------------------------------- */

#elif defined(__IBMC__) || defined(__IBMCPP__)
/* IBM XL C/C++. -------------------------------------------- */

#elif defined(_MSC_VER)
/* Microsoft Visual Studio. --------------------------------- */


#elif defined(__PGI)
/* Portland Group PGCC/PGCPP. ------------------------------- */

#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
/* Oracle Solaris Studio. ----------------------------------- */

#endif

namespace sw {
namespace unum {

// divide by zero arithmetic exception for integers
struct integer_divide_by_zero : public std::runtime_error {
	integer_divide_by_zero() : std::runtime_error("integer division by zero") {}
};

// overflow exception for integers
struct integer_overflow : public std::runtime_error {
	integer_overflow() : std::runtime_error("integer arithmetic overflow") {}
};

///////////////////////////////////////////////////////////////
// internal implementation exceptions

struct integer_byte_index_out_of_bounds : public std::runtime_error {
	integer_byte_index_out_of_bounds() : std::runtime_error("byte index out of bounds") {}
};

} // namespace unum
} // namespace sw
