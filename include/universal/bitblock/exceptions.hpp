#pragma once
// exceptions.hpp: exceptions for problems in bitblock calculations
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <stdexcept>
#include <string>

namespace sw { namespace unum {

///////////////////////////////////////////////////////////////////////////////////////////////////
/// BITBLOCK ARITHMETIC EXCEPTIONS

// base class for bitblock arithmetic exceptions
struct bitblock_arithmetic_exception
	: public std::runtime_error
{
	bitblock_arithmetic_exception(const std::string& error) : std::runtime_error(std::string("bitblock arithmetic exception: ") + error) {};
};

//////////////////////////////////////////////////////////////////////////////////////////////////
/// specialized exceptions to aid application level exception handling

// is thrown when denominator is 0 in a division operator
struct bitblock_divide_by_zero
	: public bitblock_arithmetic_exception
{
	bitblock_divide_by_zero(const std::string& error = "bitblock divide by zero") : bitblock_arithmetic_exception(error) {}
};

}} // namespace sw::unum
