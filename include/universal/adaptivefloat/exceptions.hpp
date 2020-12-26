#pragma once
// exceptions.hpp: definition of adaptivefloat exceptions
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
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

namespace sw { namespace universal {

// base class for adaptivefloat arithmetic exceptions
struct adaptivefloat_arithmetic_exception
	: public std::runtime_error
{
	explicit adaptivefloat_arithmetic_exception(const std::string& error) 
		: std::runtime_error(std::string("fixed-point arithmetic exception: ") + error) {};
};

// divide by zero arithmetic exception for fixed-point
struct adaptivefloat_divide_by_zero : public adaptivefloat_arithmetic_exception {
	explicit adaptivefloat_divide_by_zero(const std::string& error = "fixed-point division by zero") 
		: adaptivefloat_arithmetic_exception(error) {}
};

///////////////////////////////////////////////////////////////
// internal implementation exceptions

// base class for fixed-point internal exceptions
struct adaptivefloat_internal_exception
	: public std::runtime_error
{
	explicit adaptivefloat_internal_exception(const std::string& error) 
		: std::runtime_error(std::string("fixed-point internal exception: ") + error) {};
};

struct adaptivefloat_word_index_out_of_bounds : public adaptivefloat_internal_exception {
	explicit adaptivefloat_word_index_out_of_bounds(const std::string& error = "word index out of bounds") 
		: adaptivefloat_internal_exception(error) {}
};

}} // namespace sw::universal
