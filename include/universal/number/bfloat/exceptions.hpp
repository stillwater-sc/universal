#pragma once
// exceptions.hpp: definition of arbitrary configuration real exceptions
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <stdexcept>

namespace sw::universal {

///////////////////////////////////////////////////////////////////////////////////////////////////
/// REAL ARITHMETIC EXCEPTIONS

// base class for real arithmetic exceptions
struct bfloat_arithmetic_exception
	: public std::runtime_error
{
	bfloat_arithmetic_exception(const std::string& error) : std::runtime_error(std::string("real arithmetic exception: ") + error) {};
};

//////////////////////////////////////////////////////////////////////////////////////////////////
/// specialized exceptions to aid application level exception handling

// not_a_real is thrown when a rvar is NaR
struct bfloat_not_a_number
	: bfloat_arithmetic_exception
{
	bfloat_not_a_number(const std::string& error = "NaN (Not a Number)") : bfloat_arithmetic_exception(error) {}
};

// divide by zero arithmetic exception for reals
struct bfloat_divide_by_zero
	: bfloat_arithmetic_exception
{
	bfloat_divide_by_zero(const std::string& error = "real division by zero") : bfloat_arithmetic_exception(error) {}
};

// divide_by_nar is thrown when the denominator in a division operator is NaR
struct bfloat_divide_by_nan
	: bfloat_arithmetic_exception
{
	bfloat_divide_by_nan(const std::string& error = "divide by NaN") : bfloat_arithmetic_exception(error) {}
};

// numerator_is_nar is thrown when the numerator in a division operator is NaR
struct bfloat_numerator_is_nar
	: bfloat_arithmetic_exception
{
	bfloat_numerator_is_nar(const std::string& error = "numerator is nar") : bfloat_arithmetic_exception(error) {}
};

// operand_is_nar is thrown when an rvar in a binary operator is NaR
struct bfloat_operand_is_nar
	: public bfloat_arithmetic_exception
{
	bfloat_operand_is_nar(const std::string& error = "operand is nar") : bfloat_arithmetic_exception(error) {}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// REAL INTERNAL OPERATION EXCEPTIONS

struct bfloat_internal_exception
	: public std::runtime_error
{
	bfloat_internal_exception(const std::string& error) : std::runtime_error(std::string("real internal exception") + error) {};

};

struct bfloat_shift_too_large
	: bfloat_internal_exception
{
	bfloat_shift_too_large(const std::string& error = "shift value too large for given posit") : bfloat_internal_exception(error) {}
};

struct bfloat_hpos_too_large
	: bfloat_internal_exception
{
	bfloat_hpos_too_large(const std::string& error = "position of hidden bit too large for given posit") : bfloat_internal_exception(error) {}
};

struct bfloat_rbits_too_large
	: bfloat_internal_exception
{
	bfloat_rbits_too_large(const std::string& error = "number of remaining bits too large for this fraction") :bfloat_internal_exception(error) {}
};

} // namespace sw::universal
