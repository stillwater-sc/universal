#pragma once
// exceptions.hpp: definition of bfloat exceptions
//
// Copyright (C) 2022-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <exception>

namespace sw { namespace universal {

// base class for bfloat arithmetic exceptions
struct bfloat_arithmetic_exception
	: public std::runtime_error
{
	explicit bfloat_arithmetic_exception(const std::string& error) 
		: std::runtime_error(std::string("bfloat arithmetic exception: ") + error) {};
};

// divide by zero arithmetic exception for fixed-point
struct bfloat_divide_by_zero : public bfloat_arithmetic_exception {
	explicit bfloat_divide_by_zero(const std::string& error = "bfloat division by zero") 
		: bfloat_arithmetic_exception(error) {}
};

///////////////////////////////////////////////////////////////
// internal implementation exceptions

// base class for fixed-point internal exceptions
struct bfloat_internal_exception
	: public std::runtime_error
{
	explicit bfloat_internal_exception(const std::string& error) 
		: std::runtime_error(std::string("bfloat internal exception: ") + error) {};
};

struct bfloat_word_index_out_of_bounds : public bfloat_internal_exception {
	explicit bfloat_word_index_out_of_bounds(const std::string& error = "word index out of bounds") 
		: bfloat_internal_exception(error) {}
};

}} // namespace sw::universal
