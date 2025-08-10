#pragma once
// exceptions.hpp: definition of SORN number system exceptions
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

struct sorn_arithmetic_exception : public universal_arithmetic_exception {
	sorn_arithmetic_exception(const std::string& err) : universal_arithmetic_exception(std::string("sorn arithmetic exception: ") + err) {}
};

// divide by zero arithmetic exception for reals
struct sorn_divide_by_zero : public sorn_arithmetic_exception {
	sorn_divide_by_zero() : sorn_arithmetic_exception("division by zero") {}
};

// negative argument to sqrt
struct sorn_negative_sqrt_arg : public sorn_arithmetic_exception {
	sorn_negative_sqrt_arg() : sorn_arithmetic_exception("negative sqrt argument") {}
};

///////////////////////////////////////////////////////////////
// internal implementation exceptions

// base class for internal exceptions
struct sorn_internal_exception : public universal_internal_exception {
	sorn_internal_exception(const std::string& error)
		: universal_internal_exception(std::string("sorn internal error: ") + error) {};
};

struct sorn_index_out_of_bounds : public sorn_internal_exception {
	sorn_index_out_of_bounds() : sorn_internal_exception("index out of bounds") {}
};

}} // namespace sw::universal
