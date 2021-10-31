#pragma once
// exceptions.hpp: definition of rational arithmetic exceptions
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <exception>

namespace sw::universal {

struct rational_arithmetic_error : public std::runtime_error {
	rational_arithmetic_error() : std::runtime_error("rational arithmetic error") {}
	rational_arithmetic_error(const std::string& err) : std::runtime_error(err) {}
};

// divide by zero arithmetic exception for integers
struct rational_divide_by_zero : public std::runtime_error {
	rational_divide_by_zero() : std::runtime_error("rational division by zero") {}
};

// negative argument to sqrt
struct rational_negative_sqrt_arg : public std::runtime_error {
	rational_negative_sqrt_arg() : std::runtime_error("rational negative sqrt argument") {}
};

///////////////////////////////////////////////////////////////
// internal implementation exceptions

struct rational_internal_error : public std::runtime_error {
	rational_internal_error() : std::runtime_error("rational internal error") {}
	rational_internal_error(const std::string& err) : std::runtime_error(err) {}
};

struct rational_index_out_of_bounds : public rational_internal_error {
	rational_index_out_of_bounds() : rational_internal_error("index out of bounds") {}
};

} // namespace sw::universal
