#pragma once
// exceptions.hpp: definition of arbitrary configuration cfloat exceptions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>        // std::string
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

// base class for cfloat arithmetic exceptions
struct cfloat_arithmetic_exception : public universal_arithmetic_exception {
	cfloat_arithmetic_exception(const std::string& err) : universal_arithmetic_exception(std::string("cfloat arithmetic exception: ") + err) {};
};

//////////////////////////////////////////////////////////////////////////////////////////////////
/// specialized exceptions to aid application level exception handling

// divide by zero arithmetic exception for reals
struct cfloat_divide_by_zero : public cfloat_arithmetic_exception {
	cfloat_divide_by_zero() : cfloat_arithmetic_exception("divide by zero") {}
};

// divide_by_nan is thrown when the denominator in a division operator is NaN
struct cfloat_divide_by_nan : public cfloat_arithmetic_exception {
	cfloat_divide_by_nan() : cfloat_arithmetic_exception("divide by nan") {}
};

// operand_is_nan is thrown when an rvar in a binary operator is NaN
struct cfloat_operand_is_nan : public cfloat_arithmetic_exception {
	cfloat_operand_is_nan() : cfloat_arithmetic_exception("operand is nan") {}
};

// negative argument to sqrt
struct cfloat_negative_sqrt_arg : public cfloat_arithmetic_exception {
	cfloat_negative_sqrt_arg() : cfloat_arithmetic_exception("negative sqrt argument") {}
};

// cfloat raises only these: the quire, NaN-operand, shift and hidden-bit exceptions that
// used to be declared here were never thrown, and cfloat has no internal exceptions (#2089).

}} // namespace sw::universal
