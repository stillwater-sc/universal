#pragma once
// exceptions.hpp: definition of decimal integer exceptions
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <exception>
#include <universal/common/exceptions.hpp>

namespace sw::universal {

// base arithmetic error exception for decimal integer number system
struct decimal_arithmetic_error : public universal_arithmetic_error {
	decimal_arithmetic_error() : universal_arithmetic_error("decimal integer division by zero") {}
	decimal_arithmetic_error(const std::string& err) : universal_arithmetic_error(err) {}
};

// divide by zero arithmetic exception for decimal integers
struct decimal_integer_divide_by_zero : public decimal_arithmetic_error {
	decimal_integer_divide_by_zero() : decimal_arithmetic_error("decimal integer division by zero") {}
};

// sqrt negative argument arithmetic exception for integers
struct decimal_negative_sqrt_arg : public decimal_arithmetic_error {
	decimal_negative_sqrt_arg() : decimal_arithmetic_error("decimal integer negative sqrt argument") {}
};

///////////////////////////////////////////////////////////////
// internal implementation exceptions

// base internal error exception for decimal integer number system
struct decimal_internal_error : public universal_internal_error {
	decimal_internal_error() : universal_internal_error("default decimal integer internal error") {}
	decimal_internal_error(const std::string& err) : universal_internal_error(err) {}
};

struct decimal_integer_byte_index_out_of_bounds : public decimal_internal_error {
	decimal_integer_byte_index_out_of_bounds() : decimal_internal_error("decimal integer internal error: byte index out of bounds") {}
};

} // namespace sw::universal
