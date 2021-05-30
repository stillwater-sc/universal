#pragma once
// specific_value_encoding.hpp: definition of encodings of special values
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw::universal {

	// special number system values
	enum class SpecificValue {
		maxpos, minpos, zero, minneg, maxneg
	};

} // namespace sw::universal
