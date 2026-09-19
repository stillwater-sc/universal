#pragma once
// ereal_test_support.hpp: first-principles structural oracle for ereal.
//
// assert/check Priest's normal form on an ereal expansion directly from its
// limb vector -- no external oracle required (issue #954, Deliverable 1).
//
// Priest's normal form (Shewchuk's expansion arithmetic) requires, for a
// non-zero expansion z = (z_0, z_1, ..., z_{m-1}) with |z_0| >= ... >= |z_{m-1}|:
//   - decreasing magnitude:  |z_k|     >= |z_{k+1}|
//   - non-overlapping:       |z_{k+1}| <= ulp(z_k) / 2
//   - no zero components     (canonical zero is the single-limb {0.0})
//
// Note: there is intentionally NO "length <= maxlimbs" check. maxlimbs is the
// algorithmic ceiling on inputs (the static_assert in ereal_impl.hpp), not a
// cap on result length: a non-overlapping product of two N-limb expansions can
// legitimately need more than N components when its value spans more than
// N*53 binary exponents. (Investigated under #981.)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>
#include <limits>
#include <cstddef>
#include <vector>
#include <universal/number/ereal/ereal.hpp>

namespace sw { namespace universal {

// Structured result of a Priest-normal-form check: the first violation found,
// with the limb index at which it occurs, so a fuzzer failure points directly
// at the broken invariant.
struct PriestNormalResult {
	enum class Violation { None, NonDecreasing, Overlap, InteriorZero, NonFinite };
	Violation   violation = Violation::None;
	std::size_t index     = 0;

	constexpr bool ok() const noexcept { return violation == Violation::None; }
	const char* what() const noexcept {
		switch (violation) {
		case Violation::None:          return "priest-normal";
		case Violation::NonDecreasing: return "non-decreasing magnitude |z_k| < |z_{k+1}|";
		case Violation::Overlap:       return "overlap |z_{k+1}| > ulp(z_k)/2";
		case Violation::InteriorZero:  return "zero component in a non-zero expansion";
		case Violation::NonFinite:     return "non-finite (inf/nan) component";
		}
		return "unknown";
	}
};

// Core check on a raw limb vector. Returns the first violation (with index),
// or { None } if the expansion is in Priest normal form. Does not throw.
// Exposed directly so tests can feed crafted (non-normal) limb vectors that
// ereal's own operations would never produce.
// The limb type is deduced (double by default, so a braced list still means double limbs);
// the overlap bound is half an ulp of the limb type, 2^(ilogb(z_k) - digits).
template<typename FpType = double>
inline PriestNormalResult check_priest_normal(const std::vector<FpType>& L) noexcept {
	using R = PriestNormalResult;
	R r;

	// Canonical zero: a single 0.0 limb is normal by definition.
	if (L.size() == 1 && L[0] == FpType(0)) return r;

	for (std::size_t i = 0; i < L.size(); ++i) {
		// A non-finite limb is not a valid normal-form component; flag it before
		// the ilogb below, where std::ilogb(inf/nan) returns INT_MIN/INT_MAX and
		// the subsequent "- 53" would be signed-integer overflow (UB).
		if (!std::isfinite(L[i])) { r.violation = R::Violation::NonFinite; r.index = i; return r; }
		if (L[i] == FpType(0)) { r.violation = R::Violation::InteriorZero; r.index = i; return r; }
	}
	for (std::size_t i = 0; i + 1 < L.size(); ++i) {
		const FpType cur  = std::abs(L[i]);
		const FpType next = std::abs(L[i + 1]);
		if (next > cur) { r.violation = R::Violation::NonDecreasing; r.index = i; return r; }
		// ulp(z_k)/2 == 2^(ilogb(z_k) - p) for a normal z_k, p = digits of the limb (53 for double).
		const FpType half_ulp = std::ldexp(FpType(1), std::ilogb(L[i]) - std::numeric_limits<FpType>::digits);
		if (next > half_ulp) { r.violation = R::Violation::Overlap; r.index = i; return r; }
	}
	return r;
}

// Check Priest's normal form on an ereal expansion.
template<unsigned maxlimbs, typename FpType>
PriestNormalResult check_priest_normal(const ereal<maxlimbs, FpType>& v) noexcept {
	return check_priest_normal(v.limbs());
}

// Convenience predicate.
template<unsigned maxlimbs, typename FpType>
bool is_priest_normal(const ereal<maxlimbs, FpType>& v) noexcept {
	return check_priest_normal(v.limbs()).ok();
}

// Value-preserving lift of a narrow expansion into a wider configuration, by
// re-summing its (non-overlapping) limbs. Used by the precision-lifting oracle
// (#954 D2): the widest configuration ereal<19> is the value-canonical ground
// truth for any narrower one, so a narrow result lifted to wide must equal the
// same operation computed at wide precision.
template<unsigned WIDE, unsigned NARROW, typename FpType>
ereal<WIDE, FpType> widen(const ereal<NARROW, FpType>& v) {
	static_assert(WIDE >= NARROW, "widen<WIDE, NARROW>: target configuration must be at least as wide as the source");
	ereal<WIDE, FpType> w(0.0);
	for (FpType limb : v.limbs()) w += ereal<WIDE, FpType>(limb);
	return w;
}

}}  // namespace sw::universal
