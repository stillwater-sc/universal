#pragma once
// exceptions.hpp: definition of arbitrary configuration double base number system exceptions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

struct dbns_arithmetic_exception : public universal_arithmetic_exception {
	dbns_arithmetic_exception(const std::string& err) : universal_arithmetic_exception(std::string("dbns arithmetic exception: ") + err) {}
};

// divide by zero arithmetic exception for reals
struct dbns_divide_by_zero : public dbns_arithmetic_exception {
	dbns_divide_by_zero() : dbns_arithmetic_exception("division by zero") {}
};

// negative argument to sqrt
struct dbns_negative_sqrt_arg : public dbns_arithmetic_exception {
	dbns_negative_sqrt_arg() : dbns_arithmetic_exception("negative sqrt argument") {}
};

///////////////////////////////////////////////////////////////
// internal implementation exceptions

// base class for internal exceptions
struct dbns_internal_exception : public universal_internal_exception {
	dbns_internal_exception(const std::string& error)
		: universal_internal_exception(std::string("dbns internal error: ") + error) {};
};

struct dbns_index_out_of_bounds : public dbns_internal_exception {
	dbns_index_out_of_bounds() : dbns_internal_exception("index out of bounds") {}
};

}} // namespace sw::universal
