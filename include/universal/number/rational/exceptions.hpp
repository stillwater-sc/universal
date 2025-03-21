#pragma once
// exceptions.hpp: definition of fixed-sized arbitrary configuration binary rational arithmetic exceptions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

struct rational_arithmetic_exception : public universal_arithmetic_exception {
	rational_arithmetic_exception(const std::string& err)
		: universal_arithmetic_exception(std::string("rational arithmetic exception: ") + err) {}
};

// divide by zero arithmetic exception for integers
struct rational_divide_by_zero : public rational_arithmetic_exception {
	rational_divide_by_zero() : rational_arithmetic_exception("division by zero") {}
};

// negative argument to sqrt
struct rational_negative_sqrt_arg : public rational_arithmetic_exception {
	rational_negative_sqrt_arg() : rational_arithmetic_exception("negative sqrt argument") {}
};

///////////////////////////////////////////////////////////////
// internal implementation exceptions

struct rational_internal_exception : public universal_internal_exception {
	rational_internal_exception(const std::string& err) 
		: universal_internal_exception(std::string("rational internal exception: ") + err) {}
};

struct rational_index_out_of_bounds : public rational_internal_exception {
	rational_index_out_of_bounds() : rational_internal_exception("index out of bounds") {}
};

}} // namespace sw::universal
