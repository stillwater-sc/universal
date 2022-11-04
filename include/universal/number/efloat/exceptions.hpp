#pragma once
// exceptions.hpp: definition of elastic floating-point exceptions
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <exception>

namespace sw { namespace universal {

// base class for elastic float arithmetic exceptions
struct efloat_arithmetic_exception
	: public std::runtime_error
{
	explicit efloat_arithmetic_exception(const std::string& error) 
		: std::runtime_error(std::string("efloat arithmetic exception: ") + error) {};
};

// divide by zero arithmetic exception for elastic float
struct efloat_divide_by_zero : public efloat_arithmetic_exception {
	explicit efloat_divide_by_zero(const std::string& error = "efloat division by zero") 
		: efloat_arithmetic_exception(error) {}
};

///////////////////////////////////////////////////////////////
// internal implementation exceptions

// base class for elastic float internal exceptions
struct efloat_internal_exception
	: public std::runtime_error
{
	explicit efloat_internal_exception(const std::string& error) 
		: std::runtime_error(std::string("efloat internal exception: ") + error) {};
};

struct efloat_limb_index_out_of_bounds : public efloat_internal_exception {
	explicit efloat_limb_index_out_of_bounds(const std::string& error = "limb index out of bounds") 
		: efloat_internal_exception(error) {}
};

}} // namespace sw::universal
