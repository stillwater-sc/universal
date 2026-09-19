#pragma once
// ereal_reference_digits.hpp: exact decimal-agreement oracle for ereal values.
//
// Converts an ereal<maxlimbs, FpType> to its EXACT dyadic value and reports how many
// leading significant decimal digits it agrees with a decimal reference string, or with
// another exact value. This is what lets a claim about precision be checked at all:
// ereal with long double limbs carries thousands of digits (#1355), and no measurement
// that passes through a double -- or through to_string, which rounds -- can see them.
//
// Method: an expansion is the exact sum of its limbs, each a binary floating-point value
// and so an exact dyadic rational, so the whole is M * 2^s for integers M, s (the
// einteger-backed `dyadic` in dyadic_exact.hpp, whose from_fp is exact for every limb
// type). A decimal reference "d.ddd..." is the exact rational N / 10^frac. The two are
// compared by cross-multiplication in einteger: no rounding, and no code path shared with
// ereal's arithmetic, so the oracle can catch a result that is well-formed and wrong.
//
// The companion for elreal's ZBCL is elreal_reference_digits.hpp; both use the
// comparators in dyadic_exact.hpp (#1566).
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string_view>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/dyadic_exact.hpp>

namespace sw { namespace universal {

// The exact value of an ereal, read from its limbs. Every limb type is exact here:
// dyadic::from_fp takes the significand 32 bits at a time, so an x87 or binary128 limb
// keeps all of its bits, where from_double would round it to 53.
//
// inf and nan have no dyadic value; callers must not pass them (as for from_fp).
template<unsigned maxlimbs, typename FpType>
inline dyadic ereal_to_dyadic(const ereal<maxlimbs, FpType>& v) {
	dyadic acc;
	for (FpType limb : v.limbs()) acc = acc + dyadic::from_fp(limb);
	return acc;
}

// Leading significant decimal digits an ereal agrees with a decimal reference to.
//
// `cap` bounds the work and states what the reference can certify. It must stay below the
// reference's own digit count: a value correctly rounded to D digits is off by up to half
// a unit in its last one, so it cannot certify D digits of anything (#1397). The 340-digit
// constants in reference_constants.hpp are read at 320; long_reference_constants.hpp
// carries 5000 and is read at 4900.
template<unsigned maxlimbs, typename FpType>
inline int agreed_decimal_digits(const ereal<maxlimbs, FpType>& v, std::string_view ref, int cap = 320) {
	return agreed_decimal_digits(ereal_to_dyadic(v), ref, cap);
}

// Agreement of two ereal values -- of any limb types, which is how a narrow
// configuration is checked against a wide one.
template<unsigned m, typename F, unsigned n, typename G>
inline int agreed_decimal_digits(const ereal<m, F>& a, const ereal<n, G>& b, int cap = 320) {
	return agreed_decimal_digits(ereal_to_dyadic(a), ereal_to_dyadic(b), cap);
}

}}  // namespace sw::universal
