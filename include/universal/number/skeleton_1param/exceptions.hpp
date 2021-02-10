#pragma once
// exceptions.hpp: definition of arbitrary configuration logarithmic number system exceptions
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <stdexcept>

namespace sw { namespace universal {

// divide by zero arithmetic exception for reals
struct lns_divide_by_zero : public std::runtime_error {
	lns_divide_by_zero() : std::runtime_error("lns division by zero") {}
};

}} // namespace sw::universal
