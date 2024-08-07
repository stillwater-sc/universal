#pragma once
// exceptions.hpp: definition of arbitrary configuration areal exceptions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

// base class for areal arithmetic exceptions
struct areal_arithmetic_exception : public universal_arithmetic_exception {
	areal_arithmetic_exception(const std::string& err) : universal_arithmetic_exception(std::string("areal arithmetic exception: ") + err) {};
};

//////////////////////////////////////////////////////////////////////////////////////////////////
/// specialized exceptions to aid application level exception handling

// not_a_real is thrown when a rvar is NaN
struct areal_not_a_number : public areal_arithmetic_exception {
	areal_not_a_number() : areal_arithmetic_exception("not a number") {}
};

// divide by zero arithmetic exception for reals
struct areal_divide_by_zero : public areal_arithmetic_exception {
	areal_divide_by_zero() : areal_arithmetic_exception("divide by zero") {}
};

// divide_by_nan is thrown when the denominator in a division operator is NaN
struct areal_divide_by_nan : public areal_arithmetic_exception {
	areal_divide_by_nan() : areal_arithmetic_exception("divide by nan") {}
};

// operand_is_nan is thrown when an rvar in a binary operator is NaN
struct areal_operand_is_nan : public areal_arithmetic_exception {
	areal_operand_is_nan() : areal_arithmetic_exception("operand is nan") {}
};

// negative argument to sqrt
struct areal_negative_sqrt_arg : public areal_arithmetic_exception {
	areal_negative_sqrt_arg() : areal_arithmetic_exception("negative sqrt argument") {}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// REAL INTERNAL OPERATION EXCEPTIONS

struct areal_internal_exception : public universal_internal_exception {
	areal_internal_exception(const std::string& err) : universal_internal_exception(std::string("areal internal exception: ") + err) {};
};

struct areal_shift_too_large : public areal_internal_exception {
	areal_shift_too_large() : areal_internal_exception("shift value too large for given areal") {}
};

struct areal_hpos_too_large : public areal_internal_exception {
	areal_hpos_too_large() : areal_internal_exception("position of hidden bit too large for given areal") {}
};

}} // namespace sw::universal
