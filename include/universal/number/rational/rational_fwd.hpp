#pragma once
// rational_fwd.hpp: forard references for fixed-sized arbitrary configuration binary rational arithmetic type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// Forward references
	template<unsigned nbits, typename bt> class rational;
	template<unsigned nbits, typename bt> rational<nbits,bt> abs(const rational<nbits,bt>&);

}} // namespace sw::universal

