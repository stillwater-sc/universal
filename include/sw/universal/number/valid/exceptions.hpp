#pragma once
// exceptions.hpp: definition of arbitrary configuration valid number system exceptions
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

struct valid_arithmetic_exception : public universal_arithmetic_exception {
	valid_arithmetic_exception(const std::string& err) : universal_arithmetic_exception(std::string("valid arithmetic exception: ") + err) {}
};

// divide by zero arithmetic exception for reals
struct valid_divide_by_zero : public valid_arithmetic_exception {
	valid_divide_by_zero() : valid_arithmetic_exception("division by zero") {}
};

// negative argument to sqrt
struct valid_negative_sqrt_arg : public valid_arithmetic_exception {
	valid_negative_sqrt_arg() : valid_arithmetic_exception("negative sqrt argument") {}
};

///////////////////////////////////////////////////////////////
// internal implementation exceptions

// base class for internal exceptions
struct valid_internal_exception : public universal_internal_exception {
	valid_internal_exception(const std::string& error)
		: universal_internal_exception(std::string("valid internal error: ") + error) {};
};

struct valid_index_out_of_bounds : public valid_internal_exception {
	valid_index_out_of_bounds() : valid_internal_exception("index out of bounds") {}
};

}} // namespace sw::universal
