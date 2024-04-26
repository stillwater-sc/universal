#pragma once
// fixpnt_exceptions.hpp: definition of fixed-point exceptions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

// base class for fixed-point arithmetic exceptions
struct fixpnt_arithmetic_exception : public universal_arithmetic_exception {
	explicit fixpnt_arithmetic_exception(const std::string& error) 
		: universal_arithmetic_exception(std::string("fixed-point arithmetic exception: ") + error) {};
};

// divide by zero arithmetic exception for fixed-point
struct fixpnt_divide_by_zero : public fixpnt_arithmetic_exception {
	fixpnt_divide_by_zero() : fixpnt_arithmetic_exception("division by zero") {}
};

// overflow exception for fixed-point
struct fixpnt_overflow : public fixpnt_arithmetic_exception {
	fixpnt_overflow() : fixpnt_arithmetic_exception("overflow") {}
};

// negative argument to sqrt
struct fixpnt_negative_sqrt_arg : public fixpnt_arithmetic_exception {
	fixpnt_negative_sqrt_arg() : fixpnt_arithmetic_exception("negative sqrt argument") {}
};

///////////////////////////////////////////////////////////////
// internal implementation exceptions

// base class for fixed-point internal exceptions
struct fixpnt_internal_exception : public universal_internal_exception {
	explicit fixpnt_internal_exception(const std::string& error) 
		: universal_internal_exception(std::string("fixed-point internal error: ") + error) {};
};

struct fixpnt_index_out_of_bounds : public fixpnt_internal_exception {
	explicit fixpnt_index_out_of_bounds() : fixpnt_internal_exception("index out of bounds") {}
};

}} // namespace sw::universal
