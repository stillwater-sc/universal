#pragma once
// nan_encoding.hpp: definition of encodings of NaN for IEEE-754-style floating-point types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	static constexpr int NAN_TYPE_SIGNALLING = -1;   // a Signalling NaN
	static constexpr int NAN_TYPE_EITHER     =  0;   // any NaN
	static constexpr int NAN_TYPE_QUIET      =  1;   // a Quiet NaN
	static constexpr int NAN_TYPE_NEITHER    =  2;   // not a NaN

}} // namespace sw::universal
