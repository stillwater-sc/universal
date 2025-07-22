#pragma once
// exceptions.hpp: definition of exceptions for the multi-digit adaptive-precision ereal exceptions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

// base class for ereal arithmetic exceptions
struct ereal_arithmetic_exception : public universal_arithmetic_exception {
	ereal_arithmetic_exception(const std::string& err) : universal_arithmetic_exception(std::string("ereal arithmetic exception: ") + err) {};
};

//////////////////////////////////////////////////////////////////////////////////////////////////
/// specialized exceptions to aid application level exception handling

// not_a_number is thrown when a rvar is NaN
struct ereal_not_a_number : public ereal_arithmetic_exception {
	ereal_not_a_number() : ereal_arithmetic_exception("not a number") {}
};

// divide by zero arithmetic exception for reals
struct ereal_divide_by_zero : public ereal_arithmetic_exception {
	ereal_divide_by_zero() : ereal_arithmetic_exception("divide by zero") {}
};

// divide_by_nan is thrown when the denominator in a division operator is NaN
struct ereal_divide_by_nan : public ereal_arithmetic_exception {
	ereal_divide_by_nan() : ereal_arithmetic_exception("divide by nan") {}
};

// negative argument to sqrt
struct ereal_negative_sqrt_arg : public ereal_arithmetic_exception {
	ereal_negative_sqrt_arg() : ereal_arithmetic_exception("negative sqrt argument") {}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// REAL INTERNAL OPERATION EXCEPTIONS

struct ereal_internal_exception : public universal_internal_exception {
	ereal_internal_exception(const std::string& err) : universal_internal_exception(std::string("ereal internal exception: ") + err) {};
};


}} // namespace sw::universal
