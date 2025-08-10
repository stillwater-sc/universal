#pragma once
// nonconst_bitcast.hpp: nonconst bitcast for source types into destination types of the same size
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// cast bits from one type to another type of the same size
	template<typename DestinationType, typename SourceType>
	DestinationType BitCast(SourceType source) {
		static_assert(sizeof(DestinationType) == sizeof(SourceType),
				"source and destination type sizes do not match");
		DestinationType dest;
		memmove(&dest, &source, sizeof(DestinationType));
		return dest;
	}

}} // namespace sw::universal
