// rounding.hpp: generalized rounding modes for the Universal Numbers Library
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#pragma once

namespace sw { namespace universal {

// Standard IEEE-754 rounding modes used across Universal number systems
enum class RoundingMode {
	RoundToNearest,         // round to nearest, ties to even (default)
	RoundToZero,            // truncation
	RoundTowardPositive,    // round toward +infinity
	RoundTowardNegative     // round toward -infinity
};

}} // namespace sw::universal
