#pragma once
// exceptions.hpp: exceptions for problems in bitblock calculations
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <stdexcept>
#include <string>

// this is a supporting class, and its exception behavior needs to be configure by the calling environment.
// this means that the value of BITBLOCK_THROW_ARITHMETIC_EXCEPTION must be set externally
// #define BITBLOCK_THROW_ARITHMETIC_EXCEPTION exception-config-of-calling-class
// TODO: do we want to support the use-case where two seperate classes that use bitblock<> 
// but desire different exception behavior?

namespace sw { namespace universal {

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

///////////////////////////////////////////////////////////////////////////////////////////////////
/// BITBLOCK INTERNAL EXCEPTIONS

// base class for bitblock internal exceptions
struct bitblock_internal_exception
	: public std::runtime_error
{
	bitblock_internal_exception(const std::string& error) : std::runtime_error(std::string("bitblock internal exception: ") + error) {};
};

struct iteration_bound_too_large
	: bitblock_internal_exception
{
	iteration_bound_too_large(const std::string& error = "iteration bound is too large") : bitblock_internal_exception(error) {}
};

struct round_off_all
	: bitblock_internal_exception
{
	round_off_all(const std::string& error = "cannot round off all bits") : bitblock_internal_exception(error) {}
};

struct cut_off_leading_bit
	: bitblock_internal_exception
{
	cut_off_leading_bit(const std::string& error = "leading significant bit is cut off") : bitblock_internal_exception(error) {}
};

///////////////////////////////////////////////////////////////////////////////////////////////////

}} // namespace sw::universal
