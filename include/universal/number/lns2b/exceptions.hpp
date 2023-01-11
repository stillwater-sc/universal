#pragma once
// exceptions.hpp: definition of arbitrary configuration 2-base logarithmic number system exceptions
//
// Copyright (C) 2022-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

struct lns2b_arithmetic_exception : public universal_arithmetic_exception {
	lns2b_arithmetic_exception(const std::string& err) : universal_arithmetic_exception(std::string("lns2b arithmetic exception: ") + err) {}
};

// divide by zero arithmetic exception for reals
struct lns2b_divide_by_zero : public lns2b_arithmetic_exception {
	lns2b_divide_by_zero() : lns2b_arithmetic_exception("division by zero") {}
};

// negative argument to sqrt
struct lns2b_negative_sqrt_arg : public lns2b_arithmetic_exception {
	lns2b_negative_sqrt_arg() : lns2b_arithmetic_exception("negative sqrt argument") {}
};

///////////////////////////////////////////////////////////////
// internal implementation exceptions

// base class for internal exceptions
struct lns2b_internal_exception : public universal_internal_exception {
	lns2b_internal_exception(const std::string& error)
		: universal_internal_exception(std::string("lns2b internal error: ") + error) {};
};

struct lns2b_index_out_of_bounds : public lns2b_internal_exception {
	lns2b_index_out_of_bounds() : lns2b_internal_exception("index out of bounds") {}
};

}} // namespace sw::universal
