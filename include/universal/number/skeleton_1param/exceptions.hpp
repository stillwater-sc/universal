#pragma once
// exceptions.hpp: definition of arbitrary configuration logarithmic number system exceptions
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <stdexcept>

namespace sw { namespace universal {

// divide by zero arithmetic exception for reals
struct oneparam_divide_by_zero : public std::runtime_error {
	oneparam_divide_by_zero() : std::runtime_error("oneparam division by zero") {}
};

}} // namespace sw::universal
