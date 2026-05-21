#pragma once
// elreal_constants.hpp: math constants as lazy elreal streams (Phase E.1)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// =============================================================================
// Math constants for elreal (issue #887, part of Phase E #878)
// =============================================================================
//
// Each constant is exposed as a free function that returns an elreal value
// with a pre-computed multi-component expansion. The component values are
// taken from the qd-precision constants in
// `include/sw/universal/number/qd/math/constants/qd_constants.hpp`
// (precomputed offline by Scibuilders/Jack Poulson and Stillwater) and
// re-used here for the depth-0..3 corrections (~212 bits cumulative).
//
// Phase E.1 ships static 4-component expansions. Deeper refinement
// (beyond ~212 bits) is needed by Phase E.6's forward-trig range
// reduction; a future enhancement will replace the static expansion
// with a generator that produces additional bits on demand (BBP for pi,
// Taylor for e, Machin-like for ln2/ln10).

#include <universal/number/elreal/elreal_fwd.hpp>
#include <universal/number/elreal/elreal_impl.hpp>

namespace sw { namespace universal {

// Pi multiples and fractions ---------------------------------------------------

inline elreal elreal_pi() {
	return elreal::from_expansion({
		 3.1415926535897931,
		 1.2246467991473532e-16,
		-2.9947698097183397e-33,
		 1.1124542208633657e-49
	});
}

inline elreal elreal_pi_2() {
	return elreal::from_expansion({
		 1.5707963267948966,
		 6.123233995736766e-17,
		-1.4973849048591698e-33,
		 5.5622711043168283e-50
	});
}

inline elreal elreal_pi_4() {
	return elreal::from_expansion({
		 0.78539816339744828,
		 3.061616997868383e-17,
		-7.4869245242958492e-34,
		 2.7811355521584142e-50
	});
}

inline elreal elreal_2pi() {
	return elreal::from_expansion({
		 6.2831853071795862,
		 2.4492935982947064e-16,
		-5.9895396194366793e-33,
		 2.2249084417267313e-49
	});
}

// Euler's number e ------------------------------------------------------------

inline elreal elreal_e() {
	return elreal::from_expansion({
		 2.7182818284590451,
		 1.4456468917292502e-16,
		-2.1277171080381768e-33,
		 1.515630159841219e-49
	});
}

// Natural logarithms (base e) -------------------------------------------------

inline elreal elreal_ln2() {
	return elreal::from_expansion({
		 0.69314718055994529,
		 2.3190468138462996e-17,
		 5.7077084384162121e-34,
		-3.5824322106018105e-50
	});
}

inline elreal elreal_ln10() {
	return elreal::from_expansion({
		 2.3025850929940459,
		-2.1707562233822494e-16,
		-9.9842624544657766e-33,
		-4.0233574544502071e-49
	});
}

// Binary logarithms (base 2) --------------------------------------------------

inline elreal elreal_lge() {
	return elreal::from_expansion({
		 1.4426950408889634,
		 2.0355273740931033e-17,
		-1.0614659956117258e-33,
		-1.3836716780181395e-50
	});
}

inline elreal elreal_lg10() {
	return elreal::from_expansion({
		 3.3219280948873622,
		 1.6616175169735920e-16,
		 1.2215512178458181e-32,
		 5.9551189702782481e-49
	});
}

// Square roots ----------------------------------------------------------------

inline elreal elreal_sqrt2() {
	return elreal::from_expansion({
		 1.4142135623730951,
		-9.6672933134529135e-17,
		 4.1386753086994136e-33,
		 4.9355469914683509e-50
	});
}

inline elreal elreal_sqrt3() {
	return elreal::from_expansion({
		 1.7320508075688772,
		 1.0035084221806903e-16,
		-1.4959542475733896e-33,
		 5.3061475632961675e-50
	});
}

// Golden ratio ----------------------------------------------------------------

inline elreal elreal_phi() {
	return elreal::from_expansion({
		 1.6180339887498949,
		-5.4321152036825061e-17,
		 2.6543252083815655e-33,
		-3.3049919975020988e-50
	});
}

}} // namespace sw::universal
