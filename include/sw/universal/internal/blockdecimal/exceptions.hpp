#pragma once
// exceptions.hpp: exceptions for problems in blockdecimal calculations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <stdexcept>
#include <string>

// this is a supporting class, and its exception behavior needs to be configured by the calling environment.
// this means that the value of BLOCKDECIMAL_THROW_ARITHMETIC_EXCEPTION must be set externally
// #define BLOCKDECIMAL_THROW_ARITHMETIC_EXCEPTION exception-config-of-calling-class

namespace sw { namespace universal {

///////////////////////////////////////////////////////////////////////////////////////////////////
/// BLOCKDECIMAL ARITHMETIC EXCEPTIONS

// base class for blockdecimal arithmetic exceptions
struct blockdecimal_arithmetic_exception
	: public std::runtime_error
{
	blockdecimal_arithmetic_exception(const std::string& error) : std::runtime_error(std::string("blockdecimal arithmetic exception: ") + error) {};
};

//////////////////////////////////////////////////////////////////////////////////////////////////
/// specialized exceptions to aid application level exception handling

// is thrown when denominator is 0 in a division operator
struct blockdecimal_divide_by_zero
	: public blockdecimal_arithmetic_exception
{
	blockdecimal_divide_by_zero(const std::string& error = "blockdecimal divide by zero") : blockdecimal_arithmetic_exception(error) {}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// BLOCKDECIMAL INTERNAL EXCEPTIONS

// base class for blockdecimal internal exceptions
struct blockdecimal_internal_exception
	: public std::runtime_error
{
	blockdecimal_internal_exception(const std::string& error) : std::runtime_error(std::string("blockdecimal internal exception: ") + error) {};
};

struct blockdecimal_index_out_of_bounds
	: blockdecimal_internal_exception
{
	blockdecimal_index_out_of_bounds(const std::string& error = "index out of bounds") : blockdecimal_internal_exception(error) {}
};

///////////////////////////////////////////////////////////////////////////////////////////////////

}} // namespace sw::universal
