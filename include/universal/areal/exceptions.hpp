#pragma once
// areal_exceptions.hpp: definition of arbitrary configuration real exceptions
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <stdexcept>

namespace sw { namespace unum {

// divide by zero arithmetic exception for reals
struct areal_divide_by_zero : public std::runtime_error {
	areal_divide_by_zero() : std::runtime_error("areal division by zero") {}
};

}} // namespace sw::unum
