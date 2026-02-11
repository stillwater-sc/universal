#pragma once
// exceptions.hpp: definition of nvblock exceptions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

// base class for nvblock arithmetic exceptions
struct nvblock_arithmetic_exception : public universal_arithmetic_exception
{
	explicit nvblock_arithmetic_exception(const std::string& error)
		: universal_arithmetic_exception(std::string("nvblock arithmetic exception: ") + error) {};
};

// divide by zero arithmetic exception for nvblock
struct nvblock_divide_by_zero : public nvblock_arithmetic_exception {
	explicit nvblock_divide_by_zero(const std::string& error = "nvblock division by zero")
		: nvblock_arithmetic_exception(error) {}
};

///////////////////////////////////////////////////////////////
// internal implementation exceptions

// base class for nvblock internal exceptions
struct nvblock_internal_exception : public universal_internal_exception
{
	explicit nvblock_internal_exception(const std::string& error)
		: universal_internal_exception(std::string("nvblock internal exception: ") + error) {};
};

struct nvblock_index_out_of_bounds : public nvblock_internal_exception {
	explicit nvblock_index_out_of_bounds(const std::string& error = "block index out of bounds")
		: nvblock_internal_exception(error) {}
};

}} // namespace sw::universal
