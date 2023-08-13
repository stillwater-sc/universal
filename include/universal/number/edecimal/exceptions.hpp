#pragma once
// exceptions.hpp: definition of adaptive precision decimal integer exceptions
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

// base arithmetic error exception for edecimal integer number system
struct edecimal_arithmetic_exception : public universal_arithmetic_exception {
	explicit edecimal_arithmetic_exception(const std::string& err)
		: universal_arithmetic_exception(std::string("edecimal integer exception: ") + err) {}
};

// divide by zero arithmetic exception for edecimal integers
struct edecimal_integer_divide_by_zero : public edecimal_arithmetic_exception {
	edecimal_integer_divide_by_zero() : edecimal_arithmetic_exception("division by zero") {}
};

// sqrt negative argument arithmetic exception for edecimal integers
struct edecimal_negative_sqrt_arg : public edecimal_arithmetic_exception {
	edecimal_negative_sqrt_arg() : edecimal_arithmetic_exception("negative sqrt argument") {}
};

///////////////////////////////////////////////////////////////
// internal implementation exceptions

// base internal error exception for edecimal integer number system
struct edecimal_internal_exception : public universal_internal_exception {
	explicit edecimal_internal_exception(const std::string& err) 
		: universal_internal_exception(std::string("edecimal integer internal error: ") + err) {}
};

struct edecimal_integer_byte_index_out_of_bounds : public edecimal_internal_exception {
	edecimal_integer_byte_index_out_of_bounds() : edecimal_internal_exception("digit index out of bounds") {}
};

}} // namespace sw::universal
