#pragma once
// exceptions.hpp: definition of integer exceptions
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/common/exceptions.hpp>

namespace sw::universal {

struct integer_arithmetic_exception : public universal_arithmetic_exception {
	integer_arithmetic_exception(const std::string& err) : universal_arithmetic_exception(std::string("integer arithmetic exception: ") + err) {}
};

// divide by zero arithmetic exception for integers
struct integer_divide_by_zero : public integer_arithmetic_exception {
	integer_divide_by_zero() : integer_arithmetic_exception("division by zero") {}
};

// overflow exception for integers
struct integer_overflow : public integer_arithmetic_exception {
	integer_overflow() : integer_arithmetic_exception("overflow") {}
};

///////////////////////////////////////////////////////////////
// internal implementation exceptions

struct integer_internal_exception : public universal_internal_exception {
	integer_internal_exception(const std::string& err) : universal_internal_exception(std::string("integer internal exception: ") + err) {}
};

struct integer_byte_index_out_of_bounds : public integer_internal_exception {
	integer_byte_index_out_of_bounds() : integer_internal_exception("byte index out of bounds") {}
};

} // namespace sw::universal
