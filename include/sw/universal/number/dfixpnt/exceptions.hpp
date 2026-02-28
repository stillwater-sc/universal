#pragma once
// exceptions.hpp: definition of decimal fixed-point dfixpnt exceptions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

// base class for dfixpnt arithmetic exceptions
struct dfixpnt_arithmetic_exception : public universal_arithmetic_exception {
	explicit dfixpnt_arithmetic_exception(const std::string& error)
		: universal_arithmetic_exception(std::string("dfixpnt arithmetic exception: ") + error) {};
};

// divide by zero arithmetic exception for dfixpnt
struct dfixpnt_divide_by_zero : public dfixpnt_arithmetic_exception {
	dfixpnt_divide_by_zero() : dfixpnt_arithmetic_exception("division by zero") {}
};

// overflow exception for dfixpnt
struct dfixpnt_overflow : public dfixpnt_arithmetic_exception {
	dfixpnt_overflow() : dfixpnt_arithmetic_exception("overflow") {}
};

// negative argument to sqrt
struct dfixpnt_negative_sqrt_arg : public dfixpnt_arithmetic_exception {
	dfixpnt_negative_sqrt_arg() : dfixpnt_arithmetic_exception("negative sqrt argument") {}
};

///////////////////////////////////////////////////////////////
// internal implementation exceptions

struct dfixpnt_internal_exception : public universal_internal_exception {
	explicit dfixpnt_internal_exception(const std::string& error)
		: universal_internal_exception(std::string("dfixpnt internal error: ") + error) {};
};

struct dfixpnt_index_out_of_bounds : public dfixpnt_internal_exception {
	explicit dfixpnt_index_out_of_bounds() : dfixpnt_internal_exception("index out of bounds") {}
};

}} // namespace sw::universal
