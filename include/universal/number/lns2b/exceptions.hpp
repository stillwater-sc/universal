#pragma once
// exceptions.hpp: definition of arbitrary configuration logarithmic number system exceptions
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

struct mdlns_arithmetic_exception : public universal_arithmetic_exception {
	mdlns_arithmetic_exception(const std::string& err) : universal_arithmetic_exception(std::string("mdlns arithmetic exception: ") + err) {}
};

// divide by zero arithmetic exception for reals
struct mdlns_divide_by_zero : public mdlns_arithmetic_exception {
	mdlns_divide_by_zero() : mdlns_arithmetic_exception("division by zero") {}
};

// negative argument to sqrt
struct mdlns_negative_sqrt_arg : public mdlns_arithmetic_exception {
	mdlns_negative_sqrt_arg() : mdlns_arithmetic_exception("negative sqrt argument") {}
};

///////////////////////////////////////////////////////////////
// internal implementation exceptions

// base class for internal exceptions
struct mdlns_internal_exception : public universal_internal_exception {
	mdlns_internal_exception(const std::string& error)
		: universal_internal_exception(std::string("mdlns internal error: ") + error) {};
};

struct mdlns_index_out_of_bounds : public mdlns_internal_exception {
	mdlns_index_out_of_bounds() : mdlns_internal_exception("index out of bounds") {}
};

}} // namespace sw::universal
