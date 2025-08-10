#pragma once
// exceptions.hpp: definition of adaptive precision rational arithmetic exceptions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

struct erational_arithmetic_exception : public universal_arithmetic_exception {
	erational_arithmetic_exception(const std::string& err)
		: universal_arithmetic_exception(std::string("erational arithmetic exception: ") + err) {}
};

// divide by zero arithmetic exception for integers
struct erational_divide_by_zero : public erational_arithmetic_exception {
	erational_divide_by_zero() : erational_arithmetic_exception("division by zero") {}
};

// negative argument to sqrt
struct erational_negative_sqrt_arg : public erational_arithmetic_exception {
	erational_negative_sqrt_arg() : erational_arithmetic_exception("negative sqrt argument") {}
};

///////////////////////////////////////////////////////////////
// internal implementation exceptions

struct erational_internal_exception : public universal_internal_exception {
	erational_internal_exception(const std::string& err) 
		: universal_internal_exception(std::string("erational internal exception: ") + err) {}
};

struct erational_index_out_of_bounds : public erational_internal_exception {
	erational_index_out_of_bounds() : erational_internal_exception("index out of bounds") {}
};

}} // namespace sw::universal
