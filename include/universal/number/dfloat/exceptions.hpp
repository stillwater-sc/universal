#pragma once
// exceptions.hpp: definition of arbitrary configuration dfloat exceptions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

// base class for dfloat arithmetic exceptions
struct dfloat_arithmetic_exception : public universal_arithmetic_exception {
	dfloat_arithmetic_exception(const std::string& err) : universal_arithmetic_exception(std::string("dfloat arithmetic exception: ") + err) {};
};

// base class for dfloat quire arithmetic exceptions
struct dfloat_quire_exception : public dfloat_arithmetic_exception {
	dfloat_quire_exception(const std::string& err) : dfloat_arithmetic_exception(std::string("dfloat quire exception: ") + err) {}
};

//////////////////////////////////////////////////////////////////////////////////////////////////
/// specialized exceptions to aid application level exception handling

// not_a_real is thrown when a rvar is NaN
struct dfloat_not_a_number : public dfloat_arithmetic_exception {
	dfloat_not_a_number() : dfloat_arithmetic_exception("not a number") {}
};

// divide by zero arithmetic exception for reals
struct dfloat_divide_by_zero : public dfloat_arithmetic_exception {
	dfloat_divide_by_zero() : dfloat_arithmetic_exception("divide by zero") {}
};

// divide_by_nan is thrown when the denominator in a division operator is NaN
struct dfloat_divide_by_nan : public dfloat_arithmetic_exception {
	dfloat_divide_by_nan() : dfloat_arithmetic_exception("divide by nan") {}
};

// operand_is_nan is thrown when an rvar in a binary operator is NaN
struct dfloat_operand_is_nan : public dfloat_arithmetic_exception {
	dfloat_operand_is_nan() : dfloat_arithmetic_exception("operand is nan") {}
};

// negative argument to sqrt
struct dfloat_negative_sqrt_arg : public dfloat_arithmetic_exception {
	dfloat_negative_sqrt_arg() : dfloat_arithmetic_exception("negative sqrt argument") {}
};

// quire_operand_is_nan is thrown when an rvar in a binary operator is NaN
struct dfloat_quire_operand_is_nan : public dfloat_quire_exception {
	dfloat_quire_operand_is_nan() : dfloat_quire_exception("quire operand is nan") {}
};

// negative argument to sqrt
struct dfloat_quire_negative_sqrt_arg : public dfloat_quire_exception {
	dfloat_quire_negative_sqrt_arg() : dfloat_quire_exception("quire negative sqrt argument") {}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// REAL INTERNAL OPERATION EXCEPTIONS

struct dfloat_internal_exception : public universal_internal_exception {
	dfloat_internal_exception(const std::string& err) : universal_internal_exception(std::string("dfloat internal exception: ") + err) {};
};

struct dfloat_shift_too_large : public dfloat_internal_exception {
	dfloat_shift_too_large() : dfloat_internal_exception("shift value too large for given dfloat") {}
};

struct dfloat_hpos_too_large : public dfloat_internal_exception {
	dfloat_hpos_too_large() : dfloat_internal_exception("position of hidden bit too large for given dfloat") {}
};

//struct dfloat_rbits_too_large : dfloat_internal_exception {
//	dfloat_rbits_too_large(const std::string& error = "number of remaining bits too large for this fraction") :dfloat_internal_exception(error) {}
//};

}} // namespace sw::universal
