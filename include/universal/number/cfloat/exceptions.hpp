#pragma once
// exceptions.hpp: definition of arbitrary configuration cfloat exceptions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

// base class for cfloat arithmetic exceptions
struct cfloat_arithmetic_exception : public universal_arithmetic_exception {
	cfloat_arithmetic_exception(const std::string& err) : universal_arithmetic_exception(std::string("cfloat arithmetic exception: ") + err) {};
};

// base class for cfloat quire arithmetic exceptions
struct cfloat_quire_exception : public cfloat_arithmetic_exception {
	cfloat_quire_exception(const std::string& err) : cfloat_arithmetic_exception(std::string("cfloat quire exception: ") + err) {}
};

//////////////////////////////////////////////////////////////////////////////////////////////////
/// specialized exceptions to aid application level exception handling

// cfloat_not_a_number is thrown when a rvar is NaN
struct cfloat_not_a_number : public cfloat_arithmetic_exception {
	cfloat_not_a_number() : cfloat_arithmetic_exception("not a number") {}
};

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

// quire_operand_is_nan is thrown when an rvar in a binary operator is NaN
struct cfloat_quire_operand_is_nan : public cfloat_quire_exception {
	cfloat_quire_operand_is_nan() : cfloat_quire_exception("quire operand is nan") {}
};

// negative argument to sqrt
struct cfloat_quire_negative_sqrt_arg : public cfloat_quire_exception {
	cfloat_quire_negative_sqrt_arg() : cfloat_quire_exception("quire negative sqrt argument") {}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// cfloat internal operation exceptions

struct cfloat_internal_exception : public universal_internal_exception {
	cfloat_internal_exception(const std::string& err) : universal_internal_exception(std::string("cfloat internal exception: ") + err) {};
};

struct cfloat_shift_too_large : public cfloat_internal_exception {
	cfloat_shift_too_large() : cfloat_internal_exception("shift value too large for given cfloat") {}
};

struct cfloat_hpos_too_large : public cfloat_internal_exception {
	cfloat_hpos_too_large() : cfloat_internal_exception("position of hidden bit too large for given cfloat") {}
};

//struct cfloat_rbits_too_large : cfloat_internal_exception {
//	cfloat_rbits_too_large(const std::string& error = "number of remaining bits too large for this fraction") :cfloat_internal_exception(error) {}
//};

}} // namespace sw::universal
