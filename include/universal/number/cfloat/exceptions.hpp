#pragma once
// exceptions.hpp: definition of arbitrary configuration cfloat exceptions
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <stdexcept>

namespace sw::universal {

///////////////////////////////////////////////////////////////////////////////////////////////////
/// REAL ARITHMETIC EXCEPTIONS

// base class for real arithmetic exceptions
struct cfloat_arithmetic_exception
	: public std::runtime_error
{
	cfloat_arithmetic_exception(const std::string& error) : std::runtime_error(std::string("real arithmetic exception: ") + error) {};
};

//////////////////////////////////////////////////////////////////////////////////////////////////
/// specialized exceptions to aid application level exception handling

// not_a_real is thrown when a rvar is NaN
struct cfloat_not_a_number
	: cfloat_arithmetic_exception
{
	cfloat_not_a_number(const std::string& error = "NaN (Not a Number)") : cfloat_arithmetic_exception(error) {}
};

// divide by zero arithmetic exception for reals
struct cfloat_divide_by_zero
	: cfloat_arithmetic_exception
{
	cfloat_divide_by_zero(const std::string& error = "real division by zero") : cfloat_arithmetic_exception(error) {}
};

// divide_by_nar is thrown when the denominator in a division operator is NaN
struct cfloat_divide_by_nan
	: cfloat_arithmetic_exception
{
	cfloat_divide_by_nan(const std::string& error = "divide by NaN") : cfloat_arithmetic_exception(error) {}
};

// numerator_is_nar is thrown when the numerator in a division operator is NaN
struct cfloat_numerator_is_nan
	: cfloat_arithmetic_exception
{
	cfloat_numerator_is_nan(const std::string& error = "numerator is nar") : cfloat_arithmetic_exception(error) {}
};

// operand_is_nar is thrown when an rvar in a binary operator is NaN
struct cfloat_operand_is_nan
	: public cfloat_arithmetic_exception
{
	cfloat_operand_is_nan(const std::string& error = "operand is nar") : cfloat_arithmetic_exception(error) {}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// REAL INTERNAL OPERATION EXCEPTIONS

struct cfloat_internal_exception
	: public std::runtime_error
{
	cfloat_internal_exception(const std::string& error) : std::runtime_error(std::string("real internal exception") + error) {};

};

struct cfloat_shift_too_large
	: cfloat_internal_exception
{
	cfloat_shift_too_large(const std::string& error = "shift value too large for given posit") : cfloat_internal_exception(error) {}
};

struct cfloat_hpos_too_large
	: cfloat_internal_exception
{
	cfloat_hpos_too_large(const std::string& error = "position of hidden bit too large for given posit") : cfloat_internal_exception(error) {}
};

struct cfloat_rbits_too_large
	: cfloat_internal_exception
{
	cfloat_rbits_too_large(const std::string& error = "number of remaining bits too large for this fraction") :cfloat_internal_exception(error) {}
};

} // namespace sw::universal
