#pragma once
// logarithm.hpp: logarithm functions for ereal adaptive-precision floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// log: natural logarithm (base e)
	// Phase 0: stub using double conversion
	// TODO Phase 2: implement using Taylor series with argument reduction
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> log(const ereal<maxlimbs>& x) {
		return ereal<maxlimbs>(std::log(double(x)));
	}

	// log2: binary logarithm (base 2)
	// Phase 0: stub using double conversion
	// TODO Phase 2: implement using log(x) / log(2)
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> log2(const ereal<maxlimbs>& x) {
		return ereal<maxlimbs>(std::log2(double(x)));
	}

	// log10: common logarithm (base 10)
	// Phase 0: stub using double conversion
	// TODO Phase 2: implement using log(x) / log(10)
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> log10(const ereal<maxlimbs>& x) {
		return ereal<maxlimbs>(std::log10(double(x)));
	}

	// log1p: compute log(1 + x) accurately for small x
	// Phase 0: stub using double conversion
	// TODO Phase 2: implement using Taylor series (avoids cancellation)
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> log1p(const ereal<maxlimbs>& x) {
		return ereal<maxlimbs>(std::log1p(double(x)));
	}

}} // namespace sw::universal
