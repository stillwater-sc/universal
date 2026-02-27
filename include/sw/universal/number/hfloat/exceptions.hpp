#pragma once
// exceptions.hpp: definition of arbitrary configuration hfloat exceptions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

// base class for hfloat arithmetic exceptions
struct hfloat_arithmetic_exception : public universal_arithmetic_exception {
	hfloat_arithmetic_exception(const std::string& err) : universal_arithmetic_exception(std::string("hfloat arithmetic exception: ") + err) {};
};

///////////////////////////////////////////////////////////////////////////////
/// specialized exceptions to aid application level exception handling

// divide by zero arithmetic exception for hfloat
struct hfloat_divide_by_zero : public hfloat_arithmetic_exception {
	hfloat_divide_by_zero() : hfloat_arithmetic_exception("divide by zero") {}
};

// overflow: result too large to represent
struct hfloat_overflow : public hfloat_arithmetic_exception {
	hfloat_overflow() : hfloat_arithmetic_exception("overflow") {}
};

// underflow: result too small to represent
struct hfloat_underflow : public hfloat_arithmetic_exception {
	hfloat_underflow() : hfloat_arithmetic_exception("underflow") {}
};

// negative argument to sqrt
struct hfloat_negative_sqrt_arg : public hfloat_arithmetic_exception {
	hfloat_negative_sqrt_arg() : hfloat_arithmetic_exception("negative sqrt argument") {}
};

///////////////////////////////////////////////////////////////////////////////
/// INTERNAL OPERATION EXCEPTIONS

struct hfloat_internal_exception : public universal_internal_exception {
	hfloat_internal_exception(const std::string& err) : universal_internal_exception(std::string("hfloat internal exception: ") + err) {};
};

// NOTE: IBM System/360 HFP has no NaN, no infinity, no subnormals
// Overflow saturates to maxpos/maxneg
// These exceptions are only thrown when HFLOAT_THROW_ARITHMETIC_EXCEPTION is enabled

}} // namespace sw::universal
