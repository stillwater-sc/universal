#pragma once
// exceptions.hpp: definition of flexible configuration unum exceptions
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <stdexcept>

namespace sw { namespace universal {

// divide by zero arithmetic exception for unum
struct unum_divide_by_zero : public std::runtime_error {
	unum_divide_by_zero() : std::runtime_error("unum division by zero") {}
};

}} // namespace sw::universal
