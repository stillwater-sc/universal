#pragma once
// specific_value_encoding.hpp: definition of encodings of special values
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// special number system values
	enum class SpecificValue {
		maxpos, minpos, zero, minneg, maxneg, infpos, infneg, qnan, snan, nar
	};

}} // namespace sw::universal
