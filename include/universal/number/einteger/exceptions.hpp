#pragma once
// exceptions.hpp: definition of arbitrary precision integer exceptions
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

// base class for arbitrary precision integer arithmetic exceptions
struct einteger_arithmetic_exception
	: public std::runtime_error
{
	explicit einteger_arithmetic_exception(const std::string& error) 
		: std::runtime_error(std::string("einteger arithmetic exception: ") + error) {};
};

// divide by zero arithmetic exception for arbitrary precision integer
struct einteger_divide_by_zero : public einteger_arithmetic_exception {
	explicit einteger_divide_by_zero(const std::string& error = "einteger division by zero") 
		: einteger_arithmetic_exception(error) {}
};

///////////////////////////////////////////////////////////////
// internal implementation exceptions

// base class for arbitrary precision integer internal exceptions
struct einteger_internal_exception
	: public std::runtime_error
{
	explicit einteger_internal_exception(const std::string& error) 
		: std::runtime_error(std::string("einteger internal exception: ") + error) {};
};

struct einteger_word_index_out_of_bounds : public einteger_internal_exception {
	explicit einteger_word_index_out_of_bounds(const std::string& error = "word index out of bounds") 
		: einteger_internal_exception(error) {}
};

}} // namespace sw::universal
