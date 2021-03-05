#pragma once
// subnormal.hpp: definitions of helpful constants to interpret subnormals (IEEE-754, bfloats, areals. etc)
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>

namespace sw::universal {

	static constexpr double oneOver2p6 = 0.015625;
	static constexpr double oneOver2p14 = 0.00006103515625;
	static constexpr double oneOver2p30 = 1.0 / 1073741824.0;
	static constexpr double oneOver2p50 = 1.0 / 1125899906842624.0;
	static constexpr double oneOver2p62 = 1.0 / 4611686018427387904.0;
	static constexpr double oneOver2p126 = oneOver2p62 * oneOver2p62 * 0.25;
	static constexpr double oneOver2p254 = oneOver2p126 * oneOver2p126 * 0.25;
	static constexpr double oneOver2p510 = oneOver2p254 * oneOver2p254 * 0.25;
	static constexpr double oneOver2p1022 = oneOver2p510 * oneOver2p510 * 0.25;

	// precomputed values for subnormal exponents as a function of es
	static constexpr int subnormal_reciprocal_shift[] = {
		0,                    // es = 0 : not a valid value
		-1,                   // es = 1 : 2^(2 - 2^(es-1)) = 2^1
		0,                    // es = 2 : 2^(2 - 2^(es-1)) = 2^0
		2,                    // es = 3 : 2^(2 - 2^(es-1)) = 2^-2
		6,                    // es = 4 : 2^(2 - 2^(es-1)) = 2^-6
		14,                   // es = 5 : 2^(2 - 2^(es-1)) = 2^-14
		30,                   // es = 6 : 2^(2 - 2^(es-1)) = 2^-30
		62,                   // es = 7 : 2^(2 - 2^(es-1)) = 2^-62
		126,                  // es = 8 : 2^(2 - 2^(es-1)) = 2^-126
		254,                  // es = 9 : 2^(2 - 2^(es-1)) = 2^-254
		510,                  // es = 10 : 2^(2 - 2^(es-1)) = 2^-510
		1022                  // es = 11 : 2^(2 - 2^(es-1)) = 2^-1022
	};
	// es > 11 requires a long double representation, which MSVC does not provide.
	static constexpr double subnormal_exponent[] = {
		0,                    // es = 0 : not a valid value
		2.0,                  // es = 1 : 2^(2 - 2^(es-1)) = 2^1
		1.0,                  // es = 2 : 2^(2 - 2^(es-1)) = 2^0
		0.25,                 // es = 3 : 2^(2 - 2^(es-1)) = 2^-2
		oneOver2p6,           // es = 4 : 2^(2 - 2^(es-1)) = 2^-6
		oneOver2p14,          // es = 5 : 2^(2 - 2^(es-1)) = 2^-14
		oneOver2p30,          // es = 6 : 2^(2 - 2^(es-1)) = 2^-30
		oneOver2p62,          // es = 7 : 2^(2 - 2^(es-1)) = 2^-62
		oneOver2p126,         // es = 8 : 2^(2 - 2^(es-1)) = 2^-126
		oneOver2p254,         // es = 9 : 2^(2 - 2^(es-1)) = 2^-254
		oneOver2p510,         // es = 10 : 2^(2 - 2^(es-1)) = 2^-510
		oneOver2p1022         // es = 11 : 2^(2 - 2^(es-1)) = 2^-1022
	};

}  // namespace sw::universal
