#pragma once
// decimal_exceptions.hpp: definition of decimal integer exceptions
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <exception>

namespace sw {
namespace unum {

// divide by zero arithmetic exception for integers
struct decimal_integer_divide_by_zero : public std::runtime_error {
	decimal_integer_divide_by_zero() : std::runtime_error("decimal_integer division by zero") {}
};

///////////////////////////////////////////////////////////////
// internal implementation exceptions

struct decimal_integer_byte_index_out_of_bounds : public std::runtime_error {
	decimal_integer_byte_index_out_of_bounds() : std::runtime_error("decimal integer internal error: byte index out of bounds") {}
};

} // namespace unum
} // namespace sw
