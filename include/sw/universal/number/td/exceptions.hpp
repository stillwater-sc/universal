#pragma once
// exceptions.hpp: definition of arbitrary configuration triple-double exceptions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

// base class for triple-double arithmetic exceptions
struct td_arithmetic_exception : public universal_arithmetic_exception {
	td_arithmetic_exception(const std::string& err) : universal_arithmetic_exception(std::string("triple-double arithmetic exception: ") + err) {};
};

//////////////////////////////////////////////////////////////////////////////////////////////////
/// specialized exceptions to aid application level exception handling

// invalid_argument is thrown when a mathematical function argument is invalid
struct td_invalid_argument : public td_arithmetic_exception {
	td_invalid_argument() : td_arithmetic_exception("invalid argument") {}
};

// not_a_number is thrown when a rvar is NaN
struct td_not_a_number : public td_arithmetic_exception {
	td_not_a_number() : td_arithmetic_exception("not a number") {}
};

// divide by zero arithmetic exception for reals
struct td_divide_by_zero : public td_arithmetic_exception {
	td_divide_by_zero() : td_arithmetic_exception("divide by zero") {}
};

// divide_by_nan is thrown when the denominator in a division operator is NaN
struct td_divide_by_nan : public td_arithmetic_exception {
	td_divide_by_nan() : td_arithmetic_exception("divide by nan") {}
};

// operand_is_nan is thrown when an rvar in a binary operator is NaN
struct td_operand_is_nan : public td_arithmetic_exception {
	td_operand_is_nan() : td_arithmetic_exception("operand is nan") {}
};

// negative argument to sqrt
struct td_negative_sqrt_arg : public td_arithmetic_exception {
	td_negative_sqrt_arg() : td_arithmetic_exception("negative sqrt argument") {}
};

// negative argument to nroot
struct td_negative_nroot_arg : public td_arithmetic_exception {
	td_negative_nroot_arg() : td_arithmetic_exception("negative nroot argument") {}
};


///////////////////////////////////////////////////////////////////////////////////////////////////
/// REAL INTERNAL OPERATION EXCEPTIONS

struct td_internal_exception : public universal_internal_exception {
	td_internal_exception(const std::string& err) : universal_internal_exception(std::string("triple-double internal exception: ") + err) {};
};


}} // namespace sw::universal
