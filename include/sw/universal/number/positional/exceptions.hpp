#pragma once
// exceptions.hpp: definition of positional integer arithmetic exceptions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

struct positional_arithmetic_exception : public universal_arithmetic_exception {
	positional_arithmetic_exception(const std::string& err)
		: universal_arithmetic_exception(std::string("positional arithmetic exception: ") + err) {}
};

// divide by zero arithmetic exception for positional integers
struct positional_divide_by_zero : public positional_arithmetic_exception {
	positional_divide_by_zero() : positional_arithmetic_exception("division by zero") {}
};

// negative argument to sqrt
struct positional_negative_sqrt_arg : public positional_arithmetic_exception {
	positional_negative_sqrt_arg() : positional_arithmetic_exception("negative sqrt argument") {}
};

///////////////////////////////////////////////////////////////
// internal implementation exceptions

struct positional_internal_exception : public universal_internal_exception {
	positional_internal_exception(const std::string& err)
		: universal_internal_exception(std::string("positional internal exception: ") + err) {}
};

struct positional_index_out_of_bounds : public positional_internal_exception {
	positional_index_out_of_bounds() : positional_internal_exception("index out of bounds") {}
};

}} // namespace sw::universal
