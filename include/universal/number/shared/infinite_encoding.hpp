#pragma once
// infinite_encoding.hpp: definition of encodings of infinite for real types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	static constexpr int INF_TYPE_NEGATIVE = -1;   // -inf
	static constexpr int INF_TYPE_EITHER   =  0;   // any inf
	static constexpr int INF_TYPE_POSITIVE =  1;   // +inf
	static constexpr int INF_TYPE_NEITHER  =  2;   // not an inf

}} // namespace sw::universal
