#pragma once
// subnormal.hpp: definitions of helpful constants to interpret subnormals (IEEE-754, cfloats, areals. etc)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>

namespace sw { namespace universal {

	static constexpr double oneOver2p6 = 0.015625;
	static constexpr double oneOver2p14 = 0.00006103515625;
	static constexpr double oneOver2p30 = 1.0 / 1073741824.0;
	static constexpr double oneOver2p50 = 1.0 / 1125899906842624.0;
	static constexpr double oneOver2p62 = 1.0 / 4611686018427387904.0;
	static constexpr double oneOver2p126 = oneOver2p62 * oneOver2p62 * 0.25;
	static constexpr double oneOver2p254 = oneOver2p126 * oneOver2p126 * 0.25;
	static constexpr double oneOver2p510 = oneOver2p254 * oneOver2p254 * 0.25;
	static constexpr double oneOver2p1022 = oneOver2p510 * oneOver2p510 * 0.25;

	// precomputed values for subnormal exponent scales as a function of es
	static constexpr int subnormal_reciprocal_shift[] = {
		0,                    // es =  0 : not a valid value
		-1,                   // es =  1 : 2^(2 - 2^(es-1)) = 2^1
		0,                    // es =  2 : 2^(2 - 2^(es-1)) = 2^0
		2,                    // es =  3 : 2^(2 - 2^(es-1)) = 2^-2
		6,                    // es =  4 : 2^(2 - 2^(es-1)) = 2^-6
		14,                   // es =  5 : 2^(2 - 2^(es-1)) = 2^-14
		30,                   // es =  6 : 2^(2 - 2^(es-1)) = 2^-30
		62,                   // es =  7 : 2^(2 - 2^(es-1)) = 2^-62
		126,                  // es =  8 : 2^(2 - 2^(es-1)) = 2^-126
		254,                  // es =  9 : 2^(2 - 2^(es-1)) = 2^-254
		510,                  // es = 10 : 2^(2 - 2^(es-1)) = 2^-510
		1022,                 // es = 11 : 2^(2 - 2^(es-1)) = 2^-1022
		2046,                 // es = 12 : 2^(2 - 2^(es-1)) = 2^-2046
		4094,                 // es = 13 : 2^(2 - 2^(es-1)) = 2^-4094
		8190,                 // es = 14 : 2^(2 - 2^(es-1)) = 2^-8190
		16382,                // es = 15 : 2^(2 - 2^(es-1)) = 2^-16382
		32766,                // es = 16 : 2^(2 - 2^(es-1)) = 2^-32766
		65534,                // es = 17 : 2^(2 - 2^(es-1)) = 2^-65534
		131070,               // es = 18 : 2^(2 - 2^(es-1)) = 2^-131070
		262142,               // es = 19 : 2^(2 - 2^(es-1)) = 2^-262142
		524286                // es = 20 : 2^(2 - 2^(es-1)) = 2^-524286
	};

	// es > 11 requires a long double representation, which MSVC, ARM, and RISC-V do not provide.
	static constexpr double subnormal_exponent[] = {
		0,                    // es =  0 : not a valid value
		2.0,                  // es =  1 : 2^(2 - 2^(es-1)) = 2^1
		1.0,                  // es =  2 : 2^(2 - 2^(es-1)) = 2^0
		0.25,                 // es =  3 : 2^(2 - 2^(es-1)) = 2^-2
		oneOver2p6,           // es =  4 : 2^(2 - 2^(es-1)) = 2^-6
		oneOver2p14,          // es =  5 : 2^(2 - 2^(es-1)) = 2^-14
		oneOver2p30,          // es =  6 : 2^(2 - 2^(es-1)) = 2^-30
		oneOver2p62,          // es =  7 : 2^(2 - 2^(es-1)) = 2^-62
		oneOver2p126,         // es =  8 : 2^(2 - 2^(es-1)) = 2^-126
		oneOver2p254,         // es =  9 : 2^(2 - 2^(es-1)) = 2^-254
		oneOver2p510,         // es = 10 : 2^(2 - 2^(es-1)) = 2^-510
		oneOver2p1022,        // es = 11 : 2^(2 - 2^(es-1)) = 2^-1022
		0.0,                  // es = 12 : 2^(2 - 2^(es-1)) = 2^-2046
		0.0,                  // es = 13 : 2^(2 - 2^(es-1)) = 2^-4094
		0.0,                  // es = 14 : 2^(2 - 2^(es-1)) = 2^-8190
		0.0,                  // es = 15 : 2^(2 - 2^(es-1)) = 2^-16382
		0.0,                  // es = 16 : 2^(2 - 2^(es-1)) = 2^-32766
		0.0,                  // es = 17 : 2^(2 - 2^(es-1)) = 2^-65534
		0.0,                  // es = 18 : 2^(2 - 2^(es-1)) = 2^-131070
		0.0,                  // es = 19 : 2^(2 - 2^(es-1)) = 2^-262142
		0.0                   // es = 20 : 2^(2 - 2^(es-1)) = 2^-524286
	};

