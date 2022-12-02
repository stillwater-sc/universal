#pragma once
// exceptions.hpp: definition of eposit exceptions
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <exception>

namespace sw { namespace universal {

// base class for eposit arithmetic exceptions
struct eposit_arithmetic_exception
	: public std::runtime_error
{
	explicit eposit_arithmetic_exception(const std::string& error) 
		: std::runtime_error(std::string("elastic posit arithmetic exception: ") + error) {};
};

// divide by zero arithmetic exception for fixed-point
struct eposit_divide_by_zero : public eposit_arithmetic_exception {
	explicit eposit_divide_by_zero(const std::string& error = "elastic posit division by zero") 
		: eposit_arithmetic_exception(error) {}
};

///////////////////////////////////////////////////////////////
// internal implementation exceptions

// base class for fixed-point internal exceptions
struct eposit_internal_exception
	: public std::runtime_error
{
	explicit eposit_internal_exception(const std::string& error) 
		: std::runtime_error(std::string("elastic posit internal exception: ") + error) {};
};

struct eposit_word_index_out_of_bounds : public eposit_internal_exception {
	explicit eposit_word_index_out_of_bounds(const std::string& error = "word index out of bounds") 
		: eposit_internal_exception(error) {}
};

}} // namespace sw::universal
