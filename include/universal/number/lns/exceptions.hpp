#pragma once
// exceptions.hpp: definition of arbitrary configuration logarithmic number system exceptions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

struct lns_arithmetic_exception : public universal_arithmetic_exception {
	lns_arithmetic_exception(const std::string& err) : universal_arithmetic_exception(std::string("lns arithmetic exception: ") + err) {}
};

// divide by zero arithmetic exception for reals
struct lns_divide_by_zero : public lns_arithmetic_exception {
	lns_divide_by_zero() : lns_arithmetic_exception("division by zero") {}
};

// negative argument to sqrt
struct lns_negative_sqrt_arg : public lns_arithmetic_exception {
	lns_negative_sqrt_arg() : lns_arithmetic_exception("negative sqrt argument") {}
};

///////////////////////////////////////////////////////////////
// internal implementation exceptions

// base class for internal exceptions
struct lns_internal_exception : public universal_internal_exception {
	lns_internal_exception(const std::string& error)
		: universal_internal_exception(std::string("lns internal error: ") + error) {};
};

struct lns_index_out_of_bounds : public lns_internal_exception {
	lns_index_out_of_bounds() : lns_internal_exception("index out of bounds") {}
};

}} // namespace sw::universal
