#pragma once
// exceptions.hpp: definition of mxfloat exceptions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

// base class for mxfloat arithmetic exceptions
struct mxfloat_arithmetic_exception : public universal_arithmetic_exception
{
	explicit mxfloat_arithmetic_exception(const std::string& error)
		: universal_arithmetic_exception(std::string("mxfloat arithmetic exception: ") + error) {};
};

// divide by zero arithmetic exception for mxfloat
struct mxfloat_divide_by_zero : public mxfloat_arithmetic_exception {
	explicit mxfloat_divide_by_zero(const std::string& error = "mxfloat division by zero")
		: mxfloat_arithmetic_exception(error) {}
};

///////////////////////////////////////////////////////////////
// internal implementation exceptions

// base class for mxfloat internal exceptions
struct mxfloat_internal_exception : public universal_internal_exception
{
	explicit mxfloat_internal_exception(const std::string& error)
		: universal_internal_exception(std::string("mxfloat internal exception: ") + error) {};
};

struct mxfloat_index_out_of_bounds : public mxfloat_internal_exception {
	explicit mxfloat_index_out_of_bounds(const std::string& error = "block index out of bounds")
		: mxfloat_internal_exception(error) {}
};

}} // namespace sw::universal
