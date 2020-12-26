#pragma once
// exceptions.hpp: definition of arbitrary configuration real exceptions
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <stdexcept>

namespace sw { namespace universal {

///////////////////////////////////////////////////////////////////////////////////////////////////
/// REAL ARITHMETIC EXCEPTIONS

// base class for real arithmetic exceptions
struct real_arithmetic_exception
	: public std::runtime_error
{
	real_arithmetic_exception(const std::string& error) : std::runtime_error(std::string("real arithmetic exception: ") + error) {};
};

//////////////////////////////////////////////////////////////////////////////////////////////////
/// specialized exceptions to aid application level exception handling

// not_a_real is thrown when a rvar is NaR
struct real_not_a_number
	: real_arithmetic_exception
{
	real_not_a_number(const std::string& error = "NaN (Not a Number)") : real_arithmetic_exception(error) {}
};

// divide by zero arithmetic exception for reals
struct real_divide_by_zero
	: real_arithmetic_exception
{
	real_divide_by_zero(const std::string& error = "real division by zero") : real_arithmetic_exception(error) {}
};

// divide_by_nar is thrown when the denominator in a division operator is NaR
struct real_divide_by_nan
	: real_arithmetic_exception
{
	real_divide_by_nan(const std::string& error = "divide by NaN") : real_arithmetic_exception(error) {}
};

// numerator_is_nar is thrown when the numerator in a division operator is NaR
struct real_numerator_is_nar
	: real_arithmetic_exception
{
	real_numerator_is_nar(const std::string& error = "numerator is nar") : real_arithmetic_exception(error) {}
};

// operand_is_nar is thrown when an rvar in a binary operator is NaR
struct real_operand_is_nar
	: public real_arithmetic_exception
{
	real_operand_is_nar(const std::string& error = "operand is nar") : real_arithmetic_exception(error) {}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// REAL INTERNAL OPERATION EXCEPTIONS

struct real_internal_exception
	: public std::runtime_error
{
	real_internal_exception(const std::string& error) : std::runtime_error(std::string("real internal exception") + error) {};

};

struct real_shift_too_large
	: real_internal_exception
{
	real_shift_too_large(const std::string& error = "shift value too large for given posit") : real_internal_exception(error) {}
};

struct real_hpos_too_large
	: real_internal_exception
{
	real_hpos_too_large(const std::string& error = "position of hidden bit too large for given posit") : real_internal_exception(error) {}
};

struct real_rbits_too_large
	: real_internal_exception
{
	real_rbits_too_large(const std::string& error = "number of remaining bits too large for this fraction") :real_internal_exception(error) {}
};

}} // namespace sw::universal
