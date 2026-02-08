#pragma once
// exceptions.hpp: definition of interval arithmetic exceptions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

// base class for interval arithmetic exceptions
struct interval_arithmetic_exception : public universal_arithmetic_exception {
	interval_arithmetic_exception(const std::string& err) : universal_arithmetic_exception(std::string("interval arithmetic exception: ") + err) {}
};

// divide by zero arithmetic exception for interval
struct interval_divide_by_zero : public interval_arithmetic_exception {
	interval_divide_by_zero() : interval_arithmetic_exception("division by zero") {}
};

// negative argument to sqrt
struct interval_negative_sqrt_arg : public interval_arithmetic_exception {
	interval_negative_sqrt_arg() : interval_arithmetic_exception("negative sqrt argument") {}
};

///////////////////////////////////////////////////////////////
// internal implementation exceptions

// base class for interval internal exceptions
struct interval_internal_exception : public universal_internal_exception {
	interval_internal_exception(const std::string& err) : universal_internal_exception(std::string("interval internal exception: ") + err) {}
};

struct interval_index_out_of_bounds : public interval_internal_exception {
	interval_index_out_of_bounds() : interval_internal_exception("index out of bounds") {}
};

}} // namespace sw::universal
