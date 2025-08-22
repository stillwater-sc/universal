#pragma once
// exceptions.hpp: definition of bfloat exceptions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

// base class for bfloat arithmetic exceptions
struct bfloat_arithmetic_exception : public universal_arithmetic_exception
{
	explicit bfloat_arithmetic_exception(const std::string& error) 
		: universal_arithmetic_exception(std::string("bfloat arithmetic exception: ") + error) {};
};

// divide by zero arithmetic exception for fixed-point
struct bfloat_divide_by_zero : public bfloat_arithmetic_exception {
	explicit bfloat_divide_by_zero(const std::string& error = "bfloat division by zero") 
		: bfloat_arithmetic_exception(error) {}
};

///////////////////////////////////////////////////////////////
// internal implementation exceptions

// base class for fixed-point internal exceptions
struct bfloat_internal_exception : public universal_internal_exception
{
	explicit bfloat_internal_exception(const std::string& error) 
		: universal_internal_exception(std::string("bfloat internal exception: ") + error) {};
};

struct bfloat_word_index_out_of_bounds : public bfloat_internal_exception {
	explicit bfloat_word_index_out_of_bounds(const std::string& error = "word index out of bounds") 
		: bfloat_internal_exception(error) {}
};

}} // namespace sw::universal
