#pragma once
// exceptions.hpp: definition of integer exceptions
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

struct integer_arithmetic_exception : public universal_arithmetic_exception {
	integer_arithmetic_exception(const std::string& err) : universal_arithmetic_exception(std::string("integer arithmetic exception: ") + err) {}
};

struct integer_encoding_exception : public universal_arithmetic_exception {
	integer_encoding_exception(const std::string& err) : universal_arithmetic_exception(std::string("integer encoding exception: ") + err) {}
};

// divide by zero arithmetic exception for integers
struct integer_divide_by_zero : public integer_arithmetic_exception {
	integer_divide_by_zero() : integer_arithmetic_exception("division by zero") {}
};

// overflow exception for integers
struct integer_overflow : public integer_arithmetic_exception {
	integer_overflow() : integer_arithmetic_exception("overflow") {}
};

// negative argument to a sqrt function
struct integer_negative_sqrt_arg : public integer_arithmetic_exception {
	integer_negative_sqrt_arg() : integer_arithmetic_exception("negative input argument to sqrt function") {}
};

// encoding exception for Whole Integers
struct integer_wholenumber_cannot_be_zero : public integer_encoding_exception {
	integer_wholenumber_cannot_be_zero() : integer_encoding_exception("whole numbers can't be zero") {}
};

// encoding exception for Whole Integers
struct integer_wholenumber_cannot_be_negative : public integer_encoding_exception {
	integer_wholenumber_cannot_be_negative() : integer_encoding_exception("whole numbers can't be negative") {}
};

// encoding exception for Whole Integers
struct integer_naturalnumber_cannot_be_negative : public integer_encoding_exception {
	integer_naturalnumber_cannot_be_negative() : integer_encoding_exception("natural numbers can't be negative") {}
};

///////////////////////////////////////////////////////////////
// internal implementation exceptions

struct integer_internal_exception : public universal_internal_exception {
	integer_internal_exception(const std::string& err) : universal_internal_exception(std::string("integer internal exception: ") + err) {}
};

struct integer_byte_index_out_of_bounds : public integer_internal_exception {
	integer_byte_index_out_of_bounds() : integer_internal_exception("byte index out of bounds") {}
};

}} // namespace sw::universal
