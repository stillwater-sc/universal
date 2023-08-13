#pragma once
// exceptions.hpp: definition of decimal integer exceptions
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

// base arithmetic error exception for decimal integer number system
struct decimal_arithmetic_exception : public universal_arithmetic_exception {
	explicit decimal_arithmetic_exception(const std::string& err)
		: universal_arithmetic_exception(std::string("decimal integer exception: ") + err) {}
};

// divide by zero arithmetic exception for decimal integers
struct decimal_integer_divide_by_zero : public decimal_arithmetic_exception {
	decimal_integer_divide_by_zero() : decimal_arithmetic_exception("division by zero") {}
};

// sqrt negative argument arithmetic exception for integers
struct decimal_negative_sqrt_arg : public decimal_arithmetic_exception {
	decimal_negative_sqrt_arg() : decimal_arithmetic_exception("negative sqrt argument") {}
};

///////////////////////////////////////////////////////////////
// internal implementation exceptions

// base internal error exception for decimal integer number system
struct decimal_internal_exception : public universal_internal_exception {
	explicit decimal_internal_exception(const std::string& err) 
		: universal_internal_exception(std::string("decimal integer internal error: ") + err) {}
};

struct decimal_integer_byte_index_out_of_bounds : public decimal_internal_exception {
	decimal_integer_byte_index_out_of_bounds() : decimal_internal_exception("byte index out of bounds") {}
};

}} // namespace sw::universal
