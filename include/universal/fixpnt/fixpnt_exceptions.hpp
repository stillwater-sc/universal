#pragma once
// fixpnt_exceptions.hpp: definition of fixed-point exceptions
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

// base class for fixed-point arithmetic exceptions
struct fixpnt_arithmetic_exception
	: public std::runtime_error
{
	explicit fixpnt_arithmetic_exception(const std::string& error) 
		: std::runtime_error(std::string("fixed-point arithmetic exception: ") + error) {};
};

// divide by zero arithmetic exception for fixed-point
struct fixpnt_divide_by_zero : public fixpnt_arithmetic_exception {
	explicit fixpnt_divide_by_zero(const std::string& error = "fixed-point division by zero") 
		: fixpnt_arithmetic_exception(error) {}
};

// overflow exception for fixed-point
struct fixpnt_overflow : public fixpnt_arithmetic_exception {
	explicit fixpnt_overflow(const std::string& error = "fixed-point overflow") 
		: fixpnt_arithmetic_exception(error) {}
};

///////////////////////////////////////////////////////////////
// internal implementation exceptions

// base class for fixed-point internal exceptions
struct fixpnt_internal_exception
	: public std::runtime_error
{
	explicit fixpnt_internal_exception(const std::string& error) 
		: std::runtime_error(std::string("fixed-point internal exception: ") + error) {};
};

struct fixpnt_byte_index_out_of_bounds : public fixpnt_internal_exception {
	explicit fixpnt_byte_index_out_of_bounds(const std::string& error = "byte index out of bounds") 
		: fixpnt_internal_exception(error) {}
};

} // namespace unum
} // namespace sw
