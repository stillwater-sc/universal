#pragma once
// ereal_precision.hpp: "correct to k bits" checks for the ereal math tests.
//
// The ereal math tests used to compare through double, with tolerances of 1e-10 to 1e-15:
// an ereal<8> result correct to only 53 bits passed them, so they could not see a loss of
// the precision ereal exists to deliver (#2008, #2006). These checks are exact instead. Both
// values become dyadic rationals (dyadic_exact.hpp, einteger-backed), their difference is
// exact, and the agreement is counted in bits, with no floating-point step and no code
// shared with ereal's arithmetic.
//
// Each test states the precision it expects in bits, usually as ereal_bits<N, F>() less a
// margin for the function's own error. ereal_bits counts the limbs the caller asked for;
// a result may carry up to twice as many (the limb budget), so it is the floor a result
// has to reach, not what it is likely to show.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <climits>
#include <limits>
#include <string_view>
#include <universal/verification/ereal_reference_digits.hpp>

namespace sw { namespace universal {

// Bits an ereal<maxlimbs, FpType> holds: maxlimbs limbs of the limb type's significand.
template<unsigned maxlimbs, typename FpType>
constexpr int ereal_bits() {
	return static_cast<int>(maxlimbs) * std::numeric_limits<FpType>::digits;
}

// ereal_bits for a configuration named by its type: ereal_bits_of<Real>::value in a test
// templated on Real.
template<typename Real> struct ereal_bits_of;
template<unsigned maxlimbs, typename FpType>
struct ereal_bits_of<ereal<maxlimbs, FpType>> {
	static constexpr int value = ereal_bits<maxlimbs, FpType>();
};

// The value returned for an exact match.
constexpr int kExactAgreement = INT_MAX / 2;

// Leading bits to which A agrees with B, relative to B: the largest k with
// |A - B| < |B| 2^-k, to within one bit. kExactAgreement when A == B.
// B must be nonzero; both must be finite.
inline int agreed_bits(const dyadic& A, const dyadic& B) {
	dyadic::bigint bn = B.numerator;
	if (bn.sign()) bn.setsign(false);
	if (bn.iszero()) throw std::invalid_argument("agreed_bits: zero reference value");
	dyadic d = A - B;
	dyadic::bigint dn = d.numerator;
	if (dn.sign()) dn.setsign(false);
	if (dn.iszero()) return kExactAgreement;
	// |B| >= 2^(msb(B) + sB) and |A - B| < 2^(msb(d) + sd + 1)
	return (bn.findMsb() + B.scale) - (dn.findMsb() + d.scale) - 1;
}

// Bits to which an ereal agrees with another ereal, of any configuration or limb type:
// a narrow result against a wide one, or one side of an identity against the other.
template<unsigned m, typename F, unsigned n, typename G>
inline int agreed_bits(const ereal<m, F>& a, const ereal<n, G>& b) {
	return agreed_bits(ereal_to_dyadic(a), ereal_to_dyadic(b));
}

// Bits to which an ereal agrees with a decimal reference string, such as a constant from
// reference_constants.hpp. A reference of D digits certifies about 3.32 D bits; `cap`
// bounds the digits read, as for agreed_decimal_digits.
template<unsigned maxlimbs, typename FpType>
inline int agreed_bits(const ereal<maxlimbs, FpType>& v, std::string_view ref, int cap = 320) {
	const int digits = agreed_decimal_digits(v, ref, cap);
	return static_cast<int>(digits * 3.3219280948873623);   // log2(10)
}

// Absolute agreement, for a reference that is zero or a difference that should vanish:
// the largest k with |a - b| < 2^-k. kExactAgreement when a == b.
template<unsigned m, typename F, unsigned n, typename G>
inline int agreed_bits_absolute(const ereal<m, F>& a, const ereal<n, G>& b) {
	dyadic d = ereal_to_dyadic(a) - ereal_to_dyadic(b);
	dyadic::bigint dn = d.numerator;
	if (dn.sign()) dn.setsign(false);
	if (dn.iszero()) return kExactAgreement;
	return -(dn.findMsb() + d.scale) - 1;
}

// The check a test asks for: got agrees with ref to at least `bits` bits.
template<unsigned m, typename F, unsigned n, typename G>
inline bool agrees_to_bits(const ereal<m, F>& got, const ereal<n, G>& ref, int bits) {
	if (got.isnan() || ref.isnan() || got.isinf() || ref.isinf()) return false;
	if (ref.iszero()) return got.iszero();
	return agreed_bits(got, ref) >= bits;
}

template<unsigned maxlimbs, typename FpType>
inline bool agrees_to_bits(const ereal<maxlimbs, FpType>& got, std::string_view ref, int bits) {
	if (got.isnan() || got.isinf() || got.iszero()) return false;
	const int cap = static_cast<int>(bits / 3.3219280948873623) + 2;   // digits enough to certify `bits`
	return agreed_bits(got, ref, cap) >= bits;
}

}}  // namespace sw::universal
