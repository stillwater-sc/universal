#pragma once
// truncate.hpp: truncate support for ereal adaptive-precision floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// trunc: truncate value by rounding toward zero
	// Phase 2: uses Phase 1 floor/ceil based on sign
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> trunc(const ereal<maxlimbs>& x) {
		// Truncate toward zero: floor for positive, ceil for negative
		return (x >= ereal<maxlimbs>(0.0)) ? floor(x) : ceil(x);
	}

	// round: round to nearest integer, halfway cases away from zero
	// Phase 2: uses Phase 1 floor/ceil with arithmetic
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> round(const ereal<maxlimbs>& x) {
		// Round to nearest: add 0.5 then floor (for positive)
		// Symmetric handling for negative values
		if (x >= ereal<maxlimbs>(0.0)) {
			return floor(x + ereal<maxlimbs>(0.5));
		} else {
			return ceil(x - ereal<maxlimbs>(0.5));
		}
	}

	// floor: return largest integer value not greater than x
	// Phase 1: component-wise floor using expansion arithmetic
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> floor(const ereal<maxlimbs>& x) {
		const auto& limbs = x.limbs();
		if (limbs.empty() || x.iszero()) return ereal<maxlimbs>(0.0);

		// Create result expansion by flooring components
		std::vector<double> result_limbs(limbs.size(), 0.0);

		// Floor first (most significant) component
		result_limbs[0] = std::floor(limbs[0]);

		// If first component unchanged, it's already integer, check next
		if (result_limbs[0] == limbs[0]) {
			for (size_t i = 1; i < limbs.size(); ++i) {
				result_limbs[i] = std::floor(limbs[i]);
				if (result_limbs[i] != limbs[i]) {
					// This component had fractional part, zero remaining
					break;
				}
			}
		}
		// else: first component had fractional part, remaining already zeroed

		// Construct result from limbs
		ereal<maxlimbs> result;
		result = result_limbs[0];
		for (size_t i = 1; i < result_limbs.size(); ++i) {
			if (result_limbs[i] != 0.0) {
				result += ereal<maxlimbs>(result_limbs[i]);
			}
		}

		return result;
	}

	// ceil: return smallest integer value not less than x
	// Phase 1: component-wise ceil using expansion arithmetic
	template<unsigned maxlimbs>
	inline ereal<maxlimbs> ceil(const ereal<maxlimbs>& x) {
		const auto& limbs = x.limbs();
		if (limbs.empty() || x.iszero()) return ereal<maxlimbs>(0.0);

		// Create result expansion by ceiling components
		std::vector<double> result_limbs(limbs.size(), 0.0);

		// Ceil first (most significant) component
		result_limbs[0] = std::ceil(limbs[0]);

		// If first component unchanged, it's already integer, check next
		if (result_limbs[0] == limbs[0]) {
			for (size_t i = 1; i < limbs.size(); ++i) {
				result_limbs[i] = std::ceil(limbs[i]);
				if (result_limbs[i] != limbs[i]) {
					// This component had fractional part, zero remaining
					break;
				}
			}
		}
		// else: first component had fractional part, remaining already zeroed

		// Construct result from limbs
		ereal<maxlimbs> result;
		result = result_limbs[0];
		for (size_t i = 1; i < result_limbs.size(); ++i) {
			if (result_limbs[i] != 0.0) {
				result += ereal<maxlimbs>(result_limbs[i]);
			}
		}

		return result;
	}

}} // namespace sw::universal
