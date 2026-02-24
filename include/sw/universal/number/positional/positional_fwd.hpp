#pragma once
// positional_fwd.hpp: forward references for fixed-size positional (sign-magnitude, multi-radix) integer type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// Primary template
	template<unsigned _ndigits, unsigned _radix> class positional;

	// Forward reference for abs
	template<unsigned ndigits, unsigned radix>
	positional<ndigits, radix> abs(const positional<ndigits, radix>&);

}} // namespace sw::universal
