#pragma once
// exceptions.hpp: definition of arbitrary configuration double-double exceptions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

// base class for double-double arithmetic exceptions
struct dd_arithmetic_exception : public universal_arithmetic_exception {
	dd_arithmetic_exception(const std::string& err) : universal_arithmetic_exception(std::string("double-double arithmetic exception: ") + err) {};
};

//////////////////////////////////////////////////////////////////////////////////////////////////
/// specialized exceptions to aid application level exception handling

// invalid_argument is thrown when a mathematical function argument is invalid
struct dd_invalid_argument : public dd_arithmetic_exception {
	dd_invalid_argument() : dd_arithmetic_exception("invalid argument") {}
};

// not_a_number is thrown when a rvar is NaN
struct dd_not_a_number : public dd_arithmetic_exception {
	dd_not_a_number() : dd_arithmetic_exception("not a number") {}
};

// divide by zero arithmetic exception for reals
struct dd_divide_by_zero : public dd_arithmetic_exception {
	dd_divide_by_zero() : dd_arithmetic_exception("divide by zero") {}
};

// divide_by_nan is thrown when the denominator in a division operator is NaN
struct dd_divide_by_nan : public dd_arithmetic_exception {
	dd_divide_by_nan() : dd_arithmetic_exception("divide by nan") {}
};

// operand_is_nan is thrown when an rvar in a binary operator is NaN
struct dd_operand_is_nan : public dd_arithmetic_exception {
	dd_operand_is_nan() : dd_arithmetic_exception("operand is nan") {}
};

// negative argument to sqrt
struct dd_negative_sqrt_arg : public dd_arithmetic_exception {
	dd_negative_sqrt_arg() : dd_arithmetic_exception("negative sqrt argument") {}
};

// negative argument to nroot
struct dd_negative_nroot_arg : public dd_arithmetic_exception {
	dd_negative_nroot_arg() : dd_arithmetic_exception("negative nroot argument") {}
};


///////////////////////////////////////////////////////////////////////////////////////////////////
/// REAL INTERNAL OPERATION EXCEPTIONS

struct dd_internal_exception : public universal_internal_exception {
	dd_internal_exception(const std::string& err) : universal_internal_exception(std::string("double-double internal exception: ") + err) {};
};


}} // namespace sw::universal
