#pragma once
// infinite_encoding.hpp: definition of encodings of infinite for real types
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw::universal {

	static constexpr int NAN_TYPE_SIGNALLING = -1;   // a Signalling NaN
	static constexpr int NAN_TYPE_EITHER = 0;        // any NaN
	static constexpr int NAN_TYPE_QUIET = 1;         // a Quiet NaN

} // namespace sw::universal
