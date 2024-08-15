#pragma once
// exceptions.hpp: definition of arbitrary configuration quad-double exceptions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

// base class for quad-double arithmetic exceptions
struct qd_arithmetic_exception : public universal_arithmetic_exception {
	qd_arithmetic_exception(const std::string& err) : universal_arithmetic_exception(std::string("quad-double arithmetic exception: ") + err) {};
};

//////////////////////////////////////////////////////////////////////////////////////////////////
/// specialized exceptions to aid application level exception handling

// invalid_argument is thrown when a mathematical function argument is invalid
struct qd_invalid_argument : public qd_arithmetic_exception {
	qd_invalid_argument() : qd_arithmetic_exception("invalid argument") {}
};

// not_a_number is thrown when a lvar is NaN
struct qd_not_a_number : public qd_arithmetic_exception {
	qd_not_a_number() : qd_arithmetic_exception("not a number") {}
};

// divide by zero arithmetic exception for reals
struct qd_divide_by_zero : public qd_arithmetic_exception {
	qd_divide_by_zero() : qd_arithmetic_exception("divide by zero") {}
};

// divide_by_nan is thrown when the denominator in a division operator is NaN
struct qd_divide_by_nan : public qd_arithmetic_exception {
	qd_divide_by_nan() : qd_arithmetic_exception("divide by nan") {}
};

// operand_is_nan is thrown when an rvar in a binary operator is NaN
struct qd_operand_is_nan : public qd_arithmetic_exception {
	qd_operand_is_nan() : qd_arithmetic_exception("operand is nan") {}
};

// negative argument to sqrt
struct qd_negative_sqrt_arg : public qd_arithmetic_exception {
	qd_negative_sqrt_arg() : qd_arithmetic_exception("negative sqrt argument") {}
};

// negative argument to nroot
struct qd_negative_nroot_arg : public qd_arithmetic_exception {
	qd_negative_nroot_arg() : qd_arithmetic_exception("negative nroot argument") {}
};


///////////////////////////////////////////////////////////////////////////////////////////////////
/// REAL INTERNAL OPERATION EXCEPTIONS

struct qd_internal_exception : public universal_internal_exception {
	qd_internal_exception(const std::string& err) : universal_internal_exception(std::string("quad-double internal exception: ") + err) {};
};


}} // namespace sw::universal
