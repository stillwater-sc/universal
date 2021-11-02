#pragma once
// exceptions.hpp: definition of rational arithmetic exceptions
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <exception>
#include <universal/common/exceptions.hpp>

namespace sw::universal {

struct rational_arithmetic_error : public universal_arithmetic_error {
	rational_arithmetic_error() : universal_arithmetic_error("rational arithmetic error") {}
	rational_arithmetic_error(const std::string& err) : universal_arithmetic_error(err) {}
};

// divide by zero arithmetic exception for integers
struct rational_divide_by_zero : public rational_arithmetic_error {
	rational_divide_by_zero() : rational_arithmetic_error("rational division by zero") {}
};

// negative argument to sqrt
struct rational_negative_sqrt_arg : public rational_arithmetic_error {
	rational_negative_sqrt_arg() : rational_arithmetic_error("rational negative sqrt argument") {}
};

///////////////////////////////////////////////////////////////
// internal implementation exceptions

struct rational_internal_error : public universal_internal_error {
	rational_internal_error() : universal_internal_error("rational internal error") {}
	rational_internal_error(const std::string& err) : universal_internal_error(err) {}
};

struct rational_index_out_of_bounds : public rational_internal_error {
	rational_index_out_of_bounds() : rational_internal_error("index out of bounds") {}
};

} // namespace sw::universal
