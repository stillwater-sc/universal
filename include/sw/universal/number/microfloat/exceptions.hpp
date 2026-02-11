#pragma once
// exceptions.hpp: definition of microfloat exceptions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/common/exceptions.hpp>

namespace sw { namespace universal {

// base class for microfloat arithmetic exceptions
struct microfloat_arithmetic_exception : public universal_arithmetic_exception
{
	explicit microfloat_arithmetic_exception(const std::string& error)
		: universal_arithmetic_exception(std::string("microfloat arithmetic exception: ") + error) {};
};

// divide by zero arithmetic exception for microfloat
struct microfloat_divide_by_zero : public microfloat_arithmetic_exception {
	explicit microfloat_divide_by_zero(const std::string& error = "microfloat division by zero")
		: microfloat_arithmetic_exception(error) {}
};

///////////////////////////////////////////////////////////////
// internal implementation exceptions

// base class for microfloat internal exceptions
struct microfloat_internal_exception : public universal_internal_exception
{
	explicit microfloat_internal_exception(const std::string& error)
		: universal_internal_exception(std::string("microfloat internal exception: ") + error) {};
};

struct microfloat_word_index_out_of_bounds : public microfloat_internal_exception {
	explicit microfloat_word_index_out_of_bounds(const std::string& error = "word index out of bounds")
		: microfloat_internal_exception(error) {}
};

}} // namespace sw::universal