	// float subnormals with the last entry being the smallest normal value
	constexpr float ieee754_float_subnormals[24] = {
		 1.401298464324817e-45f,
		 2.802596928649634e-45f,
		 5.605193857299268e-45f,
		 1.121038771459854e-44f,
		 2.242077542919707e-44f,
		 4.484155085839415e-44f,
		 8.968310171678829e-44f,
		 1.793662034335766e-43f,
		 3.587324068671532e-43f,
		 7.174648137343063e-43f,
		 1.434929627468613e-42f,
		 2.869859254937225e-42f,
		 5.739718509874451e-42f,
		 1.147943701974890e-41f,
		 2.295887403949780e-41f,
		 4.591774807899561e-41f,
		 9.183549615799121e-41f,
		 1.836709923159824e-40f,
		 3.673419846319648e-40f,
		 7.346839692639297e-40f,
		 1.469367938527859e-39f,
		 2.938735877055719e-39f,
		 5.877471754111438e-39f,
		 1.175494350822288e-38f     // smallest normal value
	};

	// double subnormals with the last entry being the smallest normal value
	constexpr double ieee754_double_subnormals[53] = {
		 4.940656458412465e-324,
		 9.881312916824931e-324,
		 1.976262583364986e-323,
		 3.952525166729972e-323,
		 7.905050333459945e-323,
		 1.581010066691989e-322,
		 3.162020133383978e-322,
		 6.324040266767956e-322,
		 1.264808053353591e-321,
		 2.529616106707182e-321,
		 5.059232213414365e-321,
		 1.011846442682873e-320,
		 2.023692885365746e-320,
		 4.047385770731492e-320,
		 8.094771541462983e-320,
		 1.618954308292597e-319,
		 3.237908616585193e-319,
		 6.475817233170387e-319,
		 1.295163446634077e-318,
		 2.590326893268155e-318,
		 5.180653786536309e-318,
		 1.036130757307262e-317,
		 2.072261514614524e-317,
		 4.144523029229047e-317,
		 8.289046058458095e-317,
		 1.657809211691619e-316,
		 3.315618423383238e-316,
		 6.631236846766476e-316,
		 1.326247369353295e-315,
		 2.65249473870659e-315,
		 5.304989477413181e-315,
		 1.060997895482636e-314,
		 2.121995790965272e-314,
		 4.243991581930545e-314,
		 8.487983163861089e-314,
		 1.697596632772218e-313,
		 3.395193265544436e-313,
		 6.790386531088871e-313,
		 1.358077306217774e-312,
		 2.716154612435549e-312,
		 5.432309224871097e-312,
		 1.086461844974219e-311,
		 2.172923689948439e-311,
		 4.345847379896878e-311,
		 8.691694759793755e-311,
		 1.738338951958751e-310,
		 3.476677903917502e-310,
		 6.953355807835004e-310,
		 1.390671161567001e-309,
		 2.781342323134002e-309,
		 5.562684646268003e-309,
		 1.112536929253600691545e-308,
		 2.2250738585072013831e-308        // smallest normal value
	};
}} // namespace sw::universal
